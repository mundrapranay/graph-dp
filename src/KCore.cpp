

#include <math.h>
#include <mpi.h>
#include <vector>
#include "LDS.h"

// using namespace std;
// using namespace LDS;

namespace distributed_kcore{


std::vector<double> KCore(std::vector<std::vector<int>> adjacencyLists, double nu, double epsilon) {
    std::vector<double> core_numbers;
    double phi = 0.5;
    double lambda = (2/9) * (2 * nu - 5);
    double delta = 9.0;
    LDS *lds = new LDS(adjacencyLists.size(), epsilon, delta, false);
    


    return core_numbers;
}

} // end of namespace distributed_kcore