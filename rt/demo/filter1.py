#!/usr/bin/python
from EMAN2 import *
import sys

e = EMData()
e.read_image(sys.argv[1])
e.filter("ValueSqrt")
e.filter("RangeMask", {"low":EMObject(5), "high":EMObject(10)})
e.write_image(sys.argv[2], 0, MRC)
