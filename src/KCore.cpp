/**
 * @file kCore.cpp
 * @brief Approximate KCore Distributed Algorithm
*/

#include <math.h>
#include <mpi.h>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
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

LDS* KCore_compute(int rank, int nprocs, Graph* graph, double eta, double epsilon, double phi, int levels_per_group) {
    double lambda = (2/9) * (2 * eta - 5);
    double delta = 9.0;
    int n = graph->getGraphSize();
    // std::cout << "graph size: " << n << std::endl;
    double rounds_param = ceil(4.0 * pow(log_a_to_base_b(n, 1.0 + phi), 1.5));
    int number_of_rounds = static_cast<int>(rounds_param);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset, mytype, workLoad, p;
    std::unordered_map<int, std::vector<int>> adjacencyList = graph->getAdjacencyList();
    MPI_Status status;
    LDS *lds;

    if (rank == COORDINATOR) {
        lds = new LDS(n, phi, delta, levels_per_group, false);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    std::vector<int> permanentZeros(n, 1);

    for (int r = 0; r < number_of_rounds - 2; r++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> round_start, round_end;
	    std::chrono::duration<double> round_elapsed;
        double round_time = 0.0;
        // each node either releases 1 or 0 and the coordinator updates the level accordingly
        // nextLevels stores this information
        round_start = std::chrono::high_resolution_clock::now();
        std::vector<int> currentLevels(n);
        std::vector<int> nextLevels(n, 0);
        int group_index; 
        if (rank == COORDINATOR) {
            for (int node = 0; node < n; node++) {
                currentLevels[node] = lds->get_level(node);
            }
            group_index = lds->group_for_level(r);

            /**
             * @todo: figure out offset value so that each worker 
             *        can decide the nodes to work on.
            */
            offset = 0;
            mytype = FROM_MASTER;
            for (p = 1; p <= numworkers; p++) {
                workLoad = (p == numworkers) ? chunk + extra : chunk;
                MPI_Send(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&group_index, 1, MPI_INT, p, mytype, MPI_COMM_WORLD);
                MPI_Send(&currentLevels[0], currentLevels.size(), MPI_INT, p, mytype, MPI_COMM_WORLD);
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
                    lds->level_increase_v2(i, lds->L);
                } 
            }
        } else {
            // worker task
            mytype = FROM_MASTER;
            MPI_Recv(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&group_index, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&currentLevels[0], currentLevels.size(), MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);

            // perform computation
            int end_node = offset + workLoad;
            for (int i = offset; i < end_node; i++) {
                if (currentLevels[i] == r && permanentZeros[i] != 0) {
                   int U_i = 0;
                   for (auto ngh : adjacencyList[i]) {
                        if (currentLevels[ngh] == r) {
                            U_i += 1;
                        }
                   }

                   double lambda = epsilon / (2.0 * rounds_param);
                   GeometricDistribution* geom = new GeometricDistribution(lambda);
                   int noise = geom->Sample();
                   int U_hat_i = U_i + noise;
                   if (U_hat_i > pow((1 + phi), group_index)) {
                        nextLevels[i] = 1;
                   } else {
                        permanentZeros[i] = 0;
                   }
                }
            }

            // send back the completed data to COORDINATOR
            mytype = FROM_WORKER + rank;
            MPI_Send(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&nextLevels[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&permanentZeros[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);

        }

        MPI_Barrier(MPI_COMM_WORLD);
        round_end = std::chrono::high_resolution_clock::now();
        round_elapsed = round_end - round_start;
        round_time = round_elapsed.count();
        // if (rank == COORDINATOR) {
        //     std::cout << "Round " << r << " | " << number_of_levels - 2 << std::endl;
        //     std::cout << "Round time: " << round_time << std::endl;
        // }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return lds;
}

// Computing Approximate Core Numbers
std::vector<double> estimateCoreNumbers(LDS* lds, int n, double eta, double phi, double levels_per_group) {
    std::vector<double> coreNumbers(n);
    double lambda = (2.0 / 9.0) * (2.0 * eta - 5.0);
    double two_plus_lambda = 2.0 + lambda;
    double one_plus_phi = 1.0 + phi;
    for (int i = 0; i < n ; i++) {
        double frac_numerator = lds->get_level(i) + 1.0;
        double power = std::max(floor(frac_numerator / levels_per_group) - 1.0, 0.0);
        coreNumbers[i] = two_plus_lambda * pow(one_plus_phi, power);
    }
    return coreNumbers;
}


} // end of namespace distributed_kcore

int main(int argc, char** argv) {

    std::string file_loc = argv[1];
    double eta = std::stod(argv[2]);
    double epsilon = std::stod(argv[3]);
    double phi = std::stod(argv[4]);
    distributed_kcore::Graph *graph = new distributed_kcore::Graph(file_loc);
    int n = graph->getGraphSize();
    double one_plus_phi = 1.0 + phi;
    double levels_per_group = ceil(distributed_kcore::log_a_to_base_b(n, one_plus_phi));
    // double levels_per_group = 15.0;

    
    MPI_Init(&argc, &argv);
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numProcesses < 2) {
        std::cerr << "Error: At least 2 processes are required." << std::endl;
        MPI_Finalize();
        return 1;
    }
     
    if (rank == COORDINATOR) {
        // graph->printDegrees();
        // std::cout << "AdjListSum: " << graph->sumAdjList() << std::endl;
        std::chrono::time_point<std::chrono::high_resolution_clock> algo_start, algo_end;
	    std::chrono::duration<double> algo_elapsed;
        double algo_time = 0.0;
        algo_start = std::chrono::high_resolution_clock::now();
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, eta, epsilon, phi, static_cast<int>(levels_per_group));
        std::vector<double> estimated_core_numbers = distributed_kcore::estimateCoreNumbers(lds, n, eta, phi, levels_per_group);
        algo_end = std::chrono::high_resolution_clock::now();
        algo_elapsed = algo_end - algo_start;
        // std::cout << "Printing Core Numbers" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout<< i << " : " << estimated_core_numbers[i] << std::endl;
        }
        algo_time = algo_elapsed.count();
        std::cout << "Algorithm Time: " << algo_time << std::endl;
    } else {
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, eta, epsilon, phi, static_cast<int>(levels_per_group));
    }
    
    MPI_Finalize();
    return 0;
}