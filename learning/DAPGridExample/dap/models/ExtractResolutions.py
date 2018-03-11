class ExtractResolutions(object):
   """description of class"""
   def __init__(self, default_resolution = 0, resolutions = None):
      self.Default = default_resolution
      self.Resolutions = resolutions

   def __str__(self):
      return 'Default: %s, Resolutions: %s' % (self.Default, self.Resolutions)
   
   def __repr__(self):
      return 'ExtractResolutions(default_resolution=%r,resolutions=%r)' % (self.Default, self.Resolutions)


