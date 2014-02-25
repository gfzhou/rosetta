// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/stepwise/screener/FastForwardToNextResidueAlternative.cc
/// @brief
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu


#include <protocols/stepwise/screener/FastForwardToNextResidueAlternative.hh>
#include <protocols/rotamer_sampler/rigid_body/RigidBodyRotamerWithResidueAlternatives.hh>

#include <basic/Tracer.hh>

static basic::Tracer TR( "protocols.stepwise.screener.FastForwardToNextResidueAlternative" );

namespace protocols {
namespace stepwise {
namespace screener {

	//Constructor
	FastForwardToNextResidueAlternative::FastForwardToNextResidueAlternative( Size const moving_res ):
		moving_res_( moving_res )
	{}

	//Destructor
	FastForwardToNextResidueAlternative::~FastForwardToNextResidueAlternative()
	{}

	////////////////////////////////////////////////////////////////////////////
	// kind of sly -- this normally would be in fast_forward(),
	// but calling that requires 'failure' of screen.
	void
	FastForwardToNextResidueAlternative::get_update( rotamer_sampler::RotamerBaseOP sampler ){
		using namespace rotamer_sampler;
		using namespace rotamer_sampler::rigid_body;

		if ( sampler->type() == RIGID_BODY_WITH_RESIDUE_ALTERNATIVES ) {
			RigidBodyRotamerWithResidueAlternatives & rigid_body_rotamer_with_residue_alternatives = *( static_cast< RigidBodyRotamerWithResidueAlternatives * >( sampler.get() ) );
			rigid_body_rotamer_with_residue_alternatives.fast_forward_to_next_residue( moving_res_ );
		}

	}

} //screener
} //stepwise
} //protocols
