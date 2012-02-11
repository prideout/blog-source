#!/usr/bin/python

ThreadCount = 4
ShadingRate = 4 # Bigger is faster
#ImageFormat = (960,400,1)
ImageFormat = (1024,428,1)
#ImageFormat = (1920,803,1)
#ImageFormat = (1600,670,1)
#ImageFormat = (1600,1200,1)
#ImageFormat = (1600,900,1)
PixelSamples = (4,4)
OccSamples = 256
FrameDuration = 1.0 / 24.0
ShutterDuration = 1.0 / 30.0
SingleFrame = True

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

def DrawScene(ri, time):
    """Everything between RiBegin and RiEnd"""

    frameString = "%04d" % DrawScene.counter
    filename = "Art%s.tif" % frameString
    DrawScene.counter += 1

    stats = dict(endofframe=1, xmlfilename='stats.xml', filename='')
    bakeArgs = dict(displaychannels='_occlusion', samples=OccSamples)
    bakeArgs['filename'] = ''
    bakeArgs['hitgroup'] = ''

    ri.Option("limits", {"int threads" : ThreadCount})
    ri.Option("statistics", stats)

    if SingleFrame:
        ri.Display("Art", "framebuffer", "rgba")
    else:
        ri.Display(filename, "file", "rgba")

    ri.Format(*ImageFormat)
    ri.DisplayChannel("float _occlusion")
    ri.ShadingRate(ShadingRate)
    ri.PixelSamples(*PixelSamples)
    ri.Projection(ri.PERSPECTIVE, {ri.FOV: 30})
    ri.Shutter(time, time + ShutterDuration)
    ri.Translate(1.75, -1.25, 15)
    ri.Rotate(-20, 1, 0, 0)
    ri.Rotate(180, 1, 0, 0)
    curve = [1, 1, 0.8, 0.1, 0.9, 0.2, 1, 1, 1, 1]
    ri.Camera("world", {"float[10] shutteropening": curve})
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
    ri.Disk(0.55, 300, 360)
    ri.TransformEnd()
 
    ri.SetLabel('Sculpture')
    bakeArgs['color em'] = (1.25,1.25,1.4)
    ri.Surface("ComputeOcclusion", bakeArgs)
    tree = GenerativeArt.Library["Nouveau"]
    shapes = GenerativeArt.Evaluate(tree, seed = 29)
    ri.TransformBegin()
    ri.Rotate(90, 1, 0, 0)
    ri.Translate(0, 0, -0.55)
    for shape in shapes:
        name, matrix = shape
        ri.TransformBegin()
        ri.ConcatTransform(list(matrix))
        if name == "tubey":
            ri.Rotate(90,1,0,0)
            radius = 0.5
            ri.Cylinder(radius, 0, 0.5, 360)
        elif name == "boxy":
            X = 0.1
            ri.PointsPolygons([4, 4, 4, 4],
                          [2,3,1,0, 1,5,4,0, 7,6,4,5, 3,2,6,7],
                          {ri.P:[-X, -X, -X, X, -X, -X, -X, X, -X, X, X, -X, -X, -X, X, X, -X, X, -X, X, X, X, X, X]})
        elif name == "box":
            X = 0.1
            ri.PointsPolygons([4, 4, 4, 4, 4, 4],
                          [2,3,1,0,3,7,5,1,1,5,4,0,6,2,0,4,7,6,4,5,3,2,6,7],
                          {ri.P:[-X, -X, -X, X, -X, -X, -X, X, -X, X, X, -X, -X, -X, X, X, -X, X, -X, X, X, X, X, X]})
        else:
            print "unrecognized shape: ", name
            quit()
        ri.TransformEnd()
    ri.TransformEnd()
    ri.WorldEnd()
DrawScene.counter = 0

if __name__ == "__main__":

    if sys.argv[-1] == "clean":
        Clean()
        quit()

    Compile('ComputeOcclusion')
    Compile('UseOcclusion')
    Compile('Vignette')
    prman.Ri.SetLabel = SetLabel
    ri = prman.Ri()

    if SingleFrame:
        ShutterDuration = 0
        ri.Begin("launch:prman? -ctrl $ctrlin $ctrlout")
        DrawScene(ri, 0)
        ReportProgress()
        ri.End()
        PrintStats() 
    else:
        startTime, endTime = 0, 1
        currentTime = startTime
        while currentTime <= endTime:
            ri.Begin("launch:prman? -ctrl $ctrlin $ctrlout")
            DrawScene(ri, currentTime)
            ReportProgress()
            ri.End()
            currentTime += FrameDuration
            print "Compiling video..."
            os.system(r"ffmpeg -r 24 -vframes 24 -i Art%04d.tif Out.mov")
