#import pydevd
import os
import geosoft.gxpy.utility as gxu

def rungx():

    #pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

    script = os.path.join(os.path.split(__file__)[0], 'nothing_much.py')
    gxu.run_external_python(script)
    input('If you see this, it worked...')
