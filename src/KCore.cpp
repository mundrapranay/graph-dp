

#include <math.h>
#include <mpi.h>
#include <vector>
#include "LDS.h"

// using namespace std;
// using namespace LDS;




std::vector<double> KCore(std::vector<std::vector<int>> adjacencyLists, double nu, double epsilon) {
    std::vector<double> core_numbers;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    LDS::LDS *lds = new LDS::LDS(adjacencyLists.size(), epsilon, delta, false);
    


    return core_numbers;
}