/**
 * @file kCore.cpp
 * @brief Approximate KCore Distributed Algorithm
*/

#include <math.h>
#include <mpi.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "LDS.h"
#include "Graph.h"

// using namespace std;
// using namespace LDS;
#define COORDINATOR 0 
#define FROM_MASTER 1
#define FROM_WORKER 2

namespace distributed_kcore{


std::vector<double> KCore(std::unordered_map<int, std::vector<int>> adjacencyLists, double nu, double epsilon) {
    std::vector<double> core_numbers;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    std::vector<LDS> levels;
    int n = adjacencyLists.size();
    int number_of_levels = ceil(4 * pow(log(n), 2) - 1);
    for (int i = 0; i < number_of_levels; i++) {
        levels.push_back(LDS(n, epsilon, delta, false));
    }

    return core_numbers;
}


void KCore_compute(int rank, int nprocs, Graph* graph, double nu, double epsilon) {
    std::vector<LDS> levels;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    int n = graph->getGraphSize();
    std::cout << "graph size: " << n << std::endl;
    int number_of_levels = ceil(4 * pow(log(n), 2) - 1);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset, mytype, workLoad, p;
    std::unordered_map<int, std::vector<int>> adjacencyList = graph->getAdjacencyList();
    MPI_Status status;

    if (rank == COORDINATOR) {
        /**
         * @todo: 
         *  - only one LDS and that contains a vector<LDSVertex> L that keeps
         * track of level and all other info (adjacency list)
        */
        for (int i = 0; i < number_of_levels; i++) {
            levels.push_back(LDS(n, epsilon, delta, false));
        }
    }
    /**
     * @todo:
     *  - add all neighbours LDS->L[node].insert_neighbour(ngh_id)
     *         - batch_insertion (check from k_core-approx code)
     *         - EdgeOrientation/ParallelLDS/LDS.h (insert_neighbour)
     *         - Levels Class from EdgeOrientation/ParallelLDS/LDS.h copy to LDS.h 
    */
    // for (int i = 0; i < number_of_levels; i++) {
    //     levels.push_back(LDS(n, epsilon, delta, false));
    // }

    for (int r = 0; r < number_of_levels - 1; r++) {
        // each node either releases 1 or 0 and the coordinator updates the level accordingly
        // nextLevels stores this information
        // int *nextLevels = new int[n];
        // int *currentLevels = (int*)malloc(n * sizeof(int));
        std::vector<int> currentLevels(n);
        std::vector<int> nextLevels(n);
        int group_index; 
        if (rank == COORDINATOR) {
            for (int node = 0; node < n; node++) {
                currentLevels[node] = levels[r].get_level(node);
            }
            MPI_Bcast(currentLevels.data(), currentLevels.size(), MPI_INT, COORDINATOR, MPI_COMM_WORLD);
            std::cout << "Broadcasted current levels of size: " << currentLevels.size() << std::endl;
            group_index = levels[r].group_for_level(r);
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
                MPI_Send(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD);
                offset += workLoad;
            }
            std::cout << "Sent from Master" << std::endl;

            // receive results from workers
            for (p = 1; p <= numworkers; p++) {
                mytype = FROM_WORKER + p;
                MPI_Recv(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
            }
            std::cout << "Received at Master" << std::endl;
            std::cout<< "NextLevles Len: " << nextLevels.size() << std::endl;
        } else {
            // worker task
            mytype = FROM_MASTER;
            MPI_Recv(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&group_index, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            std::cout << "Received at Worker: " << rank << std::endl;
            std::cout << "Received at Worker: " << rank << " size of currentLevels: " << currentLevels.size() << std::endl;
            // perform computation
            int end_node = offset + workLoad;
            for (int i = offset; i < end_node; i++) {
                if (currentLevels[i] == r) {
                   int U_i = 0;
                //    std::cout << "Inside Loop Worker: " << rank << std::endl;
                   for (auto ngh : adjacencyList[i]) {
                        // std::cout << "Worker: " << rank << " NGH: " << ngh << std::endl;
                        if (currentLevels[ngh] == r) {
                            U_i += 1;
                        }
                   }
                   /**
                     * @todo: add google-dp library to sample from Geometric Distribution
                     * @todo: make sure the base of log is correct
                    */
                   int U_hat_i = U_i;
                   if (U_hat_i > pow((1 + phi), group_index)) {
                        nextLevels[i] = 1;
                   }
                }
            }

            // send back the completed data to COORDINATOR
            mytype = FROM_WORKER + rank;
            MPI_Send(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            MPI_Send(&nextLevels[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD);
            std::cout << "Sent from Worker: " << rank << std::endl;

        }

        MPI_Barrier(MPI_COMM_WORLD);
        // update the levels based on the data in nextLevels
        if (rank == COORDINATOR) {
            for (int i = 0; i < nextLevels.size(); i++) {
                if (nextLevels[i] == 1) {
                    // levels[r+1].L[i].level = levels[r].get_level(i) + 1;
                    // LDS->L.level_increase(i, LDS->L);
                    
                    levels[r+1].level_increase(i, levels[r].L);
                    std::cout << "level increased" << std::endl;
                } else {
                    std::cout << "level decreased" << std::endl;
                }
            }
        }

        // wait until COORDINATOR has computed the next level
        MPI_Barrier(MPI_COMM_WORLD);
    }

}

} // end of namespace distributed_kcore

int main(int argc, char** argv) {

    // std::string file_loc = argv[1];
    // double nu = std::stod(argv[2]);
    // double epsilon = std::stod(argv[3]);
    std::string file_loc = "dblp_greedy";
    double nu = 0.9;
    double epsilon = 0.5;
    distributed_kcore::Graph *graph = new distributed_kcore::Graph(file_loc);

    
    MPI_Init(&argc, &argv);
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numProcesses < 2) {
        std::cerr << "Error: At least 2 processes are required." << std::endl;
        MPI_Finalize();
        return 1;
    }
    // if (rank == COORDINATOR) {
    //     graph = new distributed_kcore::Graph(file_loc);
    //     std::cout << graph->getGraphSize() << std::endl;
    // }
     
    distributed_kcore::KCore_compute(rank, numProcesses, graph, nu, epsilon);

    // std::unordered_map<int, std::vector<int>> adjacencyList;
    // std::vector<LDS> levels;
    // double phi = 0.5;
    // double lambda = (2/9) * (2 * nu - 5);
    // double delta = 9.0;
    // int n = graph->getGraphSize();
    // int number_of_levels = ceil(4 * pow(log(n), 2) - 1);

    // if (rank == 0) {
    //     adjacencyList = graph->getAdjacencyList();
    //     for (int i = 0; i < number_of_levels; i++) {
    //         levels.push_back(LDS(n, epsilon, delta, false));
    //     }
    // }







    

    // std::vector<double> core_numbers = KCore(graph->getAdjacencyList(), nu, epsilon);
    MPI_Finalize();
    return 0;
}