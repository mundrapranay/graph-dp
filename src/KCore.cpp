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
    int number_of_levels = ceil(4 * pow(log(n), 2) - 1);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset, mytype, workLoad, p;
    std::unordered_map<int, std::vector<int>> adjacencyList = graph->getAdjacencyList();
    MPI_Status status;

    if (rank == COORDINATOR) {
        for (int i = 0; i < number_of_levels; i++) {
            levels.push_back(LDS(n, epsilon, delta, false));
        }
    }

    for (int r = 0; r < number_of_levels - 1; r++) {
        // each node either releases 1 or 0 and the coordinator updates the level accordingly
        // nextLevels stores this information
        // int *nextLevels = new int[n];
        int *nextLevels = (int*)malloc(n * sizeof(int)); 
        if (rank == COORDINATOR) {
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
                MPI_Send(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD);
                offset += workLoad;
            }

            // receive results from workers
            for (p = 1; p <= numworkers; p++) {
                mytype = FROM_WORKER + p;
                MPI_Recv(&offset, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&workLoad, 1, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
                MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, p, mytype, MPI_COMM_WORLD, &status);
            }
        } else {
            // worker task
            mytype = FROM_MASTER;
            MPI_Recv(&offset, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&workLoad, 1, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);
            MPI_Recv(&nextLevels[offset], workLoad, MPI_INT, COORDINATOR, mytype, MPI_COMM_WORLD, &status);

            // perform computation
            int end_node = offset + workLoad;
            for (int i = offset; i < end_node; i++) {
                if (levels[r].get_level(i) == r) {
                   int U_i = 0;
                   for (auto ngh : adjacencyList[i]) {
                        if (levels[r].get_level(ngh) == r) {
                            U_i += 1;
                        }
                   }
                   /**
                     * @todo: add google-dp library to sample from Geometric Distribution
                    */
                   int U_hat_i = U_i;
                   int group_index = levels[r].group_for_level(r);
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

        }

        MPI_Barrier(MPI_COMM_WORLD);
        // update the levels based on the data in nextLevels
        if (rank == COORDINATOR) {
            std::cout << "need to compute" << std::endl;
        }
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
    distributed_kcore::Graph *graph;

    
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
        graph = new distributed_kcore::Graph(file_loc);
        std::cout << graph->getGraphSize() << std::endl;
    }
     
    // KCore_compute(rank, numProcesses, graph, nu, epsilon);

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