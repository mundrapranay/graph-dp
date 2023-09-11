import os 
import subprocess
import argparse
import numpy as np
import matplotlib.pyplot as plt 
import statistics




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

    # graphs = GRAPH_SIZES.keys()
    graphs = ['hua_livejournal']
    # graphs = ['zhang_dblp']

    # Specify the number of processes as a command line argument
    num_processes = 17
    eta = 0.9
    epsilon = 0.5
    phi = 0.5
    for graph in graphs:
        # for bias in [0, 1]:
        for bias in [1]:
            for factor_id in range(1, 6):
                for bias_factor in range(1, 23):
                    output_file = f'/home/ubuntu/results_new/graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor_{bias_factor}_log2.txt'
                    if not os.path.exists(output_file):
                        cmd = [
                            'mpirun',
                            '-np', str(num_processes),
                            './build/DistributedGraphAlgorithm',
                            f'./graphs/{graph}',
                            str(eta), str(epsilon), str(phi),
                            str(factor_id), str(bias), str(bias_factor), str(GRAPH_SIZES[graph])
                        ]
                        
                        
                        with open(output_file, 'w') as f:
                            subprocess.run(cmd, stdout=f)

                    print(f'done with graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor{bias_factor}')


def get_max_approx_index(pairs):
    # Define a custom key function to extract the 'approx' value from each pair
    def get_approx(pair):
        return pair[1]

    # Find the pair with the maximum 'approx' value using the custom key function
    max_pair = max(pairs, key=get_approx)

    # Find the index of the max_pair in the original list
    max_index = pairs.index(max_pair)

    return max_index

def get_core_numbers(file):
    f = open('/home/ubuntu/results_new/{0}'.format(file), 'r')
    lines = f.readlines()
    pp_time = float((lines[0].strip().split(':'))[1].strip())
    algo_time = float((lines[-1].strip().split(':'))[1].strip())
    del lines[-1]
    del lines[0]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    estimated_core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        estimated_core_numbers.append(cn)
    return estimated_core_numbers, pp_time, algo_time

def get_ground_truth(graph):
    f = open('/home/ubuntu/ground_truth/{0}_cores'.format(graph), 'r')
    lines = f.readlines()
    f.close()
    lines = [line.strip().split(' ') for line in lines]
    core_numbers = []
    for line in lines:
        try:
            cn = float(line[1].strip())
            core_numbers.append(cn)
        except ValueError:
            continue
    return core_numbers

'''
    @todo: what if the file is empty, skip the data point
'''

def plot_benchmark_runs():
    graphs = ['zhang_dblp']
    factors = ['1/4', '1/3', '1/2', '2/3', '3/4']
    for graph in graphs:

        core_numbers = get_ground_truth(graph)
        biased_avg_approx = []
        unbiased_avg_approx = []

        biased_max_approx = []
        unbiased_max_approx = []

        biased_algo_time = []
        unbiased_algo_time = []

        biased_pp_time = []
        unbiased_pp_time = []

        for bias in [0, 1]:
            for factor_id in range(5):
                output_file = f'graph_{graph}_factor_id_{factor_id}_bias_{bias}.txt'
                approx_core_numbers, pp_time, algo_time = get_core_numbers(output_file)
                approximation_factor = np.array([float(max(s,t)) / min(s, t) for s,t in zip(core_numbers, approx_core_numbers)])
                if bias == 0:
                    unbiased_avg_approx.append(statistics.mean(approximation_factor))
                    unbiased_max_approx.append(max(approximation_factor))
                    unbiased_pp_time.append(pp_time)
                    unbiased_algo_time.append(algo_time)
                else:
                    biased_avg_approx.append(statistics.mean(approximation_factor))
                    biased_max_approx.append(max(approximation_factor))
                    biased_pp_time.append(pp_time)
                    biased_algo_time.append(algo_time)
        
        x = np.arange(len(factors))
        plt.plot(x, unbiased_avg_approx, 'o', label='Unbiased Average Approx Factor')
        plt.plot(x, biased_avg_approx, '^', label='Biased Average Approx Factor')
        plt.plot(x, unbiased_max_approx, 's', label='Unbiased Max Approx Factor')
        plt.plot(x, biased_max_approx, '*', label='Biased Max Approx Factor')
        plt.legend()
        plt.xticks(x, factors)
        plt.xlabel('Privacy Budget Distribution')
        plt.ylabel('Approximation Factor')
        plt.title(graph.upper())
        plt.tight_layout()
        plt.savefig('./figures/{0}_approx_factors.png'.format(graph))
        plt.cla()
        plt.clf()

        # plot algotime
        width = 0.3
        plt.bar(x, unbiased_pp_time, width, label='Unbiased Preprocessing Time')
        plt.bar(x, unbiased_algo_time, bottom=unbiased_pp_time, width=width, label = 'Unbiased Algorithm Time')
        plt.bar(x + width, biased_pp_time, width, label='Biased Preprocessing Time')
        plt.bar(x + width, biased_algo_time, bottom=unbiased_pp_time, width=width, label = 'Based Algorithm Time')
        plt.legend()
        plt.xticks(x + width / 2, factors)
        plt.xlabel('Privacy Budget Distribution')
        plt.ylabel('Time (seconds)')
        plt.title(graph.upper())
        plt.tight_layout()
        plt.savefig('./figures/{0}_algotime.png'.format(graph))
        plt.cla()
        plt.clf()


def plot_benchmark_runs_biasfactor():
    graphs = ['hua_ctr']
    # factors = ['1/4', '1/3', '1/2', '2/3', '3/4']
    factors = ['1/4', '1/3', '1/2']
    for graph in graphs:

        core_numbers = get_ground_truth(graph)
        biased_avg_approx = []

        biased_max_approx = []

        biased_algo_time = []

        biased_pp_time = []

        for bias in [1]:
            for factor_id in range(4):
                bf_bias_avg_approx = []
                bf_bias_max_approx = []
                bf_bias_pp_time = []
                bf_bias_algo_time = []
                for bias_factor in range(1, 31):
                    output_file = f'graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor_{bias_factor}_log2.txt'
                    approx_core_numbers, pp_time, algo_time = get_core_numbers(output_file)
                    approximation_factor = np.array([float(max(s,t)) / min(s, t) for s,t in zip(core_numbers, approx_core_numbers)])
                    bf_bias_avg_approx.append(statistics.mean(approximation_factor))
                    bf_bias_max_approx.append(max(approximation_factor))
                    bf_bias_pp_time.append(pp_time)
                    bf_bias_algo_time.append(algo_time)

                biased_avg_approx.append(bf_bias_avg_approx)
                biased_max_approx.append(bf_bias_max_approx)
                biased_pp_time.append(bf_bias_pp_time)
                biased_algo_time.append(bf_bias_algo_time)
        
    x = np.arange(len(biased_avg_approx[0]))
    for i in range(len(factors)):
        plt.plot(x, biased_avg_approx[i], '-o', label='Average Approx for {0}'.format(factors[i]))

    plt.legend()
    # plt.xticks(x)
    plt.xlabel('Bias Subtraction Term')
    plt.ylabel('Approximation Factor')
    plt.title(graph.upper())
    plt.tight_layout()
    plt.savefig('./figures/{0}_avg_approx_factors_bias_log2.png'.format(graph))
    plt.cla()
    plt.clf()

    for i in range(len(factors)):
        plt.plot(x, biased_max_approx[i], '-^', label='Max Approx for {0}'.format(factors[i]))
    
    plt.legend()
    # plt.xticks(x)
    plt.xlabel('Bias Subtraction Term')
    plt.ylabel('Approximation Factor')
    plt.title(graph.upper())
    plt.tight_layout()
    plt.savefig('./figures/{0}_max_approx_factors_bias_log2.png'.format(graph))
    plt.cla()
    plt.clf()


if __name__ == '__main__':
    run_benchmark()
    # plot_benchmark_runs()
    # plot_benchmark_runs_biasfactor()
