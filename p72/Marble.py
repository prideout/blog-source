#!/usr/bin/python

ThreadCount = 4
ShadingRate = 4
ImageFormat = (1500,600,1)
PixelSamples = (4,4)
OccSamples = 512
Crop = None

#ImageFormat = (960,400,1)
#ImageFormat = (1920,803,1)
#ImageFormat = (1600,670,1)
#ImageFormat = (1600,1200,1)
#ImageFormat = (1600,900,1)

import sys
import os
import time
import LSystem
import euclid
from itertools import izip

if 'RMANTREE' in os.environ:
    from sys import path as module_paths
    path = os.environ['RMANTREE'] + '/bin'
    module_paths.append(path)

import prman

def SetLabel( self, label, groups = '' ):
    """Sets the id and ray group(s) for subsequent gprims"""
    self.Attribute(self.IDENTIFIER,{self.NAME:label})
    if groups != '':
        self.Attribute("grouping",{"membership":groups})

def Compile(shader):
    """Compiles the given RSL file"""
    print 'Compiling %s...' % shader
    retval = os.system("shader %s.sl" % shader)
    if retval:
        quit()

def CreateBrickmap(base):
    """Creates a brick map from a point cloud"""
    if os.path.exists('%s.bkm' % base):
        print "Found brickmap for %s" % base
    else:
        print "Creating brickmap for %s..." % base
        if not os.path.exists('%s.ptc' % base):
            print "Error: %s.ptc has not been generated." % base
        else:
            os.system("brickmake %s.ptc %s.bkm" % (base, base))

def Clean():
    """Removes build artifacts from the current directory"""
    from glob import glob
    filespec = "*.slo *.bkm *.ptc *.xml *.tif *.mov"
    patterns = map(glob, filespec.split(" "))
    for files in patterns:
        map(os.remove, files)
    
def ReportProgress():
    """Communicates with the prman process, printing progress"""
    previous = progress = 0
    while progress < 100:
        prman.RicProcessCallbacks()
        progress = prman.RicGetProgress()
        if progress == 100 or progress < previous:
            break
        if progress != previous:
            print "\r%04d - %s%%" % (ReportProgress.counter, progress),
            previous = progress
            time.sleep(0.1)
    print "\r%04d - 100%%" % ReportProgress.counter
    ReportProgress.counter += 1
ReportProgress.counter = 0

Cages = []
Curves = []
def DrawScene(ri, drawPass):
    """Everything between RiBegin and RiEnd"""

    frameString = "%04d" % DrawScene.counter
    DrawScene.counter += 1

    if Crop != None:
        crop = (Crop[0] / ImageFormat[0], 
                (Crop[0] + Crop[2]) / ImageFormat[0],
                Crop[1] / ImageFormat[1],
                (Crop[1] + Crop[3]) / ImageFormat[1])
        ri.CropWindow(*crop)

    ri.Option("limits", {"int threads" : ThreadCount})

    if drawPass == 1:
        ri.Display("Art", "framebuffer", "rgba")
    else:
        ri.Display("temp", "file", "rgba")

    ri.Format(*ImageFormat)

    if drawPass == 0:
        ri.DisplayChannel("float _area")
        ri.DisplayChannel("color _radiance_t")
    else:
        ri.DisplayChannel("float _occlusion")
 
    ri.ShadingRate(ShadingRate)
    ri.ShadingInterpolation("smooth")
    ri.PixelSamples(*PixelSamples)
    ri.Projection(ri.PERSPECTIVE, {ri.FOV: 30})
    ri.Translate(-3, -1.5, 16)
    ri.Rotate(160, 1, 0, 0)
    ri.Rotate(200+72, 0, 1, 0)
    curve = [1, 1, 0.8, 0.1, 0.9, 0.2, 1, 1, 1, 1]
    ri.Imager("Vignette", {"background": (0.0/255.0,165.0/255.0,211.0/255.0)})
    ri.WorldBegin()

    ri.Declare("displaychannels", "string")
    ri.Declare("samples", "float")
    ri.Declare("coordsys", "string")
    ri.Declare("hitgroup", "string")
    ri.Declare("unitlength", "float")
    ri.Declare("Kd", "float")
    ri.Declare("Ks", "float")
    ri.Declare("roughness", "float")
    ri.Declare("background", "color")

    ri.Attribute("cull", dict(hidden=0,backfacing=0))
    ri.Attribute("dice", dict(rasterorient=0))
    ri.Attribute("visibility", dict(diffuse=1,specular=1,transmission=0))

    if drawPass == 1:
        ri.SetLabel('Floor')
        bakeArgs = dict(displaychannels='_occlusion', samples=OccSamples)
        bakeArgs['filename'] = ''
        bakeArgs['color em'] = (0.0/255.0,165.0/255.0,211.0/255.0)
        ri.Surface("ComputeOcclusion", bakeArgs)
        ri.TransformBegin()
        ri.Rotate(90, 1, 0, 0)
#        ri.Disk(-0.7, 300, 360)
        ri.TransformEnd()
#        ri.Attribute("trace",dict(bias=.01))

    ri.SetLabel('Sculpture')

    ri.LightSource("distantlight", {
            "intensity":[2],
            "from":[0, 2, 50],
            "to":[0, 0, 0]})

    ri.LightSource("distantlight", {
            "intensity":[0.0125],
            "from":[-5, 2, 0],
            "to":[0, 0, 0]})

    shaders, args = "", {}
    if drawPass == 0:
        shader = "BakeRadiance"
        args['filename'] = 'ssdiffusion.ptc'
        args['displaychannels'] = "_area,_radiance_t"
    else:
        shader = "UseRadiance"
        args['filename'] = 'ssdiffusion.optc'
        args['unitlength'] = 0.125
        args['Kd'] = 0.125
        args['Ks'] = 0.125
        args['roughness'] = 0.7
        args['samples'] = OccSamples
    ri.Surface(shader, args)

    ri.TransformBegin()
    ri.Rotate(90, 1, 0, 0)
    ri.Translate(0, 0, -0.55)

    width = 0.75
    height = 0.4
    sharpness = 2.0

    if len(Cages) == 0:
        tree = open(RulesFile).read()
        shapes = LSystem.Evaluate(tree, seed = 29)
        points, normals, cage = [], [], []
        previous = None
        for shape in shapes:
            if shape == None:
                if len(points) > 0:
                    Curves.append((points, normals))
                    Cages.append(cage)
                    previous = None
                    points, normals, cage = [], [], []
                continue
            P, N = shape
            points += P[:]
            normals += N[:]
            if previous != None:
                D = (P - previous).normalized()
                T = D.cross(N)
                c0 = P + width * T + height * N
                c1 = P + width * T - height * N
                c2 = P - width * T + height * N
                c3 = P - width * T - height * N
                cage += c0[:]
                cage += c1[:]
                cage += c2[:]
                cage += c3[:]
            previous = P
        if len(points) > 0:
            Curves.append((points, normals))
            Cages.append(cage)

    #for points, normals in Curves:
    #    ri.Curves("linear", [len(points)/3], "nonperiodic", {ri.P:points, ri.N:normals, "constantwidth": width})

    shell = [6, 4, 0, 2,
             0, 4, 5, 1,
             1, 5, 7, 3,
             3, 7, 6, 2]

    for cage in Cages:
        slices = len(cage) / 12
        faces = 4 * (slices - 1)
        indices = [x + n for x in xrange(0, faces, 4) for n in shell]
        edge0 = range(0, faces + 0, 4)
        edge1 = range(1, faces + 1, 4)
        edge2 = range(2, faces + 2, 4)
        edge3 = range(3, faces + 3, 4)
        ri.SubdivisionMesh(
            "catmull-clark",
            [4] * faces, # Indices per face
            indices,
            ["crease"] * 4, # Tags
            [slices - 1, 1] * 4, # Args per tag
            edge0 + edge1 + edge2 + edge3, # Each tag's integer args
            [sharpness] * 4, # Each tag's float args
            {"P": cage}) # Verts

    ri.TransformEnd()
    ri.WorldEnd()
DrawScene.counter = 0

def OrganizePoints(basename):
    """Process a point cloud into a spatial divisiom hierarchy"""
    print 'Processing %s...' % basename
    retval = os.system("ptfilter -filter ssdiffusion -followtopology 1 -partial 1 %s.ptc %s.optc" % (basename, basename))    
    if retval:
        quit()

if __name__ == "__main__":

    if sys.argv[-1] == "clean":
        Clean()
        quit()

    Compile('ComputeOcclusion')
    Compile('BakeRadiance')
    Compile('UseRadiance')
    Compile('Vignette')
    prman.Ri.SetLabel = SetLabel
    ri = prman.Ri()
    
    for drawPass in xrange(2):
        ri.Begin("launch:prman? -ctrl $ctrlin $ctrlout")
        DrawScene(ri, drawPass)
        ReportProgress()
        ri.End()
        if drawPass == 0:
            OrganizePoints("ssdiffusion")
