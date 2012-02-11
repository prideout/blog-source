#!/usr/bin/python

from sys import path as module_paths
module_paths.append('./pymesh')

from platform_open import *
from export_ctm import *
import rules_surface as rules

outfile = "tree.ctm"
infile = open('models/tree.xml')
xml_string = infile.read()
verts, faces = rules.surface(xml_string)
export_ctm(verts, faces, outfile)
platform_open(outfile)
