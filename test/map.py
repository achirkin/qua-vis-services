import matplotlib.pyplot as plt
import numpy as np

data = []
with open('test/isovist.txt') as fh:
    data = np.asarray(map(float, fh.readlines()[:-3]))

plt.plot(data)
plt.show()
data.resize((500, data.shape[0] / 500))
plt.imshow(data, cmap='gray', interpolation='bicubic')
plt.show();

print data
