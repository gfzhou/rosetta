// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/glycopeptide_docking/GlycopeptideDockingLowResRefinement.fwd.hh
/// @brief Low resolution refinement of substrate
/// @author Yashes Srinivasan (yashess@gmail.com)

#ifndef INCLUDED_protocols_glycopeptide_docking_GlycopeptideDockingLowResRefinement_fwd_hh
#define INCLUDED_protocols_glycopeptide_docking_GlycopeptideDockingLowResRefinement_fwd_hh

// Utility headers
#include <utility/pointer/owning_ptr.hh>


// Forward
namespace protocols {
namespace glycopeptide_docking {

class GlycopeptideDockingLowResRefinement;

typedef utility::pointer::shared_ptr< GlycopeptideDockingLowResRefinement > GlycopeptideDockingLowResRefinementOP;
typedef utility::pointer::shared_ptr< GlycopeptideDockingLowResRefinement const > GlycopeptideDockingLowResRefinementCOP;

} //protocols
} //glycopeptide_docking

#endif //INCLUDED_protocols_glycopeptide_docking_GlycopeptideDockingLowResRefinement_fwd_hh
