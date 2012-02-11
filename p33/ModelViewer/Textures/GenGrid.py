
import cairo
import os

def fillSquare(cr):
    cr.rectangle(0,0,1,1)
    cr.set_source_rgb(1,1,1)
    cr.fill_preserve()

def outlineSquare(cr):
    cr.set_source_rgb(0,0,0)
    cr.set_line_width(max(cr.device_to_user_distance(2, 2)))
    cr.stroke()

def renderSquare(s):
    surface = cairo.SVGSurface("Grid%02d.svg" % s, s, s)
    cr = cairo.Context(surface)
    cr.scale(s,s)
    fillSquare(cr)
    if s != 2:
        outlineSquare(cr)
    surface.write_to_png("Grid%02d.png" % s)

for i in [1, 2, 4, 8, 16]:
    renderSquare(i)

for file in os.listdir("."):
    if file.endswith(".svg"):
        os.remove(file)
