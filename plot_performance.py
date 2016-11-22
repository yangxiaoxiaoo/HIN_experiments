from os import listdir
from os.path import isfile, join
import numpy as np
import math
import matplotlib.pyplot as plt

def plot(AlgoMeans, AlgoStd, baseMeans, baseStd, title, unit):

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


    ax.set_ylabel(unit)
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


def stat_time(algoMeans, algoStd, baselineMeans, baselineStd, shape):
    algo_times = list()
    algo_mem = list()
    algo_space = list()

    baseline_times = list()
    baseline_mem = list()
    baseline_space = list()

    onlyfiles = [f for f in listdir("./output"+str(shape)) if isfile(join("./output"+str(shape), f))]
    for file in onlyfiles:
        with open(join("./output"+str(shape), file), "r") as fp:
            counter = 0
            for line in fp:
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    print float(line.split('\t')[0])
                    algo_mem.append(float(line.split('\t')[2]))
                    algo_space.append(float(line.split('\t')[3]))

                    counter+=1
                else:
                    baseline_times.append(float(line.split('\t')[0]))
                    baseline_mem.append(float(line.split('\t')[2]))
                    baseline_space.append(float(line.split('\t')[3]))

    algoMeans.append(sum(algo_times)/len(algo_times))
    algoStd.append(math.sqrt(np.var(algo_times)))
    baselineMeans.append(sum(baseline_times)/len(baseline_times))
    baselineStd.append(math.sqrt(np.var(baseline_times)))

    print algoMeans
    print baselineMeans

    return (algoMeans, algoStd, baselineMeans, baselineStd)

def stat_mem(algoMeans, algoStd, baselineMeans, baselineStd, shape):
    algo_times = list()
    algo_mem = list()
    algo_space = list()

    baseline_times = list()
    baseline_mem = list()
    baseline_space = list()

    onlyfiles = [f for f in listdir("./output"+str(shape)) if isfile(join("./output"+str(shape), f))]
    for file in onlyfiles:
        with open(join("./output"+str(shape), file), "r") as fp:
            counter = 0
            for line in fp:
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    print float(line.split('\t')[0])
                    algo_mem.append(float(line.split('\t')[2]))
                    algo_space.append(float(line.split('\t')[3]))

                    counter+=1
                else:
                    baseline_times.append(float(line.split('\t')[0]))
                    baseline_mem.append(float(line.split('\t')[2]))
                    baseline_space.append(float(line.split('\t')[3]))

    algoMeans.append(sum(algo_mem)/len(algo_mem))
    algoStd.append(math.sqrt(np.var(algo_mem)))
    baselineMeans.append(sum(baseline_mem)/len(baseline_mem))
    baselineStd.append(math.sqrt(np.var(baseline_mem)))

    print algoMeans
    print baselineMeans

    return (algoMeans, algoStd, baselineMeans, baselineStd)


def stat_space(algoMeans, algoStd, baselineMeans, baselineStd, shape):
    algo_times = list()
    algo_mem = list()
    algo_space = list()

    baseline_times = list()
    baseline_mem = list()
    baseline_space = list()

    onlyfiles = [f for f in listdir("./output"+str(shape)) if isfile(join("./output"+str(shape), f))]
    for file in onlyfiles:
        with open(join("./output"+str(shape), file), "r") as fp:
            counter = 0
            for line in fp:
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    print float(line.split('\t')[0])
                    algo_mem.append(float(line.split('\t')[2]))
                    algo_space.append(float(line.split('\t')[3]))

                    counter+=1
                else:
                    baseline_times.append(float(line.split('\t')[0]))
                    baseline_mem.append(float(line.split('\t')[2]))
                    baseline_space.append(float(line.split('\t')[3]))

    algoMeans.append(sum(algo_space)/len(algo_space))
    algoStd.append(math.sqrt(np.var(algo_space)))
    baselineMeans.append(sum(baseline_space)/len(baseline_space))
    baselineStd.append(math.sqrt(np.var(baseline_space)))

    print algoMeans
    print baselineMeans

    return (algoMeans, algoStd, baselineMeans, baselineStd)




def plottime():
    algoMeans = list()
    algoStd = list()
    baselineMeans = list()
    baselineStd = list()

    algoMeans, algoStd, baselineMeans, baselineStd = stat_time(algoMeans, algoStd, baselineMeans, baselineStd, 5)
    #construct comparison bar1: first ...
    algoMeans, algoStd, baselineMeans, baselineStd = stat_time(algoMeans, algoStd, baselineMeans, baselineStd, 7)
    #construct comparison bar 2: second element in lists to plot

    plot(algoMeans, algoStd, baselineMeans, baselineStd, "Running time for shapes of queries: Enron", 'Time:/s')



def plotmem():
    algoMeans = list()
    algoStd = list()
    baselineMeans = list()
    baselineStd = list()

    algoMeans, algoStd, baselineMeans, baselineStd = stat_mem(algoMeans, algoStd, baselineMeans, baselineStd, 5)
    #construct comparison bar1: first ...
    algoMeans, algoStd, baselineMeans, baselineStd = stat_mem(algoMeans, algoStd, baselineMeans, baselineStd, 7)
    #construct comparison bar 2: second element in lists to plot

    plot(algoMeans, algoStd, baselineMeans, baselineStd, "Memory usage for shapes of queries: Enron", 'numbers')


def plotspace():
    algoMeans = list()
    algoStd = list()
    baselineMeans = list()
    baselineStd = list()

    algoMeans, algoStd, baselineMeans, baselineStd = stat_space(algoMeans, algoStd, baselineMeans, baselineStd, 5)
    #construct comparison bar1: first ...
    algoMeans, algoStd, baselineMeans, baselineStd = stat_space(algoMeans, algoStd, baselineMeans, baselineStd, 7)
    #construct comparison bar 2: second element in lists to plot

    plot(algoMeans, algoStd, baselineMeans, baselineStd, "Total search space for shapes of queries: Enron", 'numbers')






def main():
    plottime()
    plotmem()
    plotspace()


if __name__ == "__main__":
    main()