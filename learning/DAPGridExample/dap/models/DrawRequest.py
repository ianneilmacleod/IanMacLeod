from . import BoundingBox

class DrawRequest:
   def __init__(self, width=1600, height=1600, extents = BoundingBox()):
      self.Width=width
      self.Height=height
      self.BoundingBox = extents

   def __str__(self):
      return '(%s, %s) Extents: %s' % (self.Width, self.Height, self.BoundingBox)

   def __repr__(self):
      return 'DrawRequest(width=%r,height=%r,extents=%r)' % (self.Width, self.Height, self.BoundingBox)
