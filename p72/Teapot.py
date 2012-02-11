#!/usr/bin/python

import sys, os, time

def DrawScene(ri, drawPass):

    ri.Option("limits", {"int threads" : 4})
    ri.Format(800, 600, 1)
    ri.ShadingRate(4)
    ri.PixelSamples(4, 4)

    if drawPass == 0:
        ri.Display("bake", "file", "rgba")
    else:
        ri.Display("Teapot", "framebuffer", "rgba")

    ri.Projection(ri.PERSPECTIVE, {ri.FOV: 18})
    ri.Translate(0, 0, 170)
    ri.Rotate(-15, 1, 0, 0)

    if drawPass == 0:
        ri.ShadingInterpolation("smooth")
        ri.DisplayChannel("float _area")
        ri.DisplayChannel("color _radiance_t")

    ri.WorldBegin()
    ri.Declare("samples", "float")
    ri.Declare("displaychannels", "string")
    ri.Attribute("visibility", dict(transmission=1))
    ri.Attribute("trace", dict(bias=0.01))

    if drawPass == 0:
        ri.Attribute("cull", dict(hidden=0,backfacing=0))
        ri.Attribute("dice", dict(rasterorient=0))

    # Set up lights:
    L = []
    L += [{ "intensity" : 10000, "from" : [-60,-60,60], "samples" : 4 }]
    L += [{ "intensity" : 75000, "from" : [   0,0,100], "samples" : 4 }]
    L += [{ "intensity" : 10000, "from" : [50,50,-200], "samples" : 4 }]
    for light in L:
        ri.LightSource("pointlight_rts", light)

    ri.AttributeBegin()
    
    # Set up shaders:
    shaders, args = "", {}
    if drawPass == 0:
        shader = "BakeRadiance"
        args['filename'] = 'ssdiffusion.ptc'
        args['displaychannels'] = "_area,_radiance_t"
    else:
        shader = "UseRadiance"
        args['filename'] = 'ssdiffusion.optc'
    ri.Surface(shader, args)

    # Define geometry:
    ri.Translate(-2, -14, 0)
    ri.Rotate(-90, 1, 0, 0)
    ri.Scale(10, 10, 10)
    ri.Sides(1)
    ri.Geometry("teapot")

    # Tidy up:
    ri.AttributeEnd()
    ri.WorldEnd()


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

def OrganizePoints(basename):
    """Process a point cloud into a spatial divisiom hierarchy"""
    print 'Processing %s...' % basename
    retval = os.system("ptfilter -filter ssdiffusion -partial 1 %s.ptc %s.optc" % (basename, basename))    
    if retval:
        quit()

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

if __name__ == "__main__":

    if 'RMANTREE' in os.environ:
        from sys import path as module_paths
        path = os.environ['RMANTREE'] + '/bin'
        module_paths.append(path)

    Compile('BakeRadiance')
    Compile('UseRadiance')

    import prman
    prman.Ri.SetLabel = SetLabel
    ri = prman.Ri()

    for drawPass in xrange(2):
        ri.Begin("launch:prman? -ctrl $ctrlin $ctrlout")
        #ri.FrameBegin(1)
        DrawScene(ri, drawPass)
        #ri.FrameEnd()
        ReportProgress()
        ri.End()
        if drawPass == 0:
            OrganizePoints("ssdiffusion")

