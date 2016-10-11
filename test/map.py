import matplotlib.pyplot as plt
import numpy as np
import sys

if __name__ == '__main__':
    data = []
    with open(sys.argv[1]) as fh:
        data = np.asarray(map(float, fh.readlines()[:-3]))

    #plt.plot(data)
    plt.show()
    data.resize((int(sys.argv[2]), data.shape[0] / int(sys.argv[2])))
    plt.imshow(data, cmap='jet', interpolation='none')
    plt.show();
