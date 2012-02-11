#!/usr/bin/python

from platform_open import *
from ctypes import *
import parametric, blob

# Import OpenCTM python bindings
from sys import path as module_paths
module_paths.append('./pyctm')
from openctm import *

###############################

filename = "torus.ctm"

verts, faces = parametric.closed_surface(40, 10, parametric.make_torus(8, 2))
pVerts = blob.make_blob(verts, c_float)
pFaces = blob.make_blob(faces, c_uint)
pNormals = POINTER(c_float)()
ctm = ctmNewContext(CTM_EXPORT)
ctmDefineMesh(ctm, pVerts, len(verts), pFaces, len(faces), pNormals)
ctmSave(ctm, filename)
ctmFreeContext(ctm)
platform_open(filename)
print "nverts = %d, nfaces = %d" % (len(verts), len(faces))