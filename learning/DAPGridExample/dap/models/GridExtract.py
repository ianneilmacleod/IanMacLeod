from enum import Enum

from . import BoundingBox

class GridExtractFormat(Enum):
   GeosoftCompressed = 0
   GeosoftUncompressed = 1
   ESRIBinaryRaster = 2
   BIL = 3
   Geopak = 4
   GXFText = 5
   GXFCompressed = 6
   ODDFPC = 7
   ODDFUnix = 8
   SurferV6 = 9
   SurferV7 = 10
   USGSPC = 11
   USGSUnix = 12
   ERMapper = 13

class GridExtract(object):
   """description of class"""

   def __init__(self, extents = BoundingBox(), filename = "Untitled", resolution = 0, format = GridExtractFormat.GeosoftCompressed):
      self.BoundingBox = extents
      self.Filename = filename
      self.Resolution = resolution
      self.Format = format

   def __str__(self):
      return 'Resolution: %s, Format: %s, Extents: %s' % (self.Resolution, self.Format, self.BoundingBox)
   
   def __repr__(self):
      return 'GridExtract(extents=%r,filename=%r,resolution=%r,format=%r)' % (self.BoundingBox, self.Filename, self.Resolution, self.Format)

