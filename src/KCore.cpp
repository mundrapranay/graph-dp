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

// using namespace std;
// using namespace LDS;
#define COORDINATOR 0 
#define FROM_MASTER 1
#define FROM_WORKER 2

namespace distributed_kcore{


int log_a_to_base_b(int a, double b) {
    // log_b a = log_2 a / log_2 b
    return log2(a) / log2(b);
}

LDS* KCore_compute(int rank, int nprocs, Graph* graph, double nu, double epsilon) {
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    int n = graph->getGraphSize();
    std::cout << "graph size: " << n << std::endl;
    int number_of_levels = ceil(4 * pow(log_a_to_base_b(n, 1 + phi), 2) - 1);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset, mytype, workLoad, p;
    std::unordered_map<int, std::vector<int>> adjacencyList = graph->getAdjacencyList();
    MPI_Status status;
    LDS *lds;

    if (rank == COORDINATOR) {
        lds = new LDS(n, phi, delta, false);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    std::vector<int> permanentZeros(n, 1);

    for (int r = 0; r < number_of_levels - 1; r++) {
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
            // distribute the task based on the num_workers
            // calculate the data size to send to workers
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
                // MPI_Send(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD);
                offset += workLoad;
            }
            // std::cout << "Sent from Master" << std::endl;
            // receive results from workers
            for (p = 1; p <= numworkers; p++) {
                mytype = FROM_WORKER + p;
                MPI_Recv(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&permanentZeros[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                // MPI_Recv(&recv_nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
            }
            // std::cout << "Received at Master" << std::endl;
            // std::cout<< "NextLevles Len: " << nextLevels.size() << std::endl;
            /**
             * @todo : nextLevel history, if at round r nextLevel[i] == 0, then 
             *          the node i doesn't participate in round r+1
            */
            // update the levels based on the data in nextLevels
            for (int i = 0; i < nextLevels.size(); i++) {
                // std::cout << i << std::endl;
                if (nextLevels[i] == 1 && permanentZeros[i] != 0) {
                    // LDS->L.level_increase(i, LDS->L);
                    lds->level_increase_v2(i, lds->L);
                    // levels[r+1].level_increase(i, levels[r].L);
                    // std::cout << "level increased" << std::endl;
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
                if (currentLevels[i] == r) {
                   int U_i = 0;
                   for (auto ngh : adjacencyList[i]) {
                        if (currentLevels[ngh] == r) {
                            U_i += 1;
                        }
                   }
                   /**
                     * @todo: add google-dp library to sample from Geometric Distribution
                    */
                   int U_hat_i = U_i;
                   if (U_hat_i > pow((1 + phi), group_index)) {
                        nextLevels[i] = 1;
                   } else {
                        // nextLevels[i] = 0;
                        permanentZeros[i] = 0;
                        /**
                         * @todo : flag to point permanent zero
                        */
                   }
                }
            }

            // send back the completed data to COORDINATOR
            mytype = FROM_WORKER + rank;
            MPI_Send(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&nextLevels[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&permanentZeros[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            // std::cout << "Sent from Worker: " << rank << std::endl;

        }

        MPI_Barrier(MPI_COMM_WORLD);
        round_end = std::chrono::high_resolution_clock::now();
        round_elapsed = round_end - round_start;
        round_time = round_elapsed.count();
        if (rank == COORDINATOR) {
            std::cout << "Round " << r << " | " << number_of_levels - 2 << std::endl;
            std::cout << "Round time: " << round_time << std::endl;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return lds;
}

std::vector<double> estimateCoreNumbers(LDS* lds, int n, double nu) {
    // std::cout << "Computing Core Numbers"  << std::endl;
    std::vector<double> coreNumbers(n);
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double first_term = 2.0 + lambda;
    double second_term = 1.0 + phi;
    for (int i = 0; i < n; i++) {
        double frac_denom = 4 * ceil(log_a_to_base_b(n, second_term));
        int frac_numer = lds->get_level(i) + 1;
        int power = std::max(int(floor(frac_numer / frac_denom)) - 1, 0);
        coreNumbers[i] = first_term * pow(second_term, power);
    }

    return coreNumbers;
}


} // end of namespace distributed_kcore

int main(int argc, char** argv) {

    // std::string file_loc = argv[1];
    // double nu = std::stod(argv[2]);
    // double epsilon = std::stod(argv[3]);
    std::string file_loc = "dblp_0_index_v2";
    double nu = 0.9;
    double epsilon = 0.5;
    distributed_kcore::Graph *graph = new distributed_kcore::Graph(file_loc);
    int n = graph->getGraphSize();

    
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
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, nu, epsilon);
        std::vector<double> estimated_core_numbers = distributed_kcore::estimateCoreNumbers(lds, n, nu);
        std::cout << "Printing Core Numbers" << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout<< i << " : " << estimated_core_numbers[i] << std::endl;
        }
    } else {
        distributed_kcore::LDS* lds = distributed_kcore::KCore_compute(rank, numProcesses, graph, nu, epsilon);
    }
    
    MPI_Finalize();
    return 0;
}