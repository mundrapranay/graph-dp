
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>


namespace distributed_kcore{

class Graph {
    private:
        std::unordered_map<int, std::vector<int>> adjacenyList;
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
        // Graph(const std::string& filename) {
        //     std::ifstream file(filename);
        //     if (!file.is_open()) {
        //         std::cerr << "Failed to open file: " << filename << std::endl;
        //         return;
        //     }
        //     std::string line;
        //     while (std::getline(file, line)) {
        //         std::vector<std::string> values = splitString(line, '\t');
        //         std::vector<int> neighbors;
        //         int vertex = std::stoi(values[0]);
        //         for (int i = 1; i < values.size(); i++) {
        //             neighbors.push_back(std::stoi(values[i]));
        //         }
        //         adjacenyList[vertex] = neighbors;
        //         graphSize += 1;
        //     }
        //     file.close();
            
        // }

        Graph(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << filename << std::endl;
                return;
            }
            std::string line;
            bool firstLine = true;
            while (std::getline(file, line)) {
                if (firstLine) {
                    firstLine = false;
                    continue;
                }
                std::vector<std::string> values = splitString(line, ' ');
                std::vector<int> neighbors;
                // to ensure that its zero indexed
                int vertex = std::stoi(values[0]) - 1;
                int ngh = std::stoi(values[1]) - 1;
                if (adjacenyList.find(vertex) == adjacenyList.end()) {
                    adjacenyList[vertex] = neighbors;
                }
                if (adjacenyList.find(ngh) == adjacenyList.end()) {
                    adjacenyList[ngh] = neighbors;
                }
                adjacenyList[vertex].push_back(ngh);
            }
            file.close();
            graphSize = adjacenyList.size();   
        }

        std::unordered_map<int, std::vector<int>> getAdjacencyList() {
            return adjacenyList;
        }

        size_t getGraphSize() {
            return graphSize;
        }
};

} // end of namespace distributed_kcore
