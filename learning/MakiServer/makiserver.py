from app import app
from geosoft.gxpy.gx import GXpy
from os import getcwd
gxc = GXpy('MakiServer', "0.0.1")
print(gxc.gid, getcwd())
