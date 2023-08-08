from collections import defaultdict
import matplotlib.pyplot as plt 
import numpy as np
import statistics
import pandas as pd
import seaborn as sns


def load_graph():
    f = open('dblp_0_index', 'r')
    lines = f.readlines()
    del lines[0]
    lines = [line.strip() for line in lines]
    f.close()
    data = defaultdict(list)
    for l in lines:
        edge = l.split(' ')
        n1 = int(edge[0])
        n2 = int(edge[1])
        data[n1].append(n2)
        data[n2].append(n1)
    
    for k,v in data.items():
        print('Node: {0} | AdjacencyListSize: {1}'.format(k, len(v)))
        


def preprocess_data():
    f = open('dblp_insertion_edges', 'r')
    lines = f.readlines()
    f.close()
    lines = [line.strip().split(' ') for line in lines]
    with open('dblp_0_index_v2', 'w') as out:
        for line in lines:
            out.write('{0} {1}\n'.format(line[1], line[2]))
    out.close()


def get_max_approx_index(pairs):
    # Define a custom key function to extract the 'approx' value from each pair
    def get_approx(pair):
        return pair[1]

    # Find the pair with the maximum 'approx' value using the custom key function
    max_pair = max(pairs, key=get_approx)

    # Find the index of the max_pair in the original list
    max_index = pairs.index(max_pair)

    return max_index

def core_numbers_distribution():
    f = open('dblp_cores', 'r')
    lines = f.readlines()
    # del lines[-1]
    f.close()
    lines = [line.strip().split(' ') for line in lines]
    core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        core_numbers.append(cn)

    print('Max Core Number: {0}'.format(max(core_numbers)))

    f = open('zhang_dblp_eta09_epsilon0.5_phi05_n17_rounds_lpg_2.txt', 'r')
    lines = f.readlines()
    del lines[-1]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    estimated_core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        estimated_core_numbers.append(cn)
    
    print('Max Approximated Core Number: {0}'.format(max(estimated_core_numbers)))
    x = np.arange(len(core_numbers))
    approximation = [((s, t), (float(max(s,t)) / min(s, t))) for s,t in zip(core_numbers, estimated_core_numbers)]
    approximation_factor = np.array([float(max(s,t)) / min(s, t) for s,t in zip(core_numbers, estimated_core_numbers)])
    df = pd.DataFrame(approximation_factor, columns=['Approximation Factor'])
    # print(df.head())


    # print('Average Approximation: {0}'.format(statistics.mean(approximation)))
    max_index = get_max_approx_index(approximation)
    print('Maximum Approximation Core Numbers: {0}, {1}'.format(approximation[max_index][0], approximation[max_index][1]))
    # print('Minimum Approximation: {0}'.format(min(approximation)))
    # for a in approximation:
    #     print(a)
    # # plt.plot(x, core_numbers, '-', label='Core Numbers')
    # plt.plot(x, approximation_factor, 'o', label='Approximation Factor')
    # plt.xscale('log')
    sns.displot(df, x="Approximation Factor", kind="kde")
    plt.legend()
    plt.tight_layout()
    # plt.show()
    plt.savefig('./zhang_dblp_approxfactor_wothreshold.png')
    # plt.cla()
    # plt.clf()
    # # print('Max Core Number: {0}'.format(max(core_numbers)))



if __name__ == '__main__':
    # load_graph()
    # preprocess_data()
    core_numbers_distribution()