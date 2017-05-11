import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

with open('results.txt') as fh:
    data = fh.readlines()

grid = []
for line in data:
    x,y,z,r = line.split(' ')
    grid.append(float(r))
grid = np.asarray(grid)
grid[grid < np.percentile(grid,5)] = np.mean(grid)
grid[grid > np.percentile(grid,5)] = np.mean(grid)
grid = grid.reshape((100,100))
grid = (grid - grid.min())/(grid.max() - grid.min())
plt.imshow(grid, interpolation=None, cmap='autumn', norm=LogNorm(vmin=0.3, vmax=0.5))
plt.savefig('map.png')
plt.show()
