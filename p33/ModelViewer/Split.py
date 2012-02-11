#!/usr/bin/python

import os
import shutil

commonFiles = [
    ('.', ('Icon.png', 'Default.png', 'Info.plist', 'main.m')),
    ('Classes', (
            "Interfaces.hpp",
            "ParametricSurface.hpp",
            "ParametricSurface.cpp",
            "ParametricEquations.hpp",
            "Quaternion.hpp",
            "Matrix.hpp",
            "Vector.hpp",
            "AppDelegate.h",
            "AppDelegate.mm"))]

def SplitSample(name, substitutions, folders, hasAppResources, hasRenderResources, createAppEngine, createEs1, createEs2):
    global commonFiles

    hasResources = hasAppResources or hasRenderResources
    target = "../ModelViewer." + name
    targetProject = target + "/" + name + ".xcodeproj"

    try:
        os.mkdir(target)
        os.mkdir(targetProject)
    except:
        pass

    folders.extend(commonFiles)
    hasEs1 = False
    hasEs2 = False
    for folder in folders:
        subfolder = folder[0]
        targetFolder = target + '/' + subfolder

        print "creating %s" % targetFolder
        try:
            os.mkdir(targetFolder)
        except:
            pass

        for file in folder[1]:

            if -1 != file.find("ES1.cpp"):
                hasEs1 = True

            if -1 != file.find("ES2.cpp"):
                hasEs2 = True

            srcFile = subfolder + '/' + file
            destFile = targetFolder + '/' + file
            if -1 != destFile.find('-'):
                srcFile = srcFile[:srcFile.find('-')]
                file = file[file.find('-') + 1:]
                destFile = targetFolder + '/' + file

            if -1 != srcFile.find(".png"):
                if not os.path.isfile(destFile):
                    print "copying from %s to %s" % (srcFile, destFile)
                    shutil.copyfile(srcFile, destFile)
                continue

            print "copying from %s to %s" % (srcFile, destFile)

            destFileObject = open(destFile, 'w')
            for line in open(srcFile):
                for sub in substitutions:
                    line = line.replace(sub[0], sub[1])
                destFileObject.write(line)
        
    targetProjectFilename = targetProject + '/project.pbxproj'
    if not os.path.isfile(targetProjectFilename):
        targetProjectFile = open(targetProjectFilename, 'w')
        for line in open('ModelViewer.xcodeproj/project.pbxproj'):
            newline = line.replace('ModelViewer', name)
            targetProjectFile.write(newline)

    targetGlViewH = open(target + "/Classes/GLView.h", 'w')
    for line in open('Classes/GLView.h'):
        if hasResources or -1 == line.find('m_resourceManager'):
            targetGlViewH.write(line)
    
    targetGlViewMm = open(target + "/Classes/GLView.mm", 'w')
    for line in open('Classes/GLView.mm'):
        if -1 != line.find('Darwin'):
            if hasResources:
                targetGlViewMm.write(line)
        elif line.find("::CreateApplicationEngine") != -1:
            if not hasAppResources:
                targetGlViewMm.write("       m_applicationEngine = " + createAppEngine + "::CreateApplicationEngine(m_renderingEngine);\n")
            else:
                targetGlViewMm.write("       m_applicationEngine = " + createAppEngine + "::CreateApplicationEngine(m_renderingEngine, m_resourceManager);\n")
        elif line.find("OpenGL ES 1.1") != -1:
            targetGlViewMm.write(line)
            if not hasRenderResources:
                targetGlViewMm.write("            m_renderingEngine = " + createEs1 + "::CreateRenderingEngine();\n")
            else:
                targetGlViewMm.write("            m_renderingEngine = " + createEs1 + "::CreateRenderingEngine(m_resourceManager);\n")
        elif line.find("OpenGL ES 2.0") != -1:
            targetGlViewMm.write(line)
            if not hasRenderResources:
                targetGlViewMm.write("            m_renderingEngine = " + createEs2 + "::CreateRenderingEngine();\n")
            else:
                targetGlViewMm.write("            m_renderingEngine = " + createEs2 + "::CreateRenderingEngine(m_resourceManager);\n")
        elif line.find("api = kEAGLRenderingAPIOpenGLES2") != -1 and not hasEs2:
            line = line.replace("ES2", "ES1")
            targetGlViewMm.write(line)
        elif line.find("m_renderingEngine = ") == -1:
            targetGlViewMm.write(line)
    if not hasEs1:
        targetGlViewMm.write("namespace SolidES1 { IRenderingEngine* CreateRenderingEngine() { return 0; } }\n")

samples = [

    ("SimpleWireframe", [],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "RenderingEngine.WireframeES1.cpp",
                   "RenderingEngine.WireframeES2.cpp")),
      ('Shaders', ("Simple.es2.vert",
                   "Simple.es2.frag"))],
     False, False, "ParametricViewer", "WireframeES1", "WireframeES2"),

    ("VertexLighting", [],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "RenderingEngine.SolidES1.cpp",
                   "RenderingEngine.SolidES2.cpp")),
      ('Shaders', ("SimpleLighting.es2.vert",
                   "SimpleLighting.es2.frag"))],
     False, False, "ParametricViewer", "SolidES1", "SolidES2"),

    ("PixelLighting", [("SimpleLighting", "PixelLighting")],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "RenderingEngine.SolidES2.cpp")),
      ('Shaders', ("PixelLighting.es2.vert",
                   "PixelLighting.es2.frag"))],
     False, False, "ParametricViewer", "SolidES1", "SolidES2"),

    ("ToonLighting", [("SimpleLighting", "ToonLighting")],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "RenderingEngine.SolidES2.cpp")),
      ('Shaders', ("ToonLighting.es2.vert",
                   "ToonLighting.es2.frag"))],
     False, False, "ParametricViewer", "SolidES1", "SolidES2"),

    ("FancyWireframe", [],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "RenderingEngine.FacetedES1.cpp"))],
     False, False, "ParametricViewer", "FacetedES1", "FacetedES2"),

    ("ObjViewer", [],
     [('Classes', ("ApplicationEngine.ObjViewer.cpp",
                   "ResourceManager.mm",
                   "ObjSurface.hpp",
                   "ObjSurface.cpp",
                   "RenderingEngine.SolidES1.cpp",
                   "RenderingEngine.SolidES2.cpp")),
      ('Shaders', ("SimpleLighting.es2.vert",
                   "SimpleLighting.es2.frag")),
      ('Models', ("Ninja.obj", "credits.txt", "micronapalmv2.obj"))],
     True, False, "ObjViewer", "SolidES1", "SolidES2"),

    ("Textured", [],
     [('Classes', ("ApplicationEngine.ParametricViewer.cpp",
                   "ResourceManager.mm",
                   "RenderingEngine.TexturedES1.cpp",
                   "RenderingEngine.TexturedES2.cpp")),
      ('Textures', ["Grid16.png"]),
      ('Shaders', ("TexturedLighting.es2.vert",
                   "TexturedLighting.es2.frag"))],
     False, True, "ParametricViewer", "TexturedES1", "TexturedES2"),
];
     
for sample in samples:
    SplitSample(*sample)

