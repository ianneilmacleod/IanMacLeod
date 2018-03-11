
class ResultFilter:
   def __init__(self, path = None, depth = 2147483647, start_index = 0, max_results = 0, valid_path = False):
      self.Path = path
      self.Depth = depth
      self.StartIndex = start_index
      self.MaxResults = max_results
      self.ValidPath = valid_path

   def __str__(self):
      return 'Path: %s, Depth: %s, StartIndex: %s, MaxResults: %s, ValidPath: %s' % (self.Path, self.Depth,self.StartIndex,self.MaxResults,self.ValidPath)
   
   def __repr__(self):
      return 'ResultFilter(path=%r,depth=%r,start_index=%r,max_results=%r,valid_path=%r)' % (self.Path, self.Depth,self.StartIndex,self.MaxResults,self.ValidPath)
