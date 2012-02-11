#!/usr/bin/python

ThreadCount = 4
ShadingRate = 4 # Bigger is faster
ImageFormat = (1920/2,803/2,1)
PixelSamples = (2,2)
OccSamples = 256
Crop = None  # left, top, width, height

import sys
import os
import time
import GenerativeArt
import euclid

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

def PrintStats():
    """Looks at the XML output and dumps render time."""
    try:    
        from elementtree.ElementTree import ElementTree
    except:
        print "Unable to load ElementTree, skipping statistics."
    else:
        doc = ElementTree(file='stats.xml')
        for timer in doc.findall('//timer'):
            if "totaltime" == timer.get("name"):
                print "Render time was %s seconds" % timer[0].text
                break

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

def DrawScene(ri, time):
    """Everything between RiBegin and RiEnd"""

    frameString = "%04d" % DrawScene.counter
    filename = "Art%s.tif" % frameString
    DrawScene.counter += 1

    stats = dict(endofframe=1, xmlfilename='stats.xml', filename='')
    bakeArgs = dict(displaychannels='_occlusion', samples=OccSamples)
    bakeArgs['filename'] = ''
    bakeArgs['hitgroup'] = ''

    if Crop:
        crop = (Crop[0] / ImageFormat[0], 
                (Crop[0] + Crop[2]) / ImageFormat[0],
                Crop[1] / ImageFormat[1],
                (Crop[1] + Crop[3]) / ImageFormat[1])
        ri.CropWindow(*crop)

    ri.Option("limits", {"int threads" : ThreadCount})
    ri.Display("far", "framebuffer", "rgba")
    ri.Format(*ImageFormat)
    ri.DisplayChannel("float _occlusion")
    ri.ShadingRate(ShadingRate)
    ri.PixelSamples(*PixelSamples)
    ri.Projection(ri.PERSPECTIVE, {ri.FOV: 5})
    ri.Translate(2, 0, 120)
    ri.Rotate(-3, 1, 0, 0)
    ri.Rotate(180, 1, 0, 0)
    ri.Rotate(180, 0, 1, 0)
    ri.Imager("Vignette")
    ri.WorldBegin()
    ri.Declare("displaychannels", "string")
    ri.Declare("samples", "float")
    ri.Declare("coordsys", "string")
    ri.Declare("hitgroup", "string")
    ri.Attribute("cull", dict(hidden=0,backfacing=0))
    ri.Attribute("dice", dict(rasterorient=0))
    ri.Attribute("visibility", dict(diffuse=1,specular=1))

    ri.SetLabel('Floor')
    bakeArgs['color em'] = (0.0/255.0,165.0/255.0,211.0/255.0)
    ri.Surface("ComputeOcclusion", bakeArgs)
    ri.TransformBegin()
    ri.Rotate(90, 1, 0, 0)
    ri.Disk(-0.7, 300, 360)
    ri.TransformEnd()
 
    ri.SetLabel('Sculpture')
    bakeArgs['color em'] = (1.0,1.0,1.0)
    bakeArgs['Ks'] = 1.0
    ri.Surface("ComputeOcclusion", bakeArgs)

    ri.TransformBegin()
    ri.Rotate(90, 1, 0, 0)
    ri.Translate(0, 0, -0.55)

    width = 0.5
    height = 0.1
    sharpness = 0.5

    if len(Cages) == 0:
        tree = GenerativeArt.Library["Nouveau"]
        shapes = GenerativeArt.Evaluate(tree, seed = 29)
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

if __name__ == "__main__":

    if sys.argv[-1] == "clean":
        Clean()
        quit()

    Compile('ComputeOcclusion')
    Compile('Vignette')
    prman.Ri.SetLabel = SetLabel
    ri = prman.Ri()

    ri.Begin("launch:prman? -ctrl $ctrlin $ctrlout")
    DrawScene(ri, 0)
    ReportProgress()
    ri.End()
