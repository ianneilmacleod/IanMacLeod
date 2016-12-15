from time import sleep
import tkinter.ttk as ttk

import os
import pythoncom

import geosoft.gxpy as gxpy
import geosoft.gxapi as gxa

class Application(ttk.Frame):
    def __init__(self, master=None):
        ttk.Frame.__init__(self, master)

        self.master.minsize(width=250, height=100)
        self.master.maxsize(width=250, height=100)
        self.master.resizable(width=False, height=False)

        print('Master Window handle {}'.format(self.master.winfo_id()))
        print('Frame Window handle {}'.format(self.winfo_id()))
        self.gxc = gxpy.gx.GXpy(parent_window=self.winfo_id())

        self.pack()
        self.createWidgets()

    def createWidgets(self):
        self.runProgressButton = ttk.Button(self, text='Show Geosoft Progress Dialog',
            command=self.showProgress)
        self.runProgressButton.pack()
        self.viewMapButton = ttk.Button(self, text='Show Map',
            command=self.showMap)
        self.viewMapButton.pack()
        self.view3DButton = ttk.Button(self, text='Show 3D View',
            command=self.show3DViewer)
        self.view3DButton.pack()

    def showProgress(self):
        print("Show Progress")
        
        gxa.GXSYS.progress(1)
        try:
            gxa.GXSYS.prog_name("Processing...", 1)
            for i in range(100):
                gxa.GXSYS.prog_update_l(i, 100)
                sleep(0.25)
        finally:
            gxa.GXSYS.progress(0)
        
    def showMap(self):
        print("Show Map")
        m = gxa.GXMAP.create(os.path.join(os.path.dirname(__file__), "2dtest.map"), gxa.MAP_WRITEOLD)
        gxa.GXGUI.simple_map_dialog(m, "Test Simple Map", "")

    def show3DViewer(self):
        print("Show 3D")
        gxa.GXGUI.show_3d_viewer_dialog("Test 3D", os.path.join(os.path.dirname(__file__), "3dtest.map"), "3D")
        

# running as stand-alone program
if __name__ == "__main__":
    app = Application() 
    app.master.title('Sample application')
    app.mainloop()