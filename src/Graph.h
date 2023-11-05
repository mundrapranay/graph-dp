
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <numeric>
#include <unordered_map>


namespace distributed_kcore{

class Graph {
    private:
        std::unordered_map<int, std::vector<int>> adjacencyList;
        // std::unordered_map<int, int> nodeDegrees;
        std::vector<int> node_degrees;
        // std::vector<int> ordered_adjacency_list;
        size_t graphSize = 0;
        std::vector<int> ordered_adjacency_list;

        std::vector<std::string> splitString(const std::string& line, char del) {
			std::vector<std::string> result;
			std::stringstream ss(line);
			std::string item;

			while (std::getline(ss, item, del)) {
				result.push_back(item);
			}
			return result;
		}
    
    public:
        // Graph(const std::string& filename) {
        //     std::ifstream file(filename);
        //     if (!file.is_open()) {
        //         std::cerr << "Failed to open file: " << filename << std::endl;
        //         return;
        //     }
        //     std::string line;
        //     while (std::getline(file, line)) {
        //         std::vector<std::string> values = splitString(line, ' ');
        //         std::vector<int> neighbors1;
        //         std::vector<int> neighbors2;
        //         // to ensure that its zero indexed
        //         int vertex = std::stoi(values[0]);
        //         int ngh = std::stoi(values[1]);
        //         // if (adjacenyList.find(vertex) == adjacenyList.end()) {
        //         if (nodeDegrees.find(vertex) == nodeDegrees.end()) {
        //             // adjacenyList[vertex] = neighbors1;
        //             nodeDegrees[vertex] = 0;
        //         }
        //         // if (adjacenyList.find(ngh) == adjacenyList.end()) {
        //         if (nodeDegrees.find(vertex) == nodeDegrees.end()) {
        //             // adjacenyList[ngh] = neighbors2;
        //             nodeDegrees[ngh] = 0;
        //         }
        //         // adjacenyList[vertex].push_back(ngh);
        //         // adjacenyList[ngh].push_back(vertex);
        //         nodeDegrees[vertex]++;
        //         nodeDegrees[ngh]++;
        //     }
        //     file.close();
        //     graphSize = adjacencyList.size();   
        // }

        // Graph(const std::string& filename, int n) {
        //     node_degrees = std::vector<int>(n, 0);
        //     std::ifstream file(filename);
        //     if (!file.is_open()) {
        //         std::cerr << "Failed to open file: " << filename << std::endl;
        //         return;
        //     }
        //     std::string line;
        //     while (std::getline(file, line)) {
        //         std::vector<std::string> values = splitString(line, ' ');
        //         std::vector<int> neighbors1;
        //         std::vector<int> neighbors2;
        //         // to ensure that its zero indexed
        //         int vertex = std::stoi(values[0]);
        //         int ngh = std::stoi(values[1]);
        //         if (adjacencyList.find(vertex) == adjacencyList.end()) {
        //             adjacencyList[vertex] = neighbors1;
        //         }
        //         if (adjacencyList.find(ngh) == adjacencyList.end()) {
        //             adjacencyList[ngh] = neighbors2;
        //         }
        //         adjacencyList[vertex].push_back(ngh);
        //         adjacencyList[ngh].push_back(vertex);
        //         node_degrees[vertex] += 1;
        //         node_degrees[ngh] += 1;
        //     }
        //     file.close();
        //     graphSize = adjacencyList.size();
        //     // std::cout << "Read the graph" << std::endl;
        //     for (int node = 0; node < n; node++) {
        //         ordered_adjacency_list.push_back(node);
        //         ordered_adjacency_list.insert(ordered_adjacency_list.end(), adjacencyList[node].begin(), adjacencyList[node].end());
        //     }
        //     // std::cout << "Read the graph" << std::endl;
        // }

        // Graph(const std::string& filename, int offset, int workLoad) {
        //     std::ifstream file(filename);
        //     std::set<int> workingNodes;
        //     int end_node = offset + workLoad;
        //     for (int i = offset; i < end_node; i++) {
        //         workingNodes.insert(i);
        //     }

        //     if (!file.is_open()) {
        //         std::cerr << "Failed to open file: " << filename << std::endl;
        //         return;
        //     }
        //     std::string line;
        //     while (std::getline(file, line)) {
        //         std::vector<std::string> values = splitString(line, ' ');
        //         std::vector<int> neighbors1;
        //         std::vector<int> neighbors2;
        //         // to ensure that its zero indexed
        //         int vertex = std::stoi(values[0]);
        //         int ngh = std::stoi(values[1]);
        //         if (workingNodes.find(vertex) != workingNodes.end()) {
        //             if (adjacencyList.find(vertex) == adjacencyList.end()) {
        //                 adjacencyList[vertex] = neighbors1;
        //             }
        //             adjacencyList[vertex].push_back(ngh);
        //         }

        //         if (workingNodes.find(ngh) != workingNodes.end()) {
        //             if (adjacencyList.find(ngh) == adjacencyList.end()) {
        //                 adjacencyList[ngh] = neighbors2;
        //             }
        //             adjacencyList[ngh].push_back(vertex);
        //         }
        //     }
        //     file.close();
        //     graphSize = adjacencyList.size();
        //     workingNodes.clear(); 
        // }

        Graph(const std::string& filename, int offset, int workLoad) {
            std::ifstream file(filename);
            std::set<int> workingNodes;
            int end_node = offset + workLoad;
            for (int i = offset; i < end_node; i++) {
                workingNodes.insert(i);
            }
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << filename << std::endl;
                return;
            }
            // std::set<int> nodes;
            std::string line;
            while (std::getline(file, line)) {
                std::vector<std::string> values = splitString(line, ' ');
                std::vector<int> neighbors1;
                std::vector<int> neighbors2;
                // to ensure that its zero indexed
                int vertex = std::stoi(values[0]);
                int ngh = std::stoi(values[1]);
                // if (adjacencyList.find(vertex) == adjacencyList.end()) {
                //     adjacencyList[vertex] = neighbors1;
                //     // nodes.insert(vertex);
                // }
                // adjacencyList[vertex].push_back(ngh);

                // // if (adjacencyList.find(ngh) == adjacencyList.end()) {
                // //     adjacencyList[ngh] = neighbors2;
                // // }
                // // adjacencyList[ngh].push_back(vertex);
                if (workingNodes.find(vertex) != workingNodes.end()) {
                    if (adjacencyList.find(vertex) == adjacencyList.end()) {
                        adjacencyList[vertex] = neighbors1;
                    }
                    adjacencyList[vertex].push_back(ngh);
                }

                if (workingNodes.find(ngh) != workingNodes.end()) {
                    if (adjacencyList.find(ngh) == adjacencyList.end()) {
                        adjacencyList[ngh] = neighbors2;
                    }
                    adjacencyList[ngh].push_back(vertex);
                }
            }
            file.close();
            graphSize = adjacencyList.size();
            node_degrees = std::vector<int>(graphSize, 0);
            int al_size = 0;
            auto it = adjacencyList.begin();
            while (it != adjacencyList.end()) {
                al_size += it->second.size();
                it++;
            }

            graphSize += al_size;
        }

        std::vector<int> computeOAL(int offset) {
            if (ordered_adjacency_list.empty()){
                auto it = adjacencyList.begin();
                ordered_adjacency_list.resize(graphSize);
                while (it != adjacencyList.end()) {
                    int node = it->first;
                    ordered_adjacency_list.push_back(node);
                    ordered_adjacency_list.insert(ordered_adjacency_list.end(), adjacencyList[node].begin(), adjacencyList[node].end());
                    it++;
                }
                adjacencyList.clear();
            } 
            return ordered_adjacency_list;
        }

        std::unordered_map<int, std::vector<int>> getAdjacencyList() {
            return adjacencyList;
        }

        std::vector<int> getNeighbors(int node) {
            return adjacencyList[node];
        }

        std::vector<int> getNodeDegreeVector(int offset) {
            auto it = adjacencyList.begin();
            while (it != adjacencyList.end()) {
                int node = it->first;
                node_degrees[node - offset] = it->second.size();
                it++;
            }
            return node_degrees;
        }

        std::vector<int> getOrderedAdjacencyList() {
            return ordered_adjacency_list;
        }

        int getNodeDegree(int node) {
            // return nodeDegrees[node];
            return node_degrees[node];
        }

        size_t getGraphSize() {
            return graphSize;
        }
};

} // end of namespace distributed_kcore
