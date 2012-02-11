#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from import_ctm import *
from export_ctm import *
from weld import *

infile = "Pericardial_Sac.ctm"
outfile = "reduced.ctm"

print "Importing..."
verts, faces = import_ctm(infile)

print "Welding verts..."
verts, faces = weld(verts, faces)

print "Exporting..."
export_ctm(verts, faces, outfile)
