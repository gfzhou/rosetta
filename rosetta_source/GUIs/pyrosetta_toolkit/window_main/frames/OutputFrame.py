
#!/usr/bin/python

# (c) Copyright Rosetta Commons Member Institutions.
# (c) This file is part of the Rosetta software suite and is made available under license.
# (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
# (c) For more information, see http://www.rosettacommons.org. Questions about this can be
# (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

## @file   /GUIs/pyrosetta_toolkit/window_main/frames/output.py
## @brief  Handles the main output component of the GUI.
## @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

#Rosetta Imports
from rosetta import *

#Tkinter Imports
from Tkinter import *

#Toolkit Imports
from modules.tools import input as input_tools
from modules.tools import output as output_tools
from modules.tools import sequence as sequence_tools

class OutputFrame(Frame):
    def __init__(self, main, toolkit, output_class, **options):
        #Initialize frame, which becomes master instead of main - Which is why we use self in making the GUI components
        Frame.__init__(self, main, **options)
        self.output_class = output_class; #Basically an OP to outputclass
        self.toolkit = toolkit; #Basically an AP to toolkit

        
        self.create_GUI_objects()
        self.grid_GUI_objects()
        
    def create_GUI_objects(self):
        #self.label_output_options = Label(self, text="Output Options", font=("Arial"))
        self.label_output_path = Label(self, text="Decoy Output Path:")
        self.label_output_name = Label(self, text="Decoy Output Name:")
        self.button_Out=Button(self, text="Choose", command=lambda: self.output_class.outdir.set(input_tools.tk_get_directory()))
        self.entry_Outname = Entry(self, textvariable = self.output_class.outname)
        self.check_button_auto_write = Checkbutton(self, text="Write after protocol?", variable = self.output_class.auto_write)
        self.button_dump_pose = Button(self, text="Write Current Pose", command=lambda: output_tools.dumpPDB(self.toolkit.pose, self.toolkit.native_pose, self.output_class.outdir.get() + "/" + self.output_class.outname.get(), self.toolkit.score_class.score))


        
    def grid_GUI_objects(self):
        #self.label_output_options.grid(row=0, column=0, columnspan=2, sticky=W+E)
        self.label_output_path.grid(row=1, column=0, sticky=W)
        self.label_output_name.grid(row=2, column=0, sticky=W)
        self.button_Out.grid(row=1, column=1, sticky=W+E)
        self.entry_Outname.grid(row=2, column=1)
        self.button_dump_pose.grid(row=3, column=1)
        
        self.check_button_auto_write.grid(row=3, column=0)
