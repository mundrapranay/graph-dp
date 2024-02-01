from collections import defaultdict
from math import ceil
import os
import networkx as nx 

GRAPH_SIZES = {
    'zhang_dblp' : 317080,
    'hua_ctr' : 14081816,
    'hua_livejournal' : 4846609,
    'hua_stackoverflow' : 2584164,
    'hua_usa' : 23947347,
    'hua_youtube' : 1138499,
    'zhang_orkut' : 3072441,
    'gplus' : 107614,
    'imdb' : 896308,
    'random_gen' : 1000
}



def generate_complete_graph(n, outputLoc):
    G = nx.complete_graph(n)
    edges = [e for e in G.edges]
    out = open(outputLoc, 'w')
    for e in edges:
        u, v = e[0], e[1]
        out.write('{0} {1}\n'.format(u, v))
    out.close()


def chunk_into_n(lst, n):
  size = ceil(len(lst) / n)
  return [lst[x * size:x * size + size] for x in range(n)]

def calculate_workloads(n, num_process):
    chunk = n // num_process
    extra = n % num_process
    offset = 0
    workloads = []

    for p in range(1, num_process + 1):
        workload = chunk + extra if p == num_process else chunk
        node_ids = list(range(offset, offset + workload))
        workloads.append(node_ids)
        offset += workload

    return workloads

def partition_graph(graph, n):
    processes = n - 1
    f = open('./graphs/{0}'.format(graph), 'r')
    # f = open('/home/ubuntu/TriangleLDP/data/{0}/{1}_adj.txt'.format(graph, graph.lower()), 'r')
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
    
    # nodes = [i for i in range(GRAPH_SIZES[graph])]
    # nodes = sorted(list(data.keys()))
    # chunked_nodes = chunk_into_n(nodes, processes)
    chunked_nodes = calculate_workloads(GRAPH_SIZES[graph.lower()], processes)

    graph_directory = './graphs/{0}_partitioned_{1}/'.format(graph.lower(), n)
    os.makedirs(graph_directory, exist_ok=True)
    total_m = 0;
    for i, cn in enumerate(chunked_nodes):
       print("Partition : {0} | Nodes : {1} | ADL : {2}".format(i, len(cn), sum([len(data[n]) for n in cn])))
       total_m += sum([len(data[n]) for n in cn])
       graph_file = graph_directory + '{0}.txt'.format(i + 1)
       with open(graph_file, 'w') as out:
          for node in cn:
            adjacency_list = data[node]
            for a in adjacency_list:
                out.write('{0} {1}\n'.format(node, a))
       out.close()
    #tota_m_up = sum([len(data[n]) for n in data.keys()])
    #assert(tota_m_up == total_m)



def load_graph(graph):
    f = open('./graphs/{0}'.format(graph), 'r')
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
    return data

#    Adjacency List for Node: 297249 : [169245, 228507, 164276, ]
def check_graphs_reads(graph):
    full_graph_adl = load_graph(graph)
    log_file = open('/home/ubuntu/results_new/graph_zhang_dblp_factor_id_0_bias_0_bias_factor_1_partitioned_no_noise_no_bias_testing.txt', 'r')
    lines = log_file.readlines()
    lines = [line.strip() for line in lines]
    log_file.close()
    data = {}
    for line in lines:
        if line.startswith('Adjacency List for Node:'):
            line_data = line.split(':')
            try:
                node = int(line_data[1])
                adl = [int(v) for v in line_data[2].replace('[', '').replace(']', '').split(',') if len(v.strip()) > 0]
                data[node] = adl
            except ValueError:
                print(line)
                
    for n, v in full_graph_adl.items():
       if (sorted(data[n]) != sorted(full_graph_adl[n])):
           print(f'Discrepancy for Node {n}')




if __name__ == '__main__':
    # partition_graph('Gplus', 17)
    # partition_graph('IMDB', 17)
    generate_complete_graph(1000, './graphs/random_gen')
    partition_graph('random_gen', 17)
#   partition_graph('zhang_dblp', 17)
#    partition_graph('zhang_orkut', 17)
    #check_graphs_reads('zhang_dblp')
    
