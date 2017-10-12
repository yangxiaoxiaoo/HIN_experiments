from scipy.cluster import hierarchy, vq
import os
from scipy.stats.stats import pearsonr, spearmanr
import auto_querygen
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.finance import candlestick, candlestick2
import copy

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
    Dataset = ''

    def __init__(self, dataset):
        self.Dataset = dataset

    def read_queries(self, list_of_files, graph, N):  #only read queries with a result
    #read array_balence, array_degree, array_depth
        for file in list_of_files:  #'5'
            filename = os.path.join(self.Dataset + "/new_queries/N" + str(N), str(file))
            with open(filename, 'r') as filesource:
                lines = filesource.readlines()
            cur_branch = int(lines[5].split()[0]) - 1
            self.array_branch = np.append(self.array_branch, cur_branch)
            print "loading queries from N = " + str(N)
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
        if self.Dataset == "Enron":
            g, v2type = auto_querygen.load_graph_struct("./Enron/enron_graph.wgt.norm")
        elif self.Dataset == "DBLP":
            g, v2type = auto_querygen.load_graph_struct("./DBLP/dblp_graph.new.wgt")
        elif self.Dataset == "PhotoNet":
            g, v2type = auto_querygen.load_graph_struct("./PhotoNet/graph_prank.graph_new.graph")
        else:
            print "Not known datasets!"
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

    def plots(self):
       # plt.scatter(self.array_tdegree, self.arr_time)
       # plt.show()
        self.plot_candlestick_depth(self.array_tdegree, self.arr_time, yname= "running time" , xname= "total degree", in_step= 2)
        self.plot_candlestick_depth(self.array_tdegree, self.arr_time_froB1, yname="time improvement ratio B1", xname = " total degree", in_step= 4)
        self.plot_candlestick_depth(self.array_tdegree, self.arr_time_froB2, yname="time improvement ratio B2", xname = " total degree", in_step= 4)
    #    self.plot_candlestick(self.array_branch, self.arr_time_froB1, yname= "time improvement ratio B1", xname = " #branch", in_step= 1)
    #    self.plot_candlestick(self.array_branch, self.arr_time_froB2, yname= "time improvement ratio B2", xname = " #branch", in_step= 1)




    def plot_heatmap(self, x, y):  #not very readable
        heatmap, xedges, yedges = np.histogram2d(x, y , bins= 50)
       # extent = [xedges[0], xedges[-1], yedges[0], yedges[-1]]
        plt.clf()
        plt.imshow(heatmap.T, origin='lower')
        plt.show()

    def plot_candlestick(self, x, y, yname, xname, in_step):
        Candles = []
        num_bins = 100
        max_x = int(max(x))
        min_x = int(min(x))
        #max_x = 15
        #min_x = 10
        step= int((max_x - min_x)/num_bins) #default step
        step = in_step #to reduce bins

        valid_bins = []
        means = []
        errs = []
        for bin in range (min_x, max_x + 2, step):
            candle_values = []
            for i in range(len(x)):
                x_value = x[i]
                y_value = y[i]
                if bin<= x_value < bin+ step:
                    candle_values.append(y_value)
            if len(candle_values) > 0: # need more than 2 values to unpack
                mean = float(sum(candle_values)/len(candle_values))
                variance = np.std(candle_values)
                valid_bins.append(str(bin) + "TO" + str(bin + step))
                means.append(mean)
                errs.append(variance)
                print "mean = "
                print mean
                print "variance = "
                print variance
        x_pos = list(range(len(means)))

        ax = plt.subplot()
       # ax.set_yscale("log", nonposy='clip')
        plt.bar(x_pos, means, yerr=errs, align='center', color = '#FFC222', alpha=0.5)
        plt.grid()

        # set height of the y-axis
        max_y = max(zip(means, errs)) # returns a tuple, here: (3, 5)
        plt.ylim([0, (max_y[0] + max_y[1]) * 1.1])
        plt.title(yname + " to " + xname)
        # set axes labels and title
        plt.ylabel(yname)
        plt.xlabel(xname)
        plt.xticks(x_pos, valid_bins)
        labels = ax.get_xticklabels()
        plt.setp(labels, rotation=30, fontsize=10)
        plt.show()

    def plot_candlestick_depth(self, x, y, yname, xname, in_step):

        num_bins = 10
        max_x = int(max(x))
        min_x = int(min(x))
        node_capacity = len(x)/num_bins

        pairs = []
        for i in range(len(x)):
            pairs.append((x[i], y[i]))
        pairs.sort(key=lambda x: x[0]) #sort pairs by x value

        valid_bins = []
        means = []
        errs = []
        l = len(x)
        for start_i in range(0, l - node_capacity, node_capacity):
            bin_start = pairs[start_i][0]
            bin_end = pairs[start_i + node_capacity][0]
            bin_name = str(bin_start) + "To" + str(bin_end)
            bin_list = []
            for j in range(start_i, start_i+ node_capacity+1):
                bin_list.append(pairs[j][1])
            mean = float(sum(bin_list)/node_capacity)
            variance = np.std(bin_list)
            valid_bins.append(bin_name)
            means.append(mean)
            errs.append(variance)

            print "mean = "
            print mean
            print "variance = "
            print variance
        x_pos = list(range(len(means)))
        ax = plt.subplot()
       # ax.set_yscale("log", nonposy='clip')
        plt.bar(x_pos, means, yerr=errs, align='center', color = '#FFC222', alpha=0.5)
        plt.grid()

        # set height of the y-axis
        max_y = max(zip(means, errs)) # returns a tuple, here: (3, 5)
        plt.ylim([0, (max_y[0] + max_y[1]) * 1.1])
        plt.title(yname + " to " + xname)
        # set axes labels and title
        plt.ylabel(yname)
        plt.xlabel(xname)
        plt.xticks(x_pos, valid_bins)
        labels = ax.get_xticklabels()
        plt.setp(labels, rotation=30, fontsize=10)
        plt.show()


def main():
    pm = Performance_Mining('Enron')
    for N in [5, 6, 7, 8, 9, 10]:
    #pm = Performance_Mining('DBLP')
    #for N in [5, 6, 7, 8]:
        pm.build_feature_from_queries(N)
   # pm.Dataset = 'Enron'
   # for N in [5, 6, 7, 8]:
   #     pm.build_feature_from_queries(N)
    pm.compute_correlation()
    pm.plots()



if __name__ == "__main__":
    main()