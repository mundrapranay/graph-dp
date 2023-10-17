from collections import defaultdict
from math import ceil
import os


GRAPH_SIZES = {
    'zhang_dblp' : 317080,
    'hua_ctr' : 14081816,
    'hua_livejournal' : 4846609,
    'hua_stackoverflow' : 2584164,
    'hua_usa' : 23947347,
    'hua_youtube' : 1138499,
    'zhang_orkut' : 3072441
}



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
    chunked_nodes = calculate_workloads(GRAPH_SIZES[graph], processes)

    graph_directory = './graphs/{0}_partitioned_{1}/'.format(graph, n)
    os.makedirs(graph_directory, exist_ok=True)
    for i, cn in enumerate(chunked_nodes):
       graph_file = graph_directory + '{0}.txt'.format(i + 1)
       with open(graph_file, 'w') as out:
          for node in cn:
            adjacency_list = data[node]
            for a in adjacency_list:
                out.write('{0} {1}\n'.format(node, a))
       out.close()
    

if __name__ == '__main__':
   partition_graph('zhang_dblp', 17)
   partition_graph('zhang_orkut', 17)
        
    