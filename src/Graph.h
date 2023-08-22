
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <unordered_map>


namespace distributed_kcore{

class Graph {
    private:
        std::unordered_map<int, std::vector<int>> adjacenyList;
        std::unordered_map<int, int> nodeDegrees;
        size_t graphSize = 0;

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
        Graph(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << filename << std::endl;
                return;
            }
            std::string line;
            while (std::getline(file, line)) {
                std::vector<std::string> values = splitString(line, ' ');
                std::vector<int> neighbors1;
                std::vector<int> neighbors2;
                // to ensure that its zero indexed
                int vertex = std::stoi(values[0]);
                int ngh = std::stoi(values[1]);
                if (adjacenyList.find(vertex) == adjacenyList.end()) {
                    // adjacenyList[vertex] = neighbors1;
                    nodeDegrees[vertex] = 0;
                }
                if (adjacenyList.find(ngh) == adjacenyList.end()) {
                    // adjacenyList[ngh] = neighbors2;
                    nodeDegrees[ngh] = 0;
                }
                // adjacenyList[vertex].push_back(ngh);
                // adjacenyList[ngh].push_back(vertex);
                nodeDegrees[vertex]++;
                nodeDegrees[ngh]++;
            }
            file.close();
            graphSize = adjacenyList.size();   
        }

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
            std::string line;
            while (std::getline(file, line)) {
                std::vector<std::string> values = splitString(line, ' ');
                std::vector<int> neighbors1;
                std::vector<int> neighbors2;
                // to ensure that its zero indexed
                int vertex = std::stoi(values[0]);
                int ngh = std::stoi(values[1]);
                if (workingNodes.find(vertex) != workingNodes.end()) {
                    if (adjacenyList.find(vertex) == adjacenyList.end()) {
                        adjacenyList[vertex] = neighbors1;
                    }
                    adjacenyList[vertex].push_back(ngh);
                }

                if (workingNodes.find(ngh) != workingNodes.end()) {
                    if (adjacenyList.find(ngh) == adjacenyList.end()) {
                        adjacenyList[ngh] = neighbors2;
                    }
                    adjacenyList[ngh].push_back(vertex);
                }
            }
            file.close();
            graphSize = adjacenyList.size();
            workingNodes.clear(); 
        }

        std::unordered_map<int, std::vector<int>> getAdjacencyList() {
            return adjacenyList;
        }

        std::unordered_map<int, int> getNodeDegrees() {
            return nodeDegrees;
        }

        size_t getGraphSize() {
            return graphSize;
        }

        int sumAdjList() {
            int sum = 0;
            for (auto it : adjacenyList) {
                sum += it.second.size();
            }
            return sum;
        }

        void printDegrees() {
            std::unordered_map<int, std::vector<int>>::iterator it;
            for (it = adjacenyList.begin(); it != adjacenyList.end(); it++) {
                int node = it->first;
                std::vector<int> nghs = it->second;
                std::cout << "Node: " << node << " | Degree : " << nghs.size() << std::endl;
            }
        }
};

} // end of namespace distributed_kcore
