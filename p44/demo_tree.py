#!/usr/bin/python

from platform_open import *
from ctypes import *
from blob import *
import rules_surface as rules

# Import OpenCTM python bindings
from sys import path as module_paths
module_paths.append('./pyctm')
from openctm import *

filename = "tree.ctm"

verts, faces = rules.surface(rules.tree)
pVerts = make_blob(verts, c_float)
pFaces = make_blob(faces, c_uint)
pNormals = POINTER(c_float)()
ctm = ctmNewContext(CTM_EXPORT)
ctmDefineMesh(ctm, pVerts, len(verts), pFaces, len(faces), pNormals)
ctmSave(ctm, filename)
ctmFreeContext(ctm)
platform_open(filename)
