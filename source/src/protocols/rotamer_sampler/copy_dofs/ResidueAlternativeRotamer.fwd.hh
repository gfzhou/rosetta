// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/rotamer_sampler/copy_dofs/ResidueAlternativeRotamer.fwd.hh
/// @brief 
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu


#ifndef INCLUDED_protocols_rotamer_sampler_copy_dofs_ResidueAlternativeRotamer_FWD_HH
#define INCLUDED_protocols_rotamer_sampler_copy_dofs_ResidueAlternativeRotamer_FWD_HH

#include <utility/pointer/owning_ptr.hh>

namespace protocols {
namespace rotamer_sampler {
namespace copy_dofs {
	
	class ResidueAlternativeRotamer;
	typedef utility::pointer::owning_ptr< ResidueAlternativeRotamer > ResidueAlternativeRotamerOP;
	typedef utility::pointer::owning_ptr< ResidueAlternativeRotamer const > ResidueAlternativeRotamerCOP;
	
} //copy_dofs
} //rotamer_sampler
} //protocols

#endif
