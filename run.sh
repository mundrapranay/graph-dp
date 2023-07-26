#!/bin/sh
cd build/ && make && cd ../ && mpirun -np ${1} ./build/DistributedGraphAlgorithm > ${2}
