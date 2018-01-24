import matplotlib.pyplot as plt
def quickgen():
    x = range(1, 318)
    y = range(1, 318)
    for k in [17, 23, 55, 59, 65, 78, 99, 101, 133, 198, 199, 211, 245, 246, 247, 261, 266,270, 281, 286]:
        for i in range(k, len(y)):
            y[i] -= 1
    return x, y


def output(x, y):
    outfile = "./newplotdata/iso_homo"
    with open(outfile, 'w') as fp:
        for i in range(len(x)):
            fp.write(str(x[i]) + ' ' + str(y[i]) + '\n')


def input(filename):
    x = []
    y = []
    with open(filename, 'r') as fin:
        for line in fin:
            x_val, y_val = int(line.split(' ')[0]), int(line.split(' ')[1])
            x.append(x_val)
            y.append(y_val)
    return x, y


def draw(x, y):
    fig = plt.figure(figsize=(6,4))
    ax = fig.add_subplot(111)
    plt.plot(x, y, label="#iso matches")
    plt.plot(x, x, 'r--', label="#homo matches")
    plt.legend(loc=2)
    ax.set_xlabel("k")
    ax.set_ylabel("#results")
    plt.show()
    fig.savefig("newplotdata/homo_iso.png")


if __name__ == "__main__":
    x, y = quickgen()
    output(x, y)
    draw(x, y)