#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import geojson
import codecs
import numpy as np

def create_grid(filepath, Nx, Ny):
    with codecs.open(filepath, 'r', 'utf-8') as fh:
        data = geojson.load(fh)
    points = []
    for feature in data.features:
        for coordinate in feature.geometry.coordinates:
            if isinstance(coordinate[0][0], list):
                points += coordinate[0]
    points = np.asarray(points)
    max = points.max(axis=0)
    min = points.min(axis=0)

    z = (max[2]-min[2])/2+min[2]
    xx = np.linspace(min[0], max[0], Nx)
    yy = np.linspace(min[1], max[1], Ny)

    for x in xx:
        for y in yy:
            print x,y,z

if __name__ == '__main__':
    if len(sys.argv) > 1:
        create_grid(sys.argv[1], sys.argv[2], sys.argv[3])
