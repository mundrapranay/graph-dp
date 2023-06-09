#include <iostream>
#include <vector>
#include <mpi.h>

// Function to perform computation on a node's adjacency list
int computeNodeValue(const std::vector<int>& adjacencyList) {
    // Perform computation on the adjacency list and return the computed value
    int sum = 0;
    for (int neighbor : adjacencyList) {
        // Compute the sum of neighbors (example computation)
        sum += neighbor;
    }
    return sum;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int worldSize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Master node (rank 0) code
        std::vector<std::vector<int>> graph = {
            {1, 2},     // Node 0 adjacency list
            {0, 3},     // Node 1 adjacency list
            {0, 3},     // Node 2 adjacency list
            {1, 2, 4},  // Node 3 adjacency list
            {3, 5},     // Node 4 adjacency list
            {4}         // Node 5 adjacency list
        };

        int numNodes = graph.size() - 1;  // Number of worker nodes
        int nodesPerWorker = numNodes / worldSize;
        int extraNodes = numNodes % worldSize;

        // Distribute work to worker nodes
        for (int dest = 1; dest <= numNodes; dest++) {
            int numNodesToProcess = nodesPerWorker + (dest <= extraNodes ? 1 : 0);
            MPI_Send(&numNodesToProcess, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            for (int i = 0; i < numNodesToProcess; i++) {
                MPI_Send(graph[dest], graph[dest].size(), MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        }

        // Receive computed values from worker nodes
        std::vector<int> allNodeValues(numNodes + 1);
        for (int src = 1; src <= numNodes; src++) {
            MPI_Recv(&allNodeValues[src], 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Output the computed values
        std::cout << "Computed values:" << std::endl;
        for (int i = 0; i <= numNodes; i++) {
            std::cout << "Node " << i << ": " << allNodeValues[i] << std::endl;
        }
    } else {
        // Worker nodes code
        int numNodesToProcess;
        MPI_Recv(&numNodesToProcess, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> localAdjacencyList(numNodesToProcess);
        MPI_Recv(localAdjacencyList.data(), numNodesToProcess, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Perform computation on the local adjacency list
        int nodeValue = computeNodeValue(localAdjacencyList);

        // Send computed value back to the master node
        MPI_Send(&nodeValue, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
