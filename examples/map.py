import numpy as np
import matplotlib.pyplot as plt

with open('results.txt') as fh:
    data = fh.readlines()

grid = []
for line in data:
    x,y,z,r = line.split(' ')
    grid.append(float(r))
grid = np.asarray(grid).reshape((100,100))
plt.imshow(grid, interpolation=None, cmap='autumn')
plt.savefig('map.png')
