import matplotlib.pyplot as plt
import numpy as np

data = []
with open('test/isovist.txtt') as fh:
    data = np.asarray(map(float, fh.readlines()[:-3]))

#plt.plot(data)
plt.show()
data.resize((200, data.shape[0] / 200))
plt.imshow(data, cmap='gray', interpolation='none')
plt.show();

print data
