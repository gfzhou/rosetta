#!/usr/bin/python

# (c) Copyright Rosetta Commons Member Institutions.
# (c) This file is part of the Rosetta software suite and is made available under license.
# (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
# (c) For more information, see http://www.rosettacommons.org. Questions about this can be
# (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

## @file   /GUIs/pyrosetta_toolkit/modules/help.py
## @brief  Functions for the help menu item.
## @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)


import tkMessageBox
import tkSimpleDialog

#### Collection of help functions. #####

def about():
    """
    This will be a menu pop up
    """
    message = "GUI Originally Created by Jared Adolf-Bryfogle, July 2012.\n"
    message = message + "Roland Dunbrack / FCCC / Drexel College of Medicine.\n"
    message = message + "PyRosetta created by Jeffrey J. Gray, Sergey Lyskov, and the PyRosetta Team at JHU.\n"
    tkMessageBox.showinfo(title="About", message=message)
    #print "Created by Jared Adolf-Bryfogle, Copyright Feb 2010, All Rights Reserved"
    #print "Roland Dunbrack / Fox Chase Cancer Center / Drexel College of Medicine"
    #print "PyRosetta copyright their respective owners."

def mutSA():
    message = "Accessible Surface Area: Square Angstroms, (Chothia 1976)"
    tkMessageBox.showinfo(title="Design Help", message=message)
def mutRM():
    message = "Relative Mutability: Arbitrary units (ALA=100).  Probability Residue will mutate in given time. (Dayhoff 1978)"
    tkMessageBox.showinfo(title="Design Help", message=message)
def mutSP():
    message = "Surface Probability: P(x). Liklihood that 5% or more of the surface area will be exposed to solution. (Chothia 1976)"
    tkMessageBox.showinfo(title="Design Help", message=message)

def show_license():

    message = "(c) Copyright Rosetta Commons Member Institutions. \n \
    (c) This file is part of the Rosetta software suite and is made available under license.\n \
    (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.\n \
    (c) For more information, see http://www.rosettacommons.org. Questions about this can be \n \
    (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu."
    tkMessageBox.showinfo(title="License", message=message)
    
def print_glossary():
    message = "all-atom = in the case of sampling, synonymous with fine movements and often including side chain information; also referred to as high-resolution \n \
    benchmark = another word for a test of a method, scoring function, algorithm, etc. by comparing results from the method to accepted methods/models \n \
    binary file = a file in machine-readable language that can be executed to do something in silico \n \
    BioPython = a set of tools for biological computing written and compatible with Python http://biopython.org/wiki/Biopython \n \
    build = to compile the source code so it can be used as a program \n \
    centroid = in Rosetta centroid mode, side chains are represented as unified spheres centered at the residue?s center of mass\n \
    cluster center \= the geometric center of a cluster, or group, of models \n \
    clustering \= in this case, grouping models with similar structure together \n \
    comparative model \= a protein model where the primary sequence from one protein (target) is placed, or threaded, onto the three dimensional coordinates of a protein of known structure (template)language (binary)"
    tkMessageBox.showinfo(title="Rosetta Glossary from Nature Protocols Paper. Written by Stephanie Hirst DeLuca", message=message)