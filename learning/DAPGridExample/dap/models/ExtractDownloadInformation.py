class ExtractDownloadInformation(object):
   """description of class"""

   def __init__(self, block_size = 0, number_of_blocks = 0, total_size = 0):
      self.BlockSize = block_size
      self.NumberOfBlocks = number_of_blocks
      self.TotalSize = total_size

   def __str__(self):
      return 'BlockSize: %s, NumberOfBlocks: %s, TotalSize: %s' % (self.BlockSize, self.NumberOfBlocks, self.TotalSize)
   
   def __repr__(self):
      return 'ExtractDownloadInformation(block_size=%r,number_of_blocks=%r,total_size=%r)' % (self.BlockSize, self.NumberOfBlocks, self.TotalSize)

