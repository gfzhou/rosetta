// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//  CVS information:
//  $Revision: 1.1.2.1 $
//  $Date: 2005/11/07 21:05:35 $
//  $Author: rhiju $
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

#ifndef INCLUDED_protocols_rna_RNA_MatchType_HH
#define INCLUDED_protocols_rna_RNA_MatchType_HH

// ObjexxFCL Headers
// AUTO-REMOVED #include <ObjexxFCL/FArray1D.hh>
// AUTO-REMOVED #include <ObjexxFCL/FArray2D.hh>
// AUTO-REMOVED #include <ObjexxFCL/FArray3D.hh>

// AUTO-REMOVED #include <core/pose/Pose.fwd.hh>
#include <core/types.hh>
// you cannot #include yourself #include <protocols/rna/RNA_MatchType.hh>
// AUTO-REMOVED #include <utility/pointer/ReferenceCount.hh>

// C++ Headers
// AUTO-REMOVED #include <string>
// AUTO-REMOVED #include <map>
#include <vector>

#include <utility/vector1.hh>




/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Goal: to make a fragment object that can choose fragments
// "on the fly" for RNA ab inito folding.
//
// After reading in a set of torsions from, e.g., the ribosome crystal structure,
//  should be able to generate fragments of size 1, 2, or 3, with
//  exact sequence matches, partial Y/R matches, or ignoring sequence.
//
namespace protocols{
namespace rna{

	enum _RNA_MatchType_ { MATCH_ALL /* 0 */, MATCH_YR /* 1 */, MATCH_EXACT /* 2 */};
}
}

#endif
