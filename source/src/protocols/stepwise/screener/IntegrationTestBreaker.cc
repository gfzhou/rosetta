// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/stepwise/screener/IntegrationTestBreaker.cc
/// @brief
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu

#include <protocols/stepwise/screener/IntegrationTestBreaker.hh>
#include <protocols/stepwise/screener/NativeRMSD_Screener.hh>
#include <protocols/rotamer_sampler/rigid_body/RigidBodyRotamerWithResidueList.hh>
#include <protocols/rotamer_sampler/rigid_body/RigidBodyRotamerWithResidueAlternatives.hh>
#include <basic/Tracer.hh>

static basic::Tracer TR( "protocols.stepwise.screener.IntegrationTestBreaker" );

using namespace protocols::rotamer_sampler;
using namespace protocols::rotamer_sampler::rigid_body;

namespace protocols {
namespace stepwise {
namespace screener {

	//Constructor
	IntegrationTestBreaker::IntegrationTestBreaker( StepWiseScreenerOP screener_whose_counts_to_check,
																									StepWiseScreenerOP final_screener /*total_count -- for turning on native screen*/,
																									NativeRMSD_ScreenerOP native_rmsd_screener ):
		screener_whose_counts_to_check_( screener_whose_counts_to_check ),
		final_screener_( final_screener ),
		native_rmsd_screener_( native_rmsd_screener )
	{}

	//Destructor
	IntegrationTestBreaker::~IntegrationTestBreaker()
	{}

	//////////////////////////////////////////////////////////////////////////
	bool
	IntegrationTestBreaker::check_screen() {

		if ( screener_whose_counts_to_check_->count() >= 100 ) return false;
		if ( final_screener_->count() >= 10 ) native_rmsd_screener_->set_do_screen( true );
		if ( native_rmsd_screener_->pass_count() >= 10 ) return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	void
	IntegrationTestBreaker::fast_forward( rotamer_sampler::RotamerBaseOP sampler ){
		if ( sampler->type() == RIGID_BODY_WITH_RESIDUE_LIST ){
			RigidBodyRotamerWithResidueList & rigid_body_rotamer_with_copy_dofs = *( static_cast< RigidBodyRotamerWithResidueList * >( sampler.get() ) );
			rigid_body_rotamer_with_copy_dofs.fast_forward();
		} else if ( sampler->type() == RIGID_BODY_WITH_RESIDUE_ALTERNATIVES ){
			RigidBodyRotamerWithResidueAlternatives & rigid_body_rotamer_with_residue_alternatives = *( static_cast< RigidBodyRotamerWithResidueAlternatives * >( sampler.get() ) );
			rigid_body_rotamer_with_residue_alternatives.fast_forward();
		}
	}

} //screener
} //stepwise
} //protocols
