#!/bin/env python

from EMAN2 import *
import os

img = os.environ["HOME"] + "/images/monomer.mrc"
x=EMData()
x.read_image(img)
x.rotate_translate(1.0329837512591338,3.7260642381912579,5.7671541529246966,12,12,12)
x.write_image("z.mrc")
x.rotate_translate(1.0329837512591338,3.7260642381912579,5.7671541529246966,16,16,16)
