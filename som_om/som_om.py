# -*- coding: utf-8 -*-
"""
Created on Sun Jan  5 10:15:34 2014

@author: Ian
"""

#The following 2 lines to support remote debugging
#import pydevd
#pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

import os
import json

import geosoft.gxapi as gxapi
import geosoft.gxpy.utility as gxu
import geosoft.gxpy.om as gxom

#t translation
def _(s): return s

def rungx():
    #pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

    # get database state
    try:
        state = gxom.state()['gdb']
        gdb_name = state['current']
    except:
        gxom.message(_('No current database'), _('An open database is required.'))

    # settings
    settings = gxu.get_parameters('SOM_OM')

    # if different database, reset database-dependent settings
    if settings.get('gdb_name', '') != gdb_name:
        settings['gdb_name'] = gdb_name
        settings['input_data'] = sorted(state['disp_chan_list'], key=str.lower)

    # analyse data
    gxapi.GXEDB.un_load(gdb_name)
    try:
        script = os.path.join(os.path.split(__file__)[0], 'som_om_qt5.py')
        results = gxu.run_external_python(script, settings)
    except:
        gxapi.GXEDB.load(gdb_name)
        raise

    # save results
    gxu.save_parameters('SOM_OM', results)

    gxapi.GXEDB.load(gdb_name)

