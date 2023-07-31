from collections import defaultdict
import matplotlib.pyplot as plt 
import numpy as np

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

    f = open('zhang_dblp_phi05_n17_corrected.txt', 'r')
    lines = f.readlines()
    del lines[-1]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    estimated_core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        estimated_core_numbers.append(cn)
    
    print('Max Approximated Core Number: {0}'.format(max(estimated_core_numbers)))
    # x = np.arange(len(core_numbers))
    # approximation = [s - t for s,t in zip(core_numbers, estimated_core_numbers)]
    # for a in approximation:
    #     print(a)
    # # plt.plot(x, core_numbers, '-', label='Core Numbers')
    # plt.plot(x, approximation, '-', label='Approximation')
    # plt.legend()
    # # plt.tight_layout()
    # plt.show()
    # plt.savefig('./dblp_core_plot.png')
    # plt.cla()
    # plt.clf()
    # print('Max Core Number: {0}'.format(max(core_numbers)))



if __name__ == '__main__':
    # load_graph()
    # preprocess_data()
    core_numbers_distribution()