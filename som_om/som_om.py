# -*- coding: utf-8 -*-
"""
Created on Sun Jan  5 10:15:34 2014

@author: Ian
"""

#The following 2 lines to support remote debugging
#import pydevd
#pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

import os

import geosoft.gxapi as gxapi
import geosoft.gxpy.utility as gxu
import geosoft.gxpy.om as gxom

#t translation
def _(s): return s

def rungx():
    #pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

    # get database state
    state = gxom.state()
    try:
        gdb_name = state.get('gdb').get('current')
    except:
        gxom.message(_('No current database'), _('An open database is required.'))

    # analyse data
    gxapi.GXEDB.un_load(gdb_name)
    try:
        script = os.path.join(os.path.split(__file__)[0], 'som_om_qt5.py')
        parms = gxu.run_external_python(script, dict=state)
    except:
        gxapi.GXEDB.load(gdb_name)
        raise