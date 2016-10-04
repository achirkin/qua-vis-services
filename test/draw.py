import numpy as np

import matplotlib.pyplot as plt
import matplotlib
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection

fig, ax = plt.subplots()
patches = []

points = []
with open('data.txt') as fh:
    for line in fh.readlines():
        points.append(map(lambda z : z / 6.0 + 0.5, map(float, line.strip()[1:-1].split(','))[:2]))
        print points[-1]

for i in range(0, len(points), 3):
    print points[i:i+3]
    polygon = Polygon(points[i:i+3], True)
    patches.append(polygon)

p = PatchCollection(patches, cmap=matplotlib.cm.jet, alpha=0.4)


ax.add_collection(p)

plt.show()
