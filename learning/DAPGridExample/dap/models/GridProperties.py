
class GridProperties:
   def __init__(self, minx = -180, miny = -90, minz = 0, maxx = 180, maxy = 90, maxz = 0, coordinate_system = '<?xml version="1.0" encoding="UTF-8"?><projection type="GEOGRAPHIC" name="WGS 84" ellipsoid="WGS 84" datum="WGS 84" wellknown_epsg="4326" datumtrf="WGS 84" datumtrf_description="[WGS 84] World" radius="6378137" eccentricity="0.08181919084" xmlns="http://www.geosoft.com/schema/geo"><shift x="0" y="0" z="0"/><units name="dega" unit_scale="1"/></projection>', xorigin = 0, yorigin = 0, xsize = 0, ysize = 0, xcellsize = 0, ycellsize = 0):
      self.NativeMinX = minx
      self.NativeMinY = miny
      self.NativeMinZ = minz
      self.NativeMaxX = maxx
      self.NativeMaxY = maxy
      self.NativeMaxZ = maxz
      self.CoordinateSystem = coordinate_system
      self.XOrigin = xorigin
      self.YOrigin = yorigin
      self.XSize = xsize
      self.YSize = ysize
      self.XCellSize = xcellsize
      self.YCellSize = ycellsize

   def __str__(self):
      return 'Origin (%s,%s) Size (%s,%s) CellSize (%s,%s) Extents [%s,%s,%s] - [%s,%s,%s] %s' % (self.XOrigin, self.YOrigin, self.XSize, self.YSize, self.XCellSize, self.YCellSize, self.NativeMinX, self.NativeMinY, self.NativeMinZ, self.NativeMaxX, self.NativeMaxY, self.NativeMaxZ, self.CoordinateSystem)
   
   def __repr__(self):
      return 'GridProperties(%r,%r,%r,%r,%r,%r,%r,%r,%r,%r,%r,%r,%r)' % (self.XOrigin, self.YOrigin, self.XSize, self.YSize, self.XCellSize, self.YCellSize, self.NativeMinX, self.NativeMinY, self.NativeMinZ, self.NativeMaxX, self.NativeMaxY, self.NativeMaxZ, self.CoordinateSystem)
      
