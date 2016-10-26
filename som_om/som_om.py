# -*- coding: utf-8 -*-
"""
Created on Sun Jan  5 10:15:34 2014

@author: Ian
"""
#TODO: Validate on some real data
#TODO: save classification map
#TODO: add Help
#TODO: graphics - show the som adjusting as it goes.

# comment-out following 2 lines to support remote debugging
import pydevd
pydevd.settrace('localhost', port=34765, stdoutToServer=True, stderrToServer=True)

import os,sys
import math
import yaml
import numpy as np
import argparse as argp

from PyQt4 import QtGui

import geosoft.gxpy.gx as gxp
import geosoft.gxpy.gdb as gxgdb

app_folder = os.path.split(__file__)[0]
sys.path.insert(0, app_folder)
import ui_som_om4 as ui

modules_folder = os.path.split(os.path.split(__file__)[0])[0]
sys.path.insert(0, modules_folder)
import modules.mvar as mvar

def decimate(data,maxn):
    ndata = len(data)
    if (ndata > maxn) and (maxn > 0):
        nth = math.ceil(ndata/maxn)
        base = np.arange(0,len(data))
        select = (base%nth)==0
        return(data[select])
    else:
        return(data)

class SOMException(Exception):
    pass

###############################################################################################

class SomDialog(QtGui.QDialog, ui.Ui_som_om):

    def __init__(self, gdb, out_fields=('',''), filter=('','')):
        super(SomDialog, self).__init__(None)
        self.setupUi(self)

        self.gdb = gdb
        self.out_fields = out_fields
        self.filter = filter
        self.stopRequest = False
        self.savedVal = {}
        self.chans = list(self.gdb.channels(chan=gxgdb.CHAN_DISPLAYED))
        self.norms = [0] * len(self.chans)

        #connect slots
        self.classButton.clicked.connect(self.classify)
        self.outClass.textChanged.connect(self.outClassChanged)
        self.norm.currentIndexChanged.connect(self.allNorms)
        self.stopButton.clicked.connect(self.stopIt)

        self.initialiseDialog()

    def refresh(self):

        def channorm(chanList,normList,chan,norm):
            if len(chan):
                chanList.setCurrentIndex(chanList.findText(chan))
                normList.setCurrentIndex(norm)

        #clear channel lists
        self.chan_1.clear()
        self.chan_2.clear()
        self.chan_3.clear()
        self.chan_4.clear()
        self.chan_5.clear()
        self.chan_6.clear()
        self.chan_7.clear()
        self.chan_8.clear()
        self.chan_9.clear()
        self.chan_10.clear()
        self.chan_11.clear()
        self.chan_12.clear()
        self.chan_13.clear()
        self.chan_14.clear()
        self.chan_15.clear()
        self.chan_16.clear()
        self.filterChan.clear()

        #set channel lists to database channels
        chans = self.gdb.channels()
        chans[''] = None
        for c in sorted(chans.keys(), key=lambda k: k.lower()):
            self.chan_1.addItem(c)
            self.chan_2.addItem(c)
            self.chan_3.addItem(c)
            self.chan_4.addItem(c)
            self.chan_5.addItem(c)
            self.chan_6.addItem(c)
            self.chan_7.addItem(c)
            self.chan_8.addItem(c)
            self.chan_9.addItem(c)
            self.chan_10.addItem(c)
            self.chan_11.addItem(c)
            self.chan_12.addItem(c)
            self.chan_13.addItem(c)
            self.chan_14.addItem(c)
            self.chan_15.addItem(c)
            self.chan_16.addItem(c)
            self.filterChan.addItem(c)

        #set default channels
        chans = self.chans
        norms = self.norms
        n = len(chans)
        if n >= 1: channorm(self.chan_1,self.norm_1,chans[0],norms[0])
        if n >= 2: channorm(self.chan_2,self.norm_2,chans[1],norms[1])
        if n >= 3: channorm(self.chan_3,self.norm_3,chans[2],norms[2])
        if n >= 4: channorm(self.chan_4,self.norm_4,chans[3],norms[3])
        if n >= 5: channorm(self.chan_5,self.norm_5,chans[4],norms[4])
        if n >= 6: channorm(self.chan_6,self.norm_6,chans[5],norms[5])
        if n >= 7: channorm(self.chan_7,self.norm_7,chans[6],norms[6])
        if n >= 8: channorm(self.chan_8,self.norm_8,chans[8],norms[8])
        if n >= 9: channorm(self.chan_9,self.norm_9,chans[8],norms[8])
        if n >= 10: channorm(self.chan_10,self.norm_10,chans[9],norms[9])
        if n >= 11: channorm(self.chan_11,self.norm_11,chans[10],norms[10])
        if n >= 12: channorm(self.chan_12,self.norm_12,chans[11],norms[11])
        if n >= 13: channorm(self.chan_13,self.norm_13,chans[12],norms[12])
        if n >= 14: channorm(self.chan_14,self.norm_14,chans[13],norms[13])
        if n >= 15: channorm(self.chan_15,self.norm_15,chans[14],norms[14])
        if n >= 16: channorm(self.chan_16,self.norm_16,chans[15],norms[15])


        #output channels
        self.outClass.setText(self.out_fields[0])
        self.outError.setText(self.out_fields[1])

        #filter
        self.filterChan.setCurrentIndex(self.filterChan.findText(self.filter[0]))
        self.filterVal.setText(self.filter[1])

        #database name
        self.databaseName.setText(self.gdb.fileName())


    def initialiseDialog(self):

        self.refresh()

        for n in ['no','normal','lognorm']:
            self.norm.addItem(n)
            self.norm_1.addItem(n)
            self.norm_2.addItem(n)
            self.norm_3.addItem(n)
            self.norm_4.addItem(n)
            self.norm_5.addItem(n)
            self.norm_6.addItem(n)
            self.norm_7.addItem(n)
            self.norm_8.addItem(n)
            self.norm_9.addItem(n)
            self.norm_10.addItem(n)
            self.norm_11.addItem(n)
            self.norm_12.addItem(n)
            self.norm_13.addItem(n)
            self.norm_14.addItem(n)
            self.norm_15.addItem(n)
            self.norm_16.addItem(n)

        self.stopB(False)

        # similarity
        sf = mvar.similarity_functions()
        for i in sf: self.similarity_func.addItem(str(i))
        self.similarity_func.setCurrentIndex(0)

        # classifications
        lc = mvar.SOM.list_dim()
        for i in lc: self.nClasses.addItem(str(i))
        self.anomPercent.setText("2.0")

    def stopB(self,b):
        self.stopButton.setEnabled(b)
        self.classButton.setEnabled(not b)
        if not b:
            self.stopRequest = False

    def stopIt(self):
        self.stopRequest = True
        self.progLabel.setText('Stopping...')

    def outClassChanged(self):
        outClass = self.outClass.text().strip()
        if outClass:
            self.outError.setText(outClass+'_eud')


    def allNorms(self,index):
        self.norm_1.setCurrentIndex(index)
        self.norm_2.setCurrentIndex(index)
        self.norm_3.setCurrentIndex(index)
        self.norm_4.setCurrentIndex(index)
        self.norm_5.setCurrentIndex(index)
        self.norm_6.setCurrentIndex(index)
        self.norm_7.setCurrentIndex(index)
        self.norm_8.setCurrentIndex(index)
        self.norm_9.setCurrentIndex(index)
        self.norm_10.setCurrentIndex(index)
        self.norm_11.setCurrentIndex(index)
        self.norm_12.setCurrentIndex(index)
        self.norm_13.setCurrentIndex(index)
        self.norm_14.setCurrentIndex(index)
        self.norm_15.setCurrentIndex(index)
        self.norm_16.setCurrentIndex(index)

    def classify(self):

        def progress(label, value=None, som=None):
            self.progLabel.setText(label)
            if value != None: self.progressBar.setValue(int(value))
            QtGui.qApp.processEvents()

        def stop_check():
            QtGui.qApp.processEvents()
            return self.stopRequest

        def addChan(cb,cn,c,n):
            cc = cb.currentText()
            if len(cc) == 0: return
            if cc in c: return
            c.append(cc)
            n.append(cn.currentIndex())

        chan = []
        norm = []
        addChan(self.chan_1 ,self.norm_1 , chan, norm)
        addChan(self.chan_2 ,self.norm_2 , chan, norm)
        addChan(self.chan_3 ,self.norm_3 , chan, norm)
        addChan(self.chan_4 ,self.norm_4 , chan, norm)
        addChan(self.chan_5 ,self.norm_5 , chan, norm)
        addChan(self.chan_6 ,self.norm_6 , chan, norm)
        addChan(self.chan_7 ,self.norm_7 , chan, norm)
        addChan(self.chan_8 ,self.norm_8 , chan, norm)
        addChan(self.chan_9 ,self.norm_9 , chan, norm)
        addChan(self.chan_10,self.norm_10, chan, norm)
        addChan(self.chan_11,self.norm_11, chan, norm)
        addChan(self.chan_12,self.norm_12, chan, norm)
        addChan(self.chan_13,self.norm_13, chan, norm)
        addChan(self.chan_14,self.norm_14, chan, norm)
        addChan(self.chan_15,self.norm_15, chan, norm)
        addChan(self.chan_16,self.norm_16, chan, norm)
        self.chans = chan
        self.norms = norm

        cls = int(self.nClasses.currentText())
        pct = min(max(0.0,float(self.anomPercent.text())),95.0)
        self.filter = (self.filterChan.currentText().strip(), self.filterVal.text().strip())
        if (len(self.filter[0]) == 0) or (len(self.filter[1]) == 0):
            self.filter = ('','')

        self.out_fields = (self.outClass.text(), self.outError.text())
        gdbChans = self.gdb.channels()
        if (self.out_fields[0] in gdbChans) or (self.out_fields[1] in gdbChans):
            butts = QtGui.QMessageBox.Yes
            butts |= QtGui.QMessageBox.No
            response = QtGui.QMessageBox.question(self,"Field exist in database",\
                                                  '"{}" or "{}" exists. Overwrite?'.format(self.out_fields[0],self.out_fields[1]),\
                                                  buttons=butts)
            if response != QtGui.QMessageBox.Yes:
                return

        self.stopB(True)
        try:
            mvar.SOMgdb( self.gdb, chan, dim=cls, per=pct,
                         normalize=norm, ch_filter=self.filter,
                         similarity=self.similarity_func.currentText(),
                         progress=progress, stop=stop_check, out_fields=self.out_fields)

            butts = QtGui.QMessageBox.Yes
            butts |= QtGui.QMessageBox.No
            response = QtGui.QMessageBox.question(self, "Classification complete",
                                                  'Continue classifying?',
                                                  buttons=butts)
            if response == QtGui.QMessageBox.No:
                self.done(0)

        except Exception as e:
            QtGui.QMessageBox.information(self, "Classification failed", '{}'.format(e), buttons=QtGui.QMessageBox.Ok)
            raise


        #refresh UI
        self.stopB(False)
        self.refresh()

        #if new channels in saved unique lists, remove them
        self.savedVal.pop(self.out_fields[0],None)
        self.savedVal.pop(self.out_fields[1],None)

        progress('...',0)

def process(gdb, in_fields=[], out_fields=['a','b']):

    gxc = gxp.GXpy().gxapi
    gxc.enable_application_windows(False)

    try:
        #launch GUI
        app = QtGui.QApplication([])
        form = SomDialog(gdb,out_fields)
        form.show()
        app.exec_()
    except:
        gxc.enable_application_windows(True)
        raise

def rungx():
    gdb = gxgdb.GXdb.open()
    process(gdb)

def main():
    '''
    Self-Organizing maps as a stand-alone Python program.
    :return: 0 on normal exit
    '''

    def progress(label, value=None, som=None):
        if value:
            print('{} {}%'.format(label, value))
        else:
            print(label)

    # get command line parameters
    parser = argp.ArgumentParser(description="SOM analysis of data in a Geosoft database")
    parser.add_argument("-om_state", help="YAML file contains Oasis montaj state")
    args = parser.parse_args()
    print("GeoSOM copyright 2014 Geosoft Inc.\n")

    if args.om_state == None:
        state = {}
    else:
        with open(args.om_state, 'r') as f:
            state = yaml.load(f)
    print(yaml.dump(state))

    currentDB = state.get('db_current', None)
    if currentDB is None:
        raise SOMException("Current database not defined.  \"db_current\" not found in properties.")
    sDbName = currentDB.get('name', None)
    if sDbName is None:
        raise SOMException("Database name not defined.  \"db_current/name\" not found in properties.")

    gxc = gxp.GXpy()
    gdb = gxgdb.GXdb.open(sDbName)
    process(gdb)
    return 0

###############################################################################################
if __name__ == '__main__':
    sys.exit(main())

