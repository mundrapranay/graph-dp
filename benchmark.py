import os 
import subprocess
import argparse
import numpy as np
import matplotlib.pyplot as plt 
import statistics
from operator import add




GRAPH_SIZES = {
    'zhang_dblp' : 317080,
    'hua_ctr' : 14081816,
    'hua_livejournal' : 4846609,
    'hua_stackoverflow' : 2584164,
    'hua_usa' : 23947347,
    'hua_youtube' : 1138499,
    'zhang_orkut' : 3072441
}


def run_benchmark():

    # Change directory to build/
    os.chdir('build/')

    # Compile the code using 'make'
    subprocess.run(['make'])

    # Change directory back to the previous directory
    os.chdir('../')

    # graphs = GRAPH_SIZES.keys()
    graphs = ['zhang_dblp', 'zhang_orkut', 'hua_livejournal', 'hua_ctr']
    # graphs = ['zhang_dblp']

    # Specify the number of processes as a command line argument
    num_processes = 17
    eta = 0.9
    epsilon = 0.5
    phi = 0.5
    for graph in graphs:
        # for bias in [0, 1]:
        for bias in [1]:
            for factor_id in range(5):
                for bias_factor in range(1, 21):
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


def run_benchmark_partition():

    # Change directory to build/
    os.chdir('build/')

    # Compile the code using 'make'
    subprocess.run(['make'])

    # Change directory back to the previous directory
    os.chdir('../')

    # graphs = GRAPH_SIZES.keys()
    graphs = ['zhang_dblp']
    # graphs = ['zhang_dblp']

    # Specify the number of processes as a command line argument
    num_processes = 17
    eta = 0.9
    epsilon = 0.5
    phi = 0.5
    for graph in graphs:
        # for bias in [0, 1]:
        for bias in [1]:
            for factor_id in range(5):
                for bias_factor in range(1, 5):
                    output_file = f'/home/ubuntu/results_new/graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor_{bias_factor}_partitioned.txt'
                    if not os.path.exists(output_file):
                        cmd = [
                            'mpirun',
                            '-np', str(num_processes),
                            './build/DistributedGraphAlgorithm',
                            f'./graphs/{graph}_partitioned_{num_processes}/',
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
    try:
        algo_time = float((lines[-1].strip().split(':'))[1].strip())
    except IndexError:
        print(file)
        algo_time = 0
    except ValueError:
        print(file)
        algo_time = 0

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
    graphs = ['zhang_dblp']
    factors = ['1/4', '1/3', '1/2', '2/3', '3/4']
    # factors = ['1/4', '1/3', '1/2']
    for graph in graphs:

        core_numbers = get_ground_truth(graph)
        biased_avg_approx = []

        biased_max_approx = []

        biased_algo_time = []

        biased_pp_time = []

        for bias in [1]:
            for factor_id in range(5):
                bf_bias_avg_approx = []
                bf_bias_max_approx = []
                bf_bias_pp_time = []
                bf_bias_algo_time = []
                for bias_factor in range(1, 21):
                    output_file = f'graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor_{bias_factor}_partitioned_no_noise.txt'
                    approx_core_numbers, pp_time, algo_time = get_core_numbers(output_file)
                    approximation_factor = np.array([float(max(s,t)) / max(1, min(s, t)) for s,t in zip(core_numbers, approx_core_numbers)])
                    # approx_x = np.arange(len(approximation_factor))
                    # approximation_factor = sorted(approximation_factor, reverse=True)
                    # plt.plot(approx_x, approximation_factor, '--o')
                    # plt.xlabel('Node IDs')
                    # plt.ylabel('Approximation Factor')
                    # plt.title('{0}_{1}'.format(factors[factor_id], bias_factor))
                    # plt.tight_layout()
                    # plt.savefig('./figures/{0}_approx_factors_factor_id_{1}_bias_{2}'.format(graph, factor_id, bias_factor))
                    # plt.cla()
                    # plt.clf()
                    bf_bias_avg_approx.append(statistics.mean(approximation_factor))
                    if max(approximation_factor) >= 10e3:
                        print(output_file)
                    bf_bias_max_approx.append(max(approximation_factor))
                    bf_bias_pp_time.append(pp_time)
                    bf_bias_algo_time.append(algo_time)

                biased_avg_approx.append(bf_bias_avg_approx)
                biased_max_approx.append(bf_bias_max_approx)
                biased_pp_time.append(bf_bias_pp_time)
                biased_algo_time.append(bf_bias_algo_time)
        
    x = np.arange(len(biased_avg_approx[0]))
    for i in range(len(factors)):
        if i == 6:
            continue
        else:
            plt.plot(x, biased_avg_approx[i], '--o', label='Average Approx for {0}'.format(factors[i]), alpha=0.7)

    plt.legend()
    # plt.xticks(x)
    plt.xlabel('Bias Subtraction Term')
    plt.ylabel('Approximation Factor')
    plt.title(graph.upper())
    plt.tight_layout()
    plt.savefig('./figures/{0}_avg_approx_factors_bias_partitioned.png'.format(graph))
    plt.cla()
    plt.clf()

    for i in range(len(factors)):
        if i == 6:
            continue
        else:
            plt.plot(x, biased_max_approx[i], '--^', label='Max Approx for {0}'.format(factors[i]), alpha=0.7)
    
    plt.legend()
    # plt.xticks(x)
    plt.xlabel('Bias Subtraction Term')
    plt.ylabel('Approximation Factor')
    plt.title(graph.upper())
    plt.tight_layout()
    plt.savefig('./figures/{0}_max_approx_factors_bias_partitioned.png'.format(graph))
    plt.cla()
    plt.clf()

    for i in range(len(factors)):
        plt.plot(x, list(map(add, biased_pp_time[i], biased_algo_time[i])), '--*', label='Algorithm Time for {0}'.format(factors[i]), alpha=0.7)
    
    plt.legend()
    plt.xlabel('Bias Subtraction Term')
    plt.ylabel('Response Time (seconds)')
    plt.title(graph.upper())
    plt.tight_layout()
    plt.savefig('./figures/{0}_response_time_bias_partitioned.png'.format(graph))
    plt.cla()
    plt.clf()


def get_image_files(factor_id, bias_factor, graph):
    image_list = []
    for f in range(factor_id+1):
        for bias in range(1, bias_factor+1):
            output_file = './figures/{0}_approx_factors_factor_id_{1}_bias_{2}.png'.format(graph, f, bias)
            image_list.append(output_file)
    return image_list

from PIL import Image
def combine_images(columns, space, images, graph):
    rows = len(images) // columns
    if len(images) % columns:
        rows += 1
    width_max = max([Image.open(image).width for image in images])
    height_max = max([Image.open(image).height for image in images])
    background_width = width_max*columns + (space*columns)-space
    background_height = height_max*rows + (space*rows)-space
    background = Image.new('RGBA', (background_width, background_height), (255, 255, 255, 255))
    x = 0
    y = 0
    for i, image in enumerate(images):
        img = Image.open(image)
        x_offset = int((width_max-img.width)/2)
        y_offset = int((height_max-img.height)/2)
        background.paste(img, (x+x_offset, y+y_offset))
        x += width_max + space
        if (i+1) % columns == 0:
            y += height_max + space
            x = 0
    background.save('./figures/{0}_approx_factors_combined_all.png'.format(graph))


def print_core_data():
    graphs = ['hua_livejournal']
    factors = ['1/4', '1/3', '1/2', '2/3', '3/4']
    for graph in graphs:
        core_numbers = get_ground_truth(graph)
        for bias in [1]:
            for factor_id in range(5):
                for bias_factor in range(1, 51):
                    output_file = f'graph_{graph}_factor_id_{factor_id}_bias_{bias}_bias_factor_{bias_factor}_log2.txt'
                    approx_core_numbers, pp_time, algo_time = get_core_numbers(output_file)
                    with open('/home/ubuntu/results_new/core_data/{0}'.format(output_file), 'w') as out:
                        for c, a_c in zip(core_numbers, approx_core_numbers):
                            out.write("{0},{1}\n".format(c, a_c))
                    out.close()

if __name__ == '__main__':
    # run_benchmark()
    run_benchmark_partition()
    # plot_benchmark_runs()
    plot_benchmark_runs_biasfactor()
    # image_list = get_image_files(factor_id=4, bias_factor=50, graph='zhang_orkut')
    # combine_images(50, 20, image_list, 'zhang_orkut')
    # print_core_data()
