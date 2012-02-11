#!/usr/bin/python

from PIL import Image
import os

width = 0
height = 0

for x in xrange(1,5):
    for y in xrange(1,5):
        filename = 'Inner%d-Outer%d.png' % (x, y)
        image = Image.open(filename)
        image.thumbnail((80, 80), Image.ANTIALIAS)
        filename = 'Inner%d-Outer%d-Thumb.png' % (x, y)
        image.save(filename)
