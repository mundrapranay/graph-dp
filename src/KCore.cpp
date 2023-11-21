/**
 * @file kCore.cpp
 * @brief Approximate KCore Distributed Algorithm
*/

/**
 * @todo: 
 *      Implement LEDP Denset Subgraph 
 *      Implement Triangle Counting 
*/

#include <math.h>
#include <mpi.h>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include <numeric>
#include "LDS.h"
#include "Graph.h"
#include "distributions.h"

#define COORDINATOR 0 
#define FROM_MASTER 1
#define FROM_WORKER 2

namespace distributed_kcore {


int log_a_to_base_b(int a, double b) {
    // log_b a = log_2 a / log_2 b
    return log2(a) / log2(b);
}

LDS* KCore_compute(int rank, int nprocs, Graph* graph, double eta, double epsilon, double phi, double lambda, int levels_per_group, double factor, int bias, int bias_factor, int n) {
    // change this to min(rounds_param, round_threshold[node]) forall node in graph
    double rounds_param = ceil(4.0 * pow(log_a_to_base_b(n, 1.0 + phi), 1.5));
    int number_of_rounds = static_cast<int>(rounds_param);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset, mytype, workLoad, p;
    int workLoadSize;
    // to decide the size of the datastructures for each process
    if (rank == COORDINATOR) {
        workLoadSize = n;
    } else {
        workLoadSize = (rank == numworkers) ? chunk + extra : chunk;
    }

    MPI_Status status;
    LDS *lds;
    std::vector<int> roundThresholds(workLoadSize, 0);
    std::vector<int> noised_degrees(workLoadSize, 0);
    std::vector<int> nodeDegrees;
    if (rank == COORDINATOR) {
        roundThresholds.clear();
        roundThresholds.shrink_to_fit();
    }
    double remaingingBudget = (factor != 1.0) ? (1.0 - factor) : 0.0;
    if (rank == COORDINATOR) {
        lds = new LDS(n, phi, levels_per_group, false);
    } else {
        int offset_nd = (rank - 1) * chunk; 
        int workLoad_nd = (rank == numworkers) ? chunk + extra : chunk;
        GeometricDistribution* geomThreshold = new GeometricDistribution(epsilon * factor);
        nodeDegrees = graph->getNodeDegreeVector((rank - 1) * chunk);
        for (int i = 0; i < nodeDegrees.size(); i++) {
            noised_degrees[i] = nodeDegrees[i];
            // noised_degrees[i] = nodeDegrees[i] + geomThreshold->Sample();
            if (bias == 1) {
                noised_degrees[i] -= std::min(noised_degrees[i] - 1, bias_factor);
            }
            int numberOfRounds = ceil(log2(noised_degrees[i])) * levels_per_group;
            roundThresholds[i] = numberOfRounds;
        }
    }

    noised_degrees.clear();
    noised_degrees.shrink_to_fit();
    MPI_Barrier(MPI_COMM_WORLD);
    std::vector<int> permanentZeros(workLoadSize, 1);

    for (int r = 0; r < number_of_rounds - 2; r++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> round_start, round_end;
	    std::chrono::duration<double> round_elapsed;
        double round_time = 0.0;
        // each node either releases 1 or 0 and the coordinator updates the level accordingly
        // nextLevels stores this information
        round_start = std::chrono::high_resolution_clock::now();
        std::vector<int> currentLevels;
        std::vector<int> nextLevels(workLoadSize, 0);
        int group_index; 
        if (rank == COORDINATOR) {

            // worker send a request for current levels their node ids
            /**
             * @todo: Check if messages are received in order. Use blocking calls 
            */
            int node_degree_sums[numworkers];
            for (p = 1; p <= numworkers; p++) {
                std::vector<int> requested_node_ids;
                int node_degree_sum;
                MPI_Recv(&node_degree_sum, 1, MPI_INT, p, FROM_WORKER + p, MPI_COMM_WORLD, &status);
                // std::cout << "Node Degree Sum " << node_degree_sum << " for worker " << p << std::endl;
                requested_node_ids.resize(node_degree_sum);
                MPI_Recv(&requested_node_ids[0], node_degree_sum, MPI_INT, p, FROM_WORKER + p, MPI_COMM_WORLD, &status);
                // std::cout << "Received from Worker " << p << " Node IDs size: " << requested_node_ids.size() << std::endl;
                for (int node = 0; node < requested_node_ids.size(); node++) {
                    currentLevels.push_back(lds->get_level(requested_node_ids[node]));
                }
                node_degree_sums [p - 1] = node_degree_sum;
                // MPI_Send(&currentLevels[0], node_degree_sum, MPI_INT, p, FROM_MASTER, MPI_COMM_WORLD);
                // currentLevels.clear();
                requested_node_ids.clear();
            }

            // for (int last_few = currentLevels.size() - 10; last_few < currentLevels.size(); last_few++) {
            //     std::cout << "Current Levels at " << last_few << " : " << currentLevels[last_few] << std::endl;
            // }

            int current_index = 0;
            for (p = 1; p <= numworkers; p++) {
                MPI_Send(&currentLevels[current_index], node_degree_sums[p - 1], MPI_INT, p, FROM_MASTER, MPI_COMM_WORLD);
                current_index += node_degree_sums[p - 1];
            }



            group_index = lds->group_for_level(r);
            offset = 0;
            mytype = FROM_MASTER;
            for (p = 1; p <= numworkers; p++) {
                workLoad = (p == numworkers) ? chunk + extra : chunk;
                MPI_Send(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&group_index, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&permanentZeros[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD);
                offset += workLoad;
            }
            
            // receive results from workers
            for (p = 1; p <= numworkers; p++) {
                mytype = FROM_WORKER + p;
                MPI_Recv(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&permanentZeros[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
            }

            // update the levels based on the data in nextLevels
            for (int i = 0; i < nextLevels.size(); i++) {
                if (nextLevels[i] == 1 && permanentZeros[i] != 0) {
                    lds->level_increase(i, lds->L);
                } 
            }
            // std::cout << "Round: " << r << std::endl;
            // // printing information for debugging
            // for (int i = 0; i < n; i++) {
            //     std::cout << nextLevels[i] << " ";
            // }
            // std::cout << std::endl;

            // for (int i = 0; i < n; i++) {
            //     std::cout << permanentZeros[i] << " ";
            // }
            // std::cout << std::endl;

        } else {
            // worker task

            // send a request to coordinator to get currentLevels for working nodes
            std::vector<int> oal = graph->computeOAL();
            int node_degree_sum = oal.size(); // n + m
            mytype = FROM_WORKER + rank;
            MPI_Send(&node_degree_sum, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&oal[0], node_degree_sum, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);

            currentLevels.resize(node_degree_sum);
            MPI_Recv(&currentLevels[0], node_degree_sum, MPI_INT, COORDINATOR, FROM_MASTER, MPI_COMM_WORLD, &status);

            mytype = FROM_MASTER;
            MPI_Recv(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&group_index, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&permanentZeros[0], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            // perform computation
            /**
             * @todo: Check if we are reading the correct neighbours 
             *        Print out the ADL from OAL and check if it's the same as the OG Graph
            */
            int end_node = offset + workLoad;
            int start = 0;
            for (int currNode = offset; currNode < end_node; currNode++) {
                int node_degree = nodeDegrees[currNode - offset];
                if (roundThresholds[currNode - offset] == r) {
                    permanentZeros[currNode - offset] = 0;
                }
                if (currentLevels[start] == r && permanentZeros[currNode - offset] != 0) {
                    int end_adl = start + node_degree + 1;
                    int U_i = 0;
                    for (int adj_start = start + 1; adj_start < end_adl; adj_start++) {
                        if (currentLevels[adj_start] == r) {
                            U_i += 1;
                        }
                    }
                    double distribution_factor = (epsilon * remaingingBudget) / (2.0 * rounds_param);
                    GeometricDistribution* geom = new GeometricDistribution(distribution_factor);
                    // int noise = geom->Sample();
                    int noise = 0;
                    int U_hat_i = U_i + noise;
                    if (U_hat_i > pow((1 + phi), group_index)) {
                            nextLevels[currNode - offset] = 1;
                    } else {
                            permanentZeros[currNode - offset] = 0;
                    }
                }
                start += node_degree + 1;
            }
            // send back the completed data to COORDINATOR
            mytype = FROM_WORKER + rank;
            MPI_Send(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&nextLevels[0], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&permanentZeros[0], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        round_end = std::chrono::high_resolution_clock::now();
        round_elapsed = round_end - round_start;
        round_time = round_elapsed.count();
    //    if (rank == COORDINATOR) {
    //         std::cout << "Round " << r << " | " << number_of_rounds - 2 << std::endl;
    //         std::cout << "Round time: " << round_time << std::endl;
    //     }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // free up memory
    permanentZeros.clear();
    // roundThresholds.clear();
    permanentZeros.shrink_to_fit();
    // roundThresholds.shrink_to_fit();
    nodeDegrees.clear();
    nodeDegrees.shrink_to_fit();
    return lds;
}

// Computing Approximate Core Numbers
std::vector<double> estimateCoreNumbers(LDS* lds, int n, double eta, double phi, double lambda, double levels_per_group) {
    std::vector<double> coreNumbers(n);
    double two_plus_lambda = 2.0 + lambda;
    double one_plus_phi = 1.0 + phi;
    for (int i = 0; i < n ; i++) {
        double frac_numerator = lds->get_level(i) + 1.0;
        double power = std::max(floor(frac_numerator / levels_per_group) - 1.0, 0.0);
        coreNumbers[i] = two_plus_lambda * pow(one_plus_phi, power);
    }
    return coreNumbers;
}


// Computing Low Out Degree Ordering
std::vector<int> lowOutDegreeOrdering(LDS* lds, int n) {
    std::vector<int> node_levels(n);
    std::vector<int> ordered_nodes(n);

    for (int i = 0; i < n; i++) {
        node_levels[i] = lds->get_level(i);
    }

    // create a vector of indices (0, 1, 2, ..., n-1)
    std::iota(ordered_nodes.begin(), ordered_nodes.end(), 0);

    // custom comparator to sort
    auto compareNodes = [&node_levels](int a, int b) {
        if (node_levels[a] == node_levels[b]) {
            return a < b; // Break ties using node IDs
        }
        return node_levels[a] < node_levels[b];
    };

    // sort indices based on the custom comparator
    std::sort(ordered_nodes.begin(), ordered_nodes.end(), compareNodes);

    return ordered_nodes;
}
} // end of namespace distributed_kcore

int main(int argc, char** argv) {

    std::string file_loc = argv[1];
    double eta = std::stod(argv[2]);
    double epsilon = std::stod(argv[3]);
    double phi = std::stod(argv[4]);
    distributed_kcore::Graph *graph;

    int factor_id = std::stoi(argv[5]);
    double factor;
    if (factor_id == 0) {
        factor = 1.0 / 4.0;
    } else if (factor_id == 1) {
        factor = 1.0 / 3.0;
    } else if (factor_id == 2) {
        factor = 1.0 / 2.0;
    } else if (factor_id == 3) {
        factor = 2.0 / 3.0;
    } else {
        factor = 3.0 / 4.0;
    }

    int bias = std::stoi(argv[6]);
    int bias_factor = std::stoi(argv[7]);
    int n = std::stoi(argv[8]);
    double one_plus_phi = 1.0 + phi;
    double levels_per_group = ceil(distributed_kcore::log_a_to_base_b(n, one_plus_phi));
    // double lambda = (2.0 / 9.0) * (2.0 * eta - 5.0);
    double lambda = 0.5;
    // double levels_per_group = 15.0;

    int *w_counter, counter;
    MPI_Aint w_size;
    int e_size;
    MPI_Win win;
    MPI_Init(&argc, &argv);
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int numworkers = numProcesses - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;


    if (rank == COORDINATOR) {
        w_size = (MPI_Aint)sizeof(int);
        e_size = sizeof(int);
        MPI_Alloc_mem(sizeof(int),MPI_INFO_NULL,&w_counter);
        *w_counter = 0;
        MPI_Win_create(w_counter, w_size, e_size, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
    } else {
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
    }

    std::vector<double> preprocessing_times;
    std::chrono::time_point<std::chrono::high_resolution_clock> pp_start, pp_end;
    std::chrono::duration<double> pp_elapsed;
    double pp_time = 0.0;
    if (rank  == COORDINATOR) {
        pp_start = std::chrono::high_resolution_clock::now();
        // graph = new distributed_kcore::Graph(file_loc);
        // graph = new distributed_kcore::Graph(file_loc, n);
        pp_end = std::chrono::high_resolution_clock::now();
        pp_elapsed = (pp_end - pp_start);
        pp_time = pp_elapsed.count();
        preprocessing_times.push_back(pp_time);
    } else {
        int offset = (rank - 1) * chunk; 
        int workLoad = (rank == numworkers) ? chunk + extra : chunk;
        pp_start = std::chrono::high_resolution_clock::now();
        // graph = new distributed_kcore::Graph(file_loc, offset, workLoad);
        file_loc = file_loc + std::to_string(rank) + ".txt";
        // std::cout << rank << " | " << file_loc << std::endl;
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
        graph = new distributed_kcore::Graph(file_loc, offset);
        MPI_Win_unlock(0, win);
        // graph->computeStats(file_loc, offset);
        pp_end = std::chrono::high_resolution_clock::now();
        pp_elapsed = (pp_end - pp_start);
        pp_time = pp_elapsed.count();
        preprocessing_times.push_back(pp_time);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double max_pp_time = *std::max_element(preprocessing_times.begin(), preprocessing_times.end());
    

    if (numProcesses < 2) {
        std::cerr << "Error: At least 2 processes are required." << std::endl;
        MPI_Finalize();
        return 1;
    }
     
    if (rank == COORDINATOR) {
        // graph->printDegrees();
        std::cout << "Preprocessing Time: " << max_pp_time << std::endl;
        std::chrono::time_point<std::chrono::high_resolution_clock> algo_start, algo_end;
	    std::chrono::duration<double> algo_elapsed;
        double algo_time = 0.0;
        algo_start = std::chrono::high_resolution_clock::now();
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, eta, epsilon, phi, lambda, static_cast<int>(levels_per_group), factor, bias, bias_factor, n);
        std::vector<double> estimated_core_numbers = distributed_kcore::estimateCoreNumbers(lds, n, eta, phi, lambda, levels_per_group);
        std::vector<int> lowOutOrdering = distributed_kcore::lowOutDegreeOrdering(lds, n);
        algo_end = std::chrono::high_resolution_clock::now();
        algo_elapsed = algo_end - algo_start;
        // std::cout << "Printing Core Numbers" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout<< i << " : " << estimated_core_numbers[i] << std::endl;
        }
        algo_time = algo_elapsed.count();
        std::cout << "Algorithm Time: " << algo_time << std::endl;
    } else {
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, eta, epsilon, phi, lambda, static_cast<int>(levels_per_group), factor, bias, bias_factor, n);
    }
    
    MPI_Finalize();
    return 0;
}
