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


void KCore(int rank, int nprocs, Graph* graph, double nu, double epsilon) {
    std::vector<LDS> levels;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    int n = graph->getGraphSize();
    int number_of_levels = ceil(4 * pow(log(n), 2) - 1);
    int numworkers = nprocs - 1;
    int chunk = n / numworkers;
    int extra = n % numworkers;
    int offset;

    if (rank == COORDINATOR) {
        for (int i = 0; i < number_of_levels; i++) {
            levels.push_back(LDS(n, epsilon, delta, false));
        }
    }

    for (int r = 0; r < number_of_levels - 1; r++) {
        if (rank == COORDINATOR) {
            // distribute the task based on the num_workers
            // calculate the data size to send to workers
            /**
             * @todo: figure out offset value so that each worker 
             *        can decide the nodes to work on.
            */
            int offset = r + 1;
            for (int p = 1; p <= numworkers; p++) {
                int workLoad = (p == numworkers) ? chunk + extra : chunk;

            }
        } else {
            // 
        }
    }

}

int main(int argc, char** argv) {

    // std::string file_loc = argv[1];
    // double nu = std::stod(argv[2]);
    // double epsilon = std::stod(argv[3]);
    std::string file_loc = "sample_graph.tsv";
    double nu = 0.9;
    double epsilon = 0.5;

    Graph *graph = new Graph(file_loc);

    MPI_Init(&argc, &argv);
    int numProcesses, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numProcesses < 2) {
        std::cerr << "Error: At least 2 processes are required." << std::endl;
        MPI_Finalize();
        return 1;
    }

    std::unordered_map<int, std::vector<int>> adjacencyList;
    std::vector<LDS> levels;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    int n = graph->getGraphSize();
    int number_of_levels = ceil(4 * pow(log(n), 2) - 1);

    if (rank == 0) {
        adjacencyList = graph->getAdjacencyList();
        for (int i = 0; i < number_of_levels; i++) {
            levels.push_back(LDS(n, epsilon, delta, false));
        }
    }







    

    std::vector<double> core_numbers = KCore(graph->getAdjacencyList(), nu, epsilon);

    return 0;
}
} // end of namespace distributed_kcore