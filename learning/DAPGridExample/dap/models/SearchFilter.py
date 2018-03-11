from . import BoundingBox

class SearchFilter:
   def __init__(self, free_text_query = None, structured_metadata_query = None, extents = BoundingBox(), entirely_within = False, version = 1):
      self.FreeTextQuery = free_text_query
      self.StructuredMetadataQuery = structured_metadata_query
      self.BoundingBox = extents
      self.EntirelyWithin = entirely_within
      self.RequestVersion = version

   def __str__(self):
      return 'FreeTextQuery: %s, StructuredMetadataQuery: %s, EntirelyWithin: %s, Extents: %s' % (self.FreeTextQuery, self.StructuredMetadataQuery, self.EntirelyWithin, self.BoundingBox)
   
   def __repr__(self):
      return 'SearchFilter(free_text_query=%r,structured_metadata_query=%r,extents=%r,entirely_within=%r,version=%r)' % (self.FreeTextQuery, self.StructuredMetadataQuery,self.BoundingBox,self.EntirelyWithin,self.RequestVersion)
