from . import BoundingBox

class DefaultExtractionResolutionParameters(object):
   """description of class"""

   def __init__(self, extents = BoundingBox(), type = 1):
      self.BoundingBox = extents
      self.DatasetType = type

   def __str__(self):
      return 'Type: %s, Extents: %s' % (self.DatasetType, self.BoundingBox)
   
   def __repr__(self):
      return 'DefaultExtractionResolutionParameters(extents=%r,type=%r)' % (self.BoundingBox, self.DatasetType)

