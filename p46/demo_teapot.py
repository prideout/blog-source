#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')
module_paths.append('./pyctm')
module_paths.append('./pyeuclid')

from platform_open import *
from export_ctm import *
from weld import *
import parametric, patch

outfile = "teapot.ctm"

print "Loading RIB file..."
funcs = patch.load_rib("models/uteapot.rib")
slices, stacks = 8, 8

print "Evaluating parametric functions..."
verts, faces = parametric.multisurface(slices, stacks, funcs)

print "Welding verts..."
verts, faces = weld(verts, faces)

export_ctm(verts, faces, outfile)
platform_open(outfile)
