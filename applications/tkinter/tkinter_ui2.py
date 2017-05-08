import tkinter.ttk as ttk

import os
import pythoncom

import geosoft.gxpy as gxpy
import geosoft.gxapi as gxa

if __name__ == "__main__":
    tk = ttk.Frame(master=None)
    gx = gxpy.gx.GXpy(parent_window=tk.winfo_id())
    m = gxa.GXMAP.create(os.path.join(os.path.dirname(__file__), "2dtest.map"), gxa.MAP_WRITEOLD)
    gxa.GXGUI.simple_map_dialog(m, "Test Simple Map", "")
    gxa.GXGUI.show_3d_viewer_dialog("Test 3D", os.path.join(os.path.dirname(__file__), "3dtest.map"), "3D")
    print("done")