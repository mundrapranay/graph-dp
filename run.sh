#!/bin/sh
# ./build/Distributed filename eta epsilon phi factor_id bias 
cd build/ && make && cd ../ && 
for graph in 'ctr' 'livejournal' 'stackoverflow' 'usa' 'youtube'
do 
    for bias in 0 1
    do 
        for factor_id in 0 1 2 3 4
        do 
            mpirun -np ${1} ./build/DistributedGraphAlgorithm ./graphs/hua_${graph}_insertion_edges 0.9 0.5 0.5 ${factor_id} ${bias} > ./results/graph_${graph}_factor_id_${factor_id}_bias_${bias}.txt
        done
    done
done 