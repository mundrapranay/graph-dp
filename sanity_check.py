from collections import defaultdict


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
    f = open('zhang_dblp_phi05_n17.txt', 'r')
    lines = f.readlines()
    del lines[-1]
    f.close()
    lines = [line.strip().split(':') for line in lines]
    core_numbers = []
    for line in lines:
        cn = float(line[1].strip())
        core_numbers.append(cn)
    
    print('Max Core Number: {0}'.format(max(core_numbers)))



if __name__ == '__main__':
    # load_graph()
    # preprocess_data()
    core_numbers_distribution()