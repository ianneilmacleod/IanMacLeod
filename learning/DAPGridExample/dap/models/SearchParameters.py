from . import SearchFilter
from . import ResultFilter

class SearchParameters:
   def __init__(self, search_filter = SearchFilter(), result_filter = ResultFilter()):
      self.SearchFilter = search_filter
      self.ResultFilter = result_filter

   def __str__(self):
      return 'SearchFilter: %s, ResultFilter: %s' % (self.SearchFilter, self.ResultFilter)
   
   def __repr__(self):
      return 'SearchParameters(search_filter=%r,result_filter=%r)' % (self.SearchFilter, self.ResultFilter)
