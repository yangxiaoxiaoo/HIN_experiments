from os import listdir
from os.path import isfile, join
import numpy as np
import math
import matplotlib.pyplot as plt

def plot(AlgoMeans, AlgoStd, baseMeans, baseStd, backboneMeans, backboneStd, title, unit):

    N = len(AlgoMeans)               # number of data entries
    ind = np.arange(N)              # the x locations for the groups
    width = 0.25                    # bar width

    fig, ax = plt.subplots()

    rects1 = ax.bar(ind, AlgoMeans,                  # data
                    width,                          # bar width
                    color='MediumSlateBlue',        # bar colour
             #       log = True,
             #       yerr=AlgoStd,                  # data for error bars
                    error_kw={'ecolor':'Tomato',    # error-bars colour
                              'linewidth':2})       # error-bar width

    rects2 = ax.bar(ind + width, baseMeans,
                    width,
                    color='Tomato',
               #     log = True,
               #     yerr=baseStd,
                    error_kw={'ecolor':'MediumSlateBlue',
                              'linewidth':2})

    rects3 = ax.bar(ind +  2*width, backboneMeans,
                    width,
                    color='Green',
             #       log = True,                            setting log true will be bad for showing significance
              #      yerr=backboneStd,
                    error_kw={'ecolor':'Green',
                              'linewidth':2})

    axes = plt.gca()


    ax.set_ylabel(unit)
    ax.set_title(title)
    ax.set_xticks(ind + width)
    ax.set_xticklabels(('shape1', 'shape2','shape5', 'shape6', 'shape7'))

 #   ax.set_ylim([100, 0.1])

    ax.legend((rects1[0], rects2[0], rects3[0]), ('Algorithm', 'Bruteforce', 'Backbone'))

    def autolabel(rects):
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width()/3., 1.05*height,
                    '%.2f' % float(height),
                    ha='center',            # vertical alignment
                    va='bottom'             # horizontal alignment
                    )

    autolabel(rects1)
    autolabel(rects2)
    autolabel(rects3)

    plt.show()


def stat(mode, algoMeans, algoStd, baselineMeans, baselineStd, backboneMeans, backboneStd, shape, instring):
    algo_times = list()
    algo_mem = list()
    algo_space = list()

    baseline_times = list()
    baseline_mem = list()
    baseline_space = list()

    backbone_times = list()
    backbone_mem = list()
    backbone_space = list()

    onlyfiles = [f for f in listdir(instring+"/output"+str(shape)) if isfile(join(instring+"/output"+str(shape), f))]
    for file in onlyfiles:
        with open(join(instring+"/output"+str(shape), file), "r") as fp:

            for counter, line in enumerate(fp):
                if counter == 0:
                    algo_times.append(float(line.split('\t')[0]))
                    algo_mem.append(float(line.split('\t')[2]))
                    algo_space.append(float(line.split('\t')[3]))

                    print line
                if counter == 1:
                    baseline_times.append(float(line.split('\t')[0]))
                    baseline_mem.append(float(line.split('\t')[2]))
                    baseline_space.append(float(line.split('\t')[3]))

                    print line
                if counter == 2:
                    backbone_times.append(float(line.split('\t')[0]))
                    backbone_mem.append(float(line.split('\t')[2]))
                    backbone_space.append(float(line.split('\t')[3]))
                    print "a three line file !"

   # print str(backbone_times == baseline_times) +"equal "

    if mode == 'time':

        algoMeans.append(sum(algo_times)/len(algo_times))
        algoStd.append(math.sqrt(np.var(algo_times)))
        baselineMeans.append(sum(baseline_times)/len(baseline_times))
        baselineStd.append(math.sqrt(np.var(baseline_times)))
        backboneMeans.append(sum(backbone_times)/len(backbone_times))
        backboneStd.append(math.sqrt(np.var(backbone_times)))

    if mode == "mem":
        algoMeans.append(sum(algo_mem)/len(algo_mem))
        algoStd.append(math.sqrt(np.var(algo_mem)))
        baselineMeans.append(sum(baseline_mem)/len(baseline_mem))
        baselineStd.append(math.sqrt(np.var(baseline_mem)))
        backboneMeans.append(sum(backbone_mem)/len(backbone_mem))
        backboneStd.append(math.sqrt(np.var(backbone_mem)))

    if mode == "space":
        algoMeans.append(sum(algo_space)/len(algo_space))
        algoStd.append(math.sqrt(np.var(algo_space)))
        baselineMeans.append(sum(baseline_space)/len(baseline_space))
        baselineStd.append(math.sqrt(np.var(baseline_space)))
        backboneMeans.append(sum(backbone_mem)/len(backbone_space))
        backboneStd.append(math.sqrt(np.var(backbone_space)))


    print algoMeans
    print baselineMeans

    return (algoMeans, algoStd, baselineMeans, baselineStd, backboneMeans, backboneStd)




def compare(mode, dataset, instring, shapes):


    algoMeans = list()
    algoStd = list()
    baselineMeans = list()
    baselineStd = list()
    backboneMeans = list()
    backboneStd = list()

    for shape in shapes:
        print shape
        algoMeans, algoStd, baselineMeans, baselineStd, backboneMeans, backboneStd = stat(mode, algoMeans, algoStd, baselineMeans, baselineStd,  backboneMeans, backboneStd, shape, instring)
    if mode == 'time':
        plot(algoMeans, algoStd, baselineMeans, baselineStd,  backboneMeans, backboneStd, "Running time for shapes of queries:"+dataset, 'Time:/s')
    if mode == 'mem':
        plot(algoMeans, algoStd, baselineMeans, baselineStd,  backboneMeans, backboneStd, "Memory usage for shapes of queries: "+dataset, 'counts')
    if mode == 'space':
        plot(algoMeans, algoStd, baselineMeans, baselineStd,  backboneMeans, backboneStd, "Total search space for shapes of queries:"+dataset, 'counts')




def main():

    dataset = 'Enron'
    instring = dataset + 'outputs'
    shapes = [1, 2, 6]
 #   measurement = ['time', 'mem', 'space']  memory and space definition need to be updated
    measurement = ['time']
    for item in measurement:
        compare(item, dataset, instring, shapes)


#Enron has only query template 1, 2, 6En
    datasets = ['PhotoNet', 'DBLP', 'YelpPhoto']
    for dataset in datasets:
        instring = dataset + 'outputs'
        shapes = [1, 2, 5, 6, 7]

     #   measurement = ['time', 'mem', 'space']  memory and space definition need to be updated
        measurement = ['time']
        for item in measurement:
            compare(item, dataset, instring, shapes)


if __name__ == "__main__":
    main()