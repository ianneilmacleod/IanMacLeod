import os
class Config:
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'de6e7d9d-169b-4768-bc4e-d76185bc9178'