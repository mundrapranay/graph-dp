#!/bin/sh
# ./build/Distributed filename eta epsilon phi
cd build/ && make && cd ../ && mpirun -np ${1} ./build/DistributedGraphAlgorithm zhang_dblp 0.9 0.5 0.5 > ${2}
