#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from platform_open import *
from export_ctm import *
import parametric, blob

filename = "torus.ctm"
func = parametric.make_torus(8, 2)
slices, stacks = 40, 10

verts, faces = parametric.closed_surface(slices, stacks, func)

export_ctm(verts, faces, filename)

platform_open(filename)
