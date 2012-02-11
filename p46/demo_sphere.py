#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from platform_open import *
from export_ctm import *
import polyhedra, blob

filename = "sphere.ctm"
num_subdivisions = 3

verts, faces = polyhedra.icosahedron()
for x in xrange(num_subdivisions):
    verts, faces = polyhedra.subdivide(verts, faces)
export_ctm(verts, faces, filename)
platform_open(filename)
