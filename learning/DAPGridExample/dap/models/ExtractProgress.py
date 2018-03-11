from enum import Enum

class ExtractProgressStatus(Enum):
   Prepare = 0
   Extract = 1
   Compress = 2
   Complete = 3
   Cancelled = 4
   Failed = 5

class ExtractProgress(object):
   """description of class"""

   def __init__(self, percent_complete = 0, message = None, stage = ExtractProgressStatus.Prepare):
      self.PercentComplete = percent_complete
      self.Message = message
      self.Stage = stage

   def __str__(self):
      return 'PercentComplete: %s, Message: %s, Stage: %s' % (self.PercentComplete, self.Message, self.Stage)
   
   def __repr__(self):
      return 'ExtractProgress(percent_complete=%r,message=%r,stage=%r)' % (self.PercentComplete, self.Message, self.Stage)


