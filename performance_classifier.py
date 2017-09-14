from scipy.cluster import hierarchy, vq
import os
from scipy.stats.stats import pearsonr, spearmanr
import auto_querygen
import networkx as nx

import numpy as np

class Performance_Mining():

    array_branch = np.append([], []) #number of branch = 0 when the tree is a path
    array_terminal_rate = np.append([], []) #number of terminals devided by the number of nodes
    array_tdegree = np.append([], [])

    arr_node_num = np.append([], [])
    arr_search_space = np.append([], [])
    arr_memory = np.append([],[])

    #observatives
    arr_time = np.append([],[])
    arr_time_froB1 = np.append([],[])
    arr_time_froB2 = np.append([],[])

    def read_queries(self, list_of_files, graph, N):  #only read queries with a result
    #read array_balence, array_degree, array_depth
        for file in list_of_files:  #'5'
            filename = os.path.join("Enron/new_queries/N" + str(N), str(file))
            with open(filename, 'r') as filesource:
                lines = filesource.readlines()
            cur_branch = int(lines[5].split()[0]) - 1
            self.array_branch = np.append(self.array_branch, cur_branch)
            tdegree = 0
            for terminal in lines[5].split()[1:]:
                tdegree += graph.degree(int(terminal))
            self.array_tdegree = np.append(self.array_tdegree, tdegree)
            terminal_rate = float(lines[5].split()[0])/float(N)
            self.array_terminal_rate = np.append(self.array_terminal_rate, terminal_rate)



    def read_performance(self, dir, N):
    #read arr_node_num(batch in a dir)
    #along with arr_search_space, arr_memory, arr_time, arr_time_froB1, arr_time_froB2
        list_of_files = []  #those query files
        for performance_file in os.listdir(dir):
            list_of_files.append(performance_file.strip('.')[0])
            with open(os.path.join(dir, performance_file), 'r') as filesource:
                lines = filesource.readlines()
            time_prophet = float(lines[0].split()[0])
            search_space_prophet = int(lines[0].split()[3])
            time_B1 = float(lines[1].split()[0])
            time_B2 = float(lines[2].split()[0])

            self.arr_time = np.append(self.arr_time, time_prophet)
            self.arr_search_space = np.append(self.arr_search_space, search_space_prophet)
            self.arr_time_froB1 = np.append(self.arr_time_froB1, time_prophet/time_B1)
            self.arr_time_froB2 = np.append(self.arr_time_froB2, time_prophet/time_B2)
            self.arr_node_num = np.append(self.arr_node_num, N)
        return list_of_files





    def build_feature_from_queries(self, N):
        g, v2type = auto_querygen.load_graph_struct("./Enron/enron_graph.wgt.norm")
        perform_dir = 'Enron/auto_outputs/N'+ str(N)
        N_list_of_files = self.read_performance(perform_dir, N)
        self.read_queries(N_list_of_files, g, N)


    #read from query original file and performance file to calculate corralation
    def compute_correlation(self):
        print "corralation between degreesum and time"
        print pearsonr(self.array_tdegree, self.arr_time)  #be careful of doing constant value
        print spearmanr(self.array_tdegree, self.arr_time)
      #  print "corralation between degreesum and memory"
      #  print pearsonr(self.array_tdegree, self.arr_memory)
        print "correlation between degreesum and improvement from baseline 1"
        print pearsonr(self.array_tdegree, self.arr_time_froB1)
        print spearmanr(self.array_tdegree, self.arr_time_froB1)
        print "correlation between degreesum and improvement from baseline 2"
        print pearsonr(self.array_tdegree, self.arr_time_froB2)
        print spearmanr(self.array_tdegree, self.arr_time_froB2)
        print "search space and time"
        print pearsonr(self.arr_search_space, self.arr_time)
        print spearmanr(self.arr_search_space, self.arr_time)

    def examine_clusters(self):
        #cluster based on the three performance measurements and output the mean and variance of potentially effective factors
        indexes = range(len(self.arr_time))

        data = np.vstack((indexes, self.arr_time, self.arr_memory, self.arr_time_froB1, self.arr_time_froB2))#stack the performances
        np.transpose(data)
        clusters = vq.kmeans(data)
        #find index and print factors statistics


def main():
    pm = Performance_Mining()
    pm.build_feature_from_queries(5)
    pm.compute_correlation()


if __name__ == "__main__":
    main()