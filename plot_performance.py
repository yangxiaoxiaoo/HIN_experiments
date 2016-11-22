from os import listdir
from os.path import isfile, join
import numpy as np
import math
import matplotlib.pyplot as plt

def plot(AlgoMeans, AlgoStd, baseMeans, baseStd, title):

    N = len(AlgoMeans)               # number of data entries
    ind = np.arange(N)              # the x locations for the groups
    width = 0.35                    # bar width

    fig, ax = plt.subplots()

    rects1 = ax.bar(ind, AlgoMeans,                  # data
                    width,                          # bar width
                    color='MediumSlateBlue',        # bar colour
                    yerr=AlgoStd,                  # data for error bars
                    error_kw={'ecolor':'Tomato',    # error-bars colour
                              'linewidth':2})       # error-bar width

    rects2 = ax.bar(ind + width, baseMeans,
                    width,
                    color='Tomato',
                    yerr=baseStd,
                    error_kw={'ecolor':'MediumSlateBlue',
                              'linewidth':2})

    axes = plt.gca()
 #   axes.set_ylim([0, 41])             # y-axis bounds


    ax.set_ylabel('Time:/s')
    ax.set_title(title)
    ax.set_xticks(ind + width)
    ax.set_xticklabels(('shape5', 'shape7'))

    ax.legend((rects1[0], rects2[0]), ('Algorithm', 'Baseline'))

    def autolabel(rects):
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                    '%f' % float(height),
                    ha='center',            # vertical alignment
                    va='bottom'             # horizontal alignment
                    )

    autolabel(rects1)
    autolabel(rects2)

    plt.show()



def main():
    menMeans = (20, 35, 30, 35, 27)
    menStd = (2, 3, 4, 1, 2)
    womenMeans = (25, 32, 34, 20, 25)
    womenStd = (3, 5, 2, 3, 3)


    algoMeans = list()
    algoStd = list()
    baselineMeans = list()
    baselineStd = list()

    algo_times = list()
    baseline_times = list()
    onlyfiles = [f for f in listdir("./output5") if isfile(join("./output5", f))]
    for file in onlyfiles:
        with open(join("./output5", file), "r") as fp:
            counter = 0
            for line in fp:
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    counter+=1
                else:
                    baseline_times.append(float(line.split('\t')[0]))

    algoMeans.append(sum(algo_times)/len(algo_times))
    algoStd.append(math.sqrt(np.var(algo_times)))
    baselineMeans.append(sum(baseline_times)/len(baseline_times))
    baselineStd.append(math.sqrt(np.var(baseline_times)))

    onlyfiles = [f for f in listdir("./output7") if isfile(join("./output7", f))]
    for file in onlyfiles:
        with open(join("./output7", file), "r") as fp:
            counter = 0
            for line in fp:
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    counter+=1
                else:
                    baseline_times.append(float(line.split('\t')[0]))

    algoMeans.append(sum(algo_times)/len(algo_times))
    algoStd.append(math.sqrt(np.var(algo_times)))
    baselineMeans.append(sum(baseline_times)/len(baseline_times))
    baselineStd.append(math.sqrt(np.var(baseline_times)))



    plot(algoMeans, algoStd, baselineMeans, baselineStd, "Running time for shapes of queries: Enron")


if __name__ == "__main__":
    main()