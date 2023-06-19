import os
import sys
import numpy as np
import matplotlib.pyplot as plt

assert len(sys.argv) == 2
outfile = sys.argv[1]
assert os.path.isfile(outfile)

with open(outfile, 'r') as f:
    lines = f.readlines()

n = int(lines[0].rstrip())

fig, ax = plt.subplots()

for h in range(n):
    line = lines[h+1].rstrip()
    points = []
    for w in range(n-1):
        if line[w] == '1':
            points.append(w)
    points = np.array(points)
    ax.hlines([h]*len(points), points, points+1, colors='black')

for h in range(n-1):
    line = lines[n+h+1].rstrip()
    points = []
    for w in range(n):
        if line[w] == '1':
            points.append(w)
    ax.vlines(points, [h]*len(points), [h+1]*len(points), colors='black')

plt.show()

