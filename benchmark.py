import os 
import subprocess
import argparse






GRAPH_SIZES = {
    'zhang_dblp' : 317080,
    'hua_ctr' : 14081816,
    'hua_livejournal' : 4846609,
    'hua_stackoverflow' : 2584164,
    'hua_usa' : 23947347,
    'hua_youtube' : 1138499
}


def run_benchmark():

    # Change directory to build/
    os.chdir('build/')

    # Compile the code using 'make'
    subprocess.run(['make'])

    # Change directory back to the previous directory
    os.chdir('../')

    graphs = ['zhang_dblp', 'hua_ctr', 'hua_livejournal', 'hua_stackoverflow', 'hua_usa', 'hua_youtube']

    # Specify the number of processes as a command line argument
    num_processes = 17
    eta = 0.9
    epsilon = 0.5
    phi = 0.5
    for graph in graphs:
        for bias in [0, 1]:
            for factor_id in range(5):
                cmd = [
                    'mpirun',
                    '-np', str(num_processes),
                    './build/DistributedGraphAlgorithm',
                    f'./graphs/{graph}',
                    str(eta), str(epsilon), str(phi),
                    str(factor_id), str(bias), str(GRAPH_SIZES[graph])
                ]
                output_file = f'./results_new/graph_{graph}_factor_id_{factor_id}_bias_{bias}.txt'
                
                with open(output_file, 'w') as f:
                    subprocess.run(cmd, stdout=f)

                print(f'done with graph_{graph}_factor_id_{factor_id}_bias_{bias}')



if __name__ == '__main__':
    run_benchmark()