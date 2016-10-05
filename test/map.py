import matplotlib.pyplot as plt
import numpy as np

data = []
with open('test/isovist.txt') as fh:
    data = np.asarray(map(float, fh.readlines()[:-1]))

data.resize((100, 100))
plt.matshow(data)
plt.show();

print data
