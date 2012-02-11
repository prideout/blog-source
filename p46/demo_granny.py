#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from platform_open import *
from export_ctm import *
import parametric, tubes

filename = "granny.ctm"
func = tubes.granny
slices, stacks = 128, 16

verts, faces = parametric.closed_surface(slices, stacks, func)
export_ctm(verts, faces, filename)
platform_open(filename)
