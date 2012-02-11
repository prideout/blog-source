#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from platform_open import *
from export_ctm import *
from weld import *
import parametric, patch

outfile = "gumbo.ctm"

print "Loading RIB file..."
funcs = patch.load_rib("models/gumbo.rib")
slices, stacks = 8, 8

print "Evaluating parametric functions..."
verts, faces = parametric.multisurface(slices, stacks, funcs)

if False:
    print "Welding verts..."
    verts, faces = weld(verts, faces)

export_raw(verts, faces, outfile)
platform_open(outfile)
