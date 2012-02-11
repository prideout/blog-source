﻿from math import *
from euclid import *

def make_torus(major, minor):
    def torus(u, v):
        u = u * 2
        x = (major + minor * cos(v)) * cos(u)
        y = (major + minor * cos(v)) * sin(u)
        z = minor * sin(v)
        return x, y, z
    return torus

def sphere(u, v):
    x = sin(u) * cos(v)
    y = cos(u)
    z = -sin(u) * sin(v)
    return x, y, z

def klein(u, v):
    u = u * 2
    if u < pi:
        x = 3 * cos(u) * (1 + sin(u)) + (2 * (1 - cos(u) / 2)) * cos(u) * cos(v)
        z = -8 * sin(u) - 2 * (1 - cos(u) / 2) * sin(u) * cos(v)
    else:
        x = 3 * cos(u) * (1 + sin(u)) + (2 * (1 - cos(u) / 2)) * cos(v + pi)
        z = -8 * sin(u)
    y = -2 * (1 - cos(u) / 2) * sin(v)
    return x, y, z
    
def mobius(u, t):
    u = u * 2
    phi = u / 2
    major, a, b = 1.25, 0.125, 0.5
    x = a * cos(t) * cos(phi) - b * sin(t) * sin(phi)
    z = a * cos(t) * sin(phi) + b * sin(t) * cos(phi)
    y = (major + x) * sin(u)
    x = (major + x) * cos(u)
    return x, y, z

def closed_surface(slices, stacks, func):
    verts = []
    for i in xrange(slices):
        theta = i * pi / slices
        for j in xrange(stacks):
            phi = j * 2.0 * pi / stacks
            p = func(theta, phi)
            verts.append(p)
            
    faces = []
    v = 0
    for i in xrange(slices - 1):
        for j in xrange(stacks):
            next = (j + 1) % stacks
            faces.append((v + j, v + next, v + j + stacks))
            faces.append((v + next, v + next + stacks, v + j + stacks))
        v = v + stacks
    for j in xrange(stacks):
        next = (j + 1) % stacks
        faces.append((v + j, v + next, j))
        faces.append((v + next, next, j))

    return verts, faces

def open_surface(slices, stacks, func):
    verts = []
    for i in xrange(slices + 1):
        theta = i * pi / slices
        for j in xrange(stacks):
            phi = j * 2.0 * pi / stacks
            p = func(theta, phi)
            verts.append(p)
            
    faces = []
    v = 0
    for i in xrange(slices):
        for j in xrange(stacks):
            next = (j + 1) % stacks
            faces.append((v + j, v + next, v + j + stacks))
            faces.append((v + next, v + next + stacks, v + j + stacks))
        v = v + stacks

    return verts, faces

def surface(slices, stacks, func):
    return open_surface(slices, stacks, func)
