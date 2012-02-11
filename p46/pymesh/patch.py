from math import *
from euclid import *
from itertools import *
import re
import math

# Turn this flag on if you wish to generate a C file from the RIB.
DumpHeader = False
if DumpHeader:
    global PatchIndex
    global HeaderFile
    HeaderFile = open('PatchData.h', 'w')
    HeaderFile.write('float PatchData[][3] = {\n');
    PatchIndex = 0

def catmull_rom():
    m = Matrix4()
    m[0:16] = (
        -0.5, 1.5, -1.5, 0.5,
        1, 2.5, 2, -0.5,
        -0.5,  0, 0.5, 0,
        0, 1, 0, 0)
    return m

def hermite():
    m = Matrix4()
    m[0:16] = (
        2,-2, 1, 1,
        -3, 3, -2, -1,
        0, 0, 1, 0,
        1, 0, 0, 0)
    return m

def bspline():
    m = Matrix4()
    m[0:16] = (
        -1/6.,  1/2., -1/2.,  1/6.,
         1/2., -1,     1/2.,  0,
        -1/2.,  0,     1/2.,  0,
         1/6.,  2/3.,  1/6.,  0 )
    return m

def bezier():
    m = Matrix4()
    m[0:16] = (
        -1, 3, -3, 1,
        3, -6, 3, 0,
        -3, 3, 0, 0,
        1, 0, 0, 0 )
    return m

def compute_patch_matrices(knots, B):
    assert(len(knots) == 3)
    return [B * P * B.transposed() for P in knots]

def create_patch(tokens, transform, file):
    """ Parse a RIB line that looks like this:
        Patch "bicubic" "P" [6 0 1 ... 3 0 1]
    """
    basis = tokens[1].strip('"')
    if basis != 'bicubic':
        print "Sorry, I only understand bicubic patches."
        quit()
    tokens = tokens[3:]
    
    # Parse all knots on this line:
    coords = map(float, ifilter(lambda t:len(t)>0, tokens))
            
    # If necessary, continue to the next line, looping until done:
    while len(coords) < 16*3:
        line = file.next()
        tokens = re.split('[\s\[\]]', line)
        c = map(float, ifilter(lambda t:len(t)>0, tokens))
        coords.extend(c)

    # Transform each knot position by the given transform:
    args = [iter(coords)] * 3
    knots = [transform * Point3(*v) for v in izip(*args)]
    
    # Optionally dump out the coords in C syntax:
    if DumpHeader:
        global PatchIndex
        global HeaderFile
        HeaderFile.write("\t// Patch " + str(PatchIndex) + '\n')
        for p in knots:
            s = []
            for value in p[:]:
                if math.floor(value) == value:
                    s.append(str(int(value)))
                else:
                    s.append('%gf' % value)
            HeaderFile.write("\t{ %s, %s, %s },\n" % tuple(s))
        PatchIndex = PatchIndex + 1

    # Split the coordinates into separate lists of X, Y, and Z:
    knots = [c for v in knots for c in v]
    knots = [islice(knots, n, None, 3) for n in (0,1,2)]

    # Arrange the coordinates into 4x4 matrices:
    mats = [Matrix4() for n in (0,1,2)]
    for knot, mat in izip(knots, mats):
        mat[0:16] = list(knot)
    
    return compute_patch_matrices(mats, bezier())

def create_matrices_from_rib(filename):
    file = open(filename,'r') 
    coefficient_matrices = []
    transform = Matrix4()
    transform_stack = []
    for line in file:
        line = line.strip(' \t\n\r')
        tokens = re.split('[\s\[\]]', line)
        if len(tokens) < 1: continue
            
        action = tokens[0]
        if action == 'TransformBegin':
            transform_stack.append(transform)
        elif action == 'TransformEnd':
            transform = transform_stack[-1]
            transform_stack = transform_stack[:-1]
        elif action == 'Translate':
            xlate = map(float, tokens[1:])
            transform = transform * Matrix4.new_translate(*xlate)
        elif action == 'Scale':
            scale = map(float, tokens[1:])
            transform = transform * Matrix4.new_scale(*scale)
        elif action == 'Rotate':
            angle = float(tokens[1])
            angle = math.radians(angle)
            axis = Vector3(*map(float, tokens[2:]))
            transform = transform * Matrix4.new_rotate_axis(angle, axis)
        elif action == 'Patch':
            Cx, Cy, Cz = create_patch(tokens, transform, file)
            coefficient_matrices.append((Cx, Cy, Cz))
            
    return coefficient_matrices
    
def make_col_vector(a, b, c, d):
    V = Matrix4()
    V[0:16] = [a, b, c, d] + [0] * 12
    return V

def make_row_vector(a, b, c, d):
    return make_col_vector(a, b, c, d).transposed()

def make_patch_func(Cx, Cy, Cz):
    def patch(u, v):
        U = make_row_vector(u*u*u, u*u, u, 1)
        V = make_col_vector(v*v*v, v*v, v, 1)
        x = U * Cx * V
        y = U * Cy * V
        z = U * Cz * V
        return x[0], y[0], z[0]
    return patch

def load_rib(filename):
    
    # Parse the RIB, apply transforms, obtain a triplet of 4x4 matrices for each patch:
    knots = create_matrices_from_rib(filename)
    
    # Create a list of function objects that can be passed to the parametric evaluator:
    return [make_patch_func(Cx, Cy, Cz) for Cx, Cy, Cz in knots]
