#read from query original file and performance file to build a classifier
from scipy import stats
import numpy as np

def read_queries(dir, array_node_num, array_balence, array_depth):
    for query_file in dir:
        with open(query_file, 'r') as filesource:
            cur_node_num = ...
            array_node_num = np.append(array_node_num, cur_node_num)




def build_feature_from_queries(dir):
    array_node_num = np.append([], [])
    array_balence = np.append([], [])
    array_depth = np.append([], [])
    read_queries(dir, array_node_num, array_balence, array_depth)


def build_data_vec(dir):
    
