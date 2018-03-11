
class BoundingBox:
   def __init__(self, minx = -180, miny = -90, minz = 0, maxx = 180, maxy = 90, maxz = 0, coordinate_system = '<?xml version="1.0" encoding="UTF-8"?><projection type="GEOGRAPHIC" name="WGS 84" ellipsoid="WGS 84" datum="WGS 84" wellknown_epsg="4326" datumtrf="WGS 84" datumtrf_description="[WGS 84] World" radius="6378137" eccentricity="0.08181919084" xmlns="http://www.geosoft.com/schema/geo"><shift x="0" y="0" z="0"/><units name="dega" unit_scale="1"/></projection>' ):
      self.MinX = minx
      self.MinY = miny
      self.MinZ = minz
      self.MaxX = maxx
      self.MaxY = maxy
      self.MaxZ = maxz
      self.CoordinateSystem = coordinate_system

   def __str__(self):
      return '[%s, %s, %s] - [%s, %s %s], %s' % (self.MinX, self.MinY, self.MinZ, self.MaxX, self.MaxY, self.MaxZ, self.CoordinateSystem)
   
   def __repr__(self):
      return 'BoundingBox(minx=%r,miny=%r,minz=%r,maxx=%r,maxy=%r,maxz=%r,coordinate_system=%r)' % (self.MinX, self.MinY, self.MinZ, self.MaxX, self.MaxY, self.MaxZ, self.CoordinateSystem)
