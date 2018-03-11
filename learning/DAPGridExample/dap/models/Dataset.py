from enum import Enum

from . import BoundingBox

class DatasetType(Enum):
   Map = 0
   Grid = 1
   Picture = 2
   Point = 3
   Database = 4
   Document = 5
   SPF = 6
   Generic = 7
   Voxel = 8
   ArcGIS= 9
   ImageServer= 10
   PictureSection = 11
   GridSection = 12
   ProjectZip = 13
   Drillhole = 14
   NoData = 15
   ThreeDV = 16
   Geostring = 17
   GMSYS3D = 18
   VOXI = 19
   PDF = 20
   Geosurface = 21
   GMSYS2D = 22
   VectorVoxel = 23
   GeosoftOffline = 24


class Dataset:
   def __init__(self, id = None, title = None, type = 0, hierarchy = None, stylesheet = None, extents = BoundingBox(), has_original = False):
      self.Id = id
      self.Title = title
      self.Type = type
      self.Hierarchy = hierarchy
      self.Stylesheet = stylesheet
      self.Extents = extents
      self.HasOriginal = has_original

   def __str__(self):
      return 'Id: %s, Title: %s, Type: %s, Hierarchy: %s, Extents: %s' % (self.Id, self.Title, self.Type, self.Hierarchy, self.Extents)
   
   def __repr__(self):
      return 'Dataset(id=%r,title=%r,type=%r,hierarchy=%r,stylesheet=%r,extents=%r,has_original=%r)' % (self.Id, self.Title, self.Type, self.Hierarchy, self.Stylesheet, self.Extents, self.HasOriginal)
