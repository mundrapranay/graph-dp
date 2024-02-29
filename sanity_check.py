from collections import defaultdict
import matplotlib.pyplot as plt 
import numpy as np
import statistics
import pandas as pd
import seaborn as sns


def load_graph(node_id):
    f = open('./graphs/zhang_orkut', 'r')
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
    
    # for k,v in data.items():
    #     print('Node: {0} | AdjacencyListSize: {1}'.format(k, len(v)))
    # print(len(data[node_id]))
    print(len(data))
        


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

def core_numbers_distribution(filename, graph):
    # f = open('/home/ubuntu/ground_truth/zhang_{0}_cores'.format(graph), 'r')
    f = open('/home/ubuntu/ground_truth/zhang_dblp_cores'.format(graph), 'r')
    lines = f.readlines()
    # del lines[-1]
    f.close()
    lines = [line.strip().split(' ') for line in lines]
    core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        core_numbers.append(cn)

    print('Max Core Number: {0}'.format(max(core_numbers)))

    f = open(filename, 'r')
    lines = f.readlines()
    del lines[-1]
    del lines[0]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    estimated_core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        estimated_core_numbers.append(cn)
    
    print(filename)
    # print('Max Approximated Core Number: {0}'.format(max(estimated_core_numbers)))
    x = np.arange(len(core_numbers))
    approximation = [((s, t), (float(max(s,t)) / min(s, t))) for s,t in zip(core_numbers, estimated_core_numbers)]
    approximation_factor = np.array([float(max(s,t)) / min(s, t) for s,t in zip(core_numbers, estimated_core_numbers)])
    # l2_loss = np.linalg.norm(np.array(core_numbers) - np.array(estimated_core_numbers))
    # print('L2 Loss: {0}'.format(l2_loss))
    # df = pd.DataFrame(approximation_factor, columns=['Approximation Factor'])
    # print(df.head())


    # print('Average Approximation: {0}'.format(statistics.mean(approximation_factor)))
    max_index = get_max_approx_index(approximation)
    print('Node ID: {0}'.format(max_index))
    print('Maximum Approximation Core Numbers: {0}, {1}'.format(approximation[max_index][0], approximation[max_index][1]))
    # print('Minimum Approximation: {0}'.format(min(approximation)))
    # for a in approximation:
    #     print(a)
    # # plt.plot(x, core_numbers, '-', label='Core Numbers')
    # plt.plot(x, approximation_factor, 'o', label='Approximation Factor')
    # plt.xscale('log')
    # sns.displot(df, x="Approximation Factor", kind="kde")
    # plt.legend()
    # plt.tight_layout()
    # # plt.show()
    # plt.savefig('./zhang_dblp_approxfactor_wothreshold.png')
    # plt.cla()
    # plt.clf()
    # # print('Max Core Number: {0}'.format(max(core_numbers)))
    return estimated_core_numbers

def cutoff_thresholds():
    f = open('zhang_dblp_eta09_epsilon0.5_phi05_n17_rounds_lpg_thresholding_2.txt', 'r')
    lines = f.readlines()
    del lines[-1]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    cutoff_threholds = []
    for line in lines:
        cn = int(line[1].strip())
        cutoff_threholds.append(cn)
    
    cutoff_threholds = np.array(cutoff_threholds)
    # df = pd.DataFrame(cutoff_threholds, columns=['Cutoff Thresholds'])
    # sns.displot(df, x="Cutoff Thresholds", kind="kde")
    # x = np.arange(len(cutoff_threholds))
    # print(len(cutoff_threholds))
    # plt.plot(x, cutoff_threholds)
    # plt.savefig('./zhang_dblp_cutoff_threholds_2.png')
    print('Max Cutoff: {0}\t Min Cutoff: {1}\t Average Cutoff: {2}'.format(max(cutoff_threholds), min(cutoff_threholds), statistics.mean(cutoff_threholds)))

def read_data(filename):
    next_levels_round = {}
    permanent_zeros_round = {}
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    for i in range(0, len(lines), 3):
        round_number = int(lines[i].strip().split(":")[1].strip())
        next_levels_vector = [int(val) for val in lines[i+1].split()]
        permanent_zeros_vector = [int(val) for val in lines[i+2].split()]
        next_levels_round[round_number] = next_levels_vector
        permanent_zeros_round[round_number] = permanent_zeros_vector
    
    return next_levels_round, permanent_zeros_round

def debugger():
    files = ['/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_1_partitioned_no_noise_bias_testing.txt', '/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_1_no_noise_no_bias_dev_testing_oal.txt']
    pt_code_nl_dict, pt_code_pz_dict = read_data(files[0])
    oal_code_nl_dict, oal_code_pz_dict = read_data(files[1])
    assert(len(pt_code_pz_dict) == len(oal_code_pz_dict))
    rounds = list(pt_code_nl_dict.keys())
    print(f'Total Number of Rounds {rounds}')
    for r in rounds:
        if (pt_code_nl_dict[r] != oal_code_nl_dict[r]):
            print(f'Next Levels Dont Match for Round {r}')
        
        if (pt_code_pz_dict[r] != oal_code_pz_dict[r]):
            print(f'Permanent Zeros Dont Match for Round {r}')


def get_rounds(file):
    f = open(file, 'r')
    lines = f.readlines()
    f.close()
    lines = [line.strip() for line in lines]
    data = {}
    max_threshold = 0
    for value in lines:
        values = value.split('|')
        if len(values) > 1:
            data[int(values[0].strip())] = float(values[1].strip())
            max_threshold = max(max_threshold, float(values[1].strip()))
    print(max_threshold)

    return data

def debugger_rt():
    files = ['/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_1_dev_debugging_n2.txt', '/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_1_partitioned_debugging_n2.txt']
    dev_testing_data = get_rounds(files[0])
    dev_openstacks_data = get_rounds(files[1]) 
    nodes = list(dev_openstacks_data.keys())
    for n in nodes:
        if (dev_testing_data[n] != dev_openstacks_data[n]):
            print(f'Discrepancy for node: {n}')



if __name__ == '__main__':
    # load_graph(274467)
    # preprocess_data()
    for bf in ['dblp_adj']:
        #output_file = f'/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_{bf}_partitioned_no_noise_no_bias_testing_n_17.txt'
        print(bf)
        print("CDP")
        output_file = f"/home/ubuntu/results_new/golang_results/{bf}_0_1_1_central_dp_kcore_noise_final.txt"
        approx_core_numbers_cdp = core_numbers_distribution(output_file, bf)
        print()
        print("LDP")
        output_file = f"/home/ubuntu/results_new/golang_results/{bf}_0_1_1_local_dp_kcore_noise_final.txt"
        approx_core_numbers_ldp = core_numbers_distribution(output_file, bf)
        print()
        # i = 0
        # for l_c, c_c in zip(approx_core_numbers_ldp, approx_core_numbers_cdp):
        #     if l_c != c_c:
        #         print('LDP Estimate: {0}\t CDP Estimte: {1}\t Node: {2}'.format(l_c, c_c, i))
        #     i += 1
    #get_rounds('/home/ubuntu/golan_dblp_test.txt')
    # core_numbers_distribution()
    # cutoff_thresholds()
    # debugger()
    # debugger_rt()
