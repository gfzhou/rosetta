// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file DockinLowRes
/// @brief low resolution mode for docking
/// @detailed
/// @author Monica Berrondo
/// @author Modified by Sergey Lyskov


#ifndef INCLUDED_protocols_docking_DockingLowRes_hh
#define INCLUDED_protocols_docking_DockingLowRes_hh

#include <protocols/docking/types.hh>
#include <protocols/docking/DockingLowRes.fwd.hh>
#include <protocols/docking/DockingEnsemble.fwd.hh>

// Package headers
#include <core/kinematics/MoveMap.fwd.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/scoring/ScoreFunction.fwd.hh>

#include <protocols/moves/ConformerSwitchMover.fwd.hh>
#include <protocols/moves/MoverContainer.fwd.hh>
#include <protocols/moves/MonteCarlo.fwd.hh>
#include <protocols/moves/Mover.hh>
#include <protocols/moves/RigidBodyMover.fwd.hh>
// AUTO-REMOVED #include <protocols/filters/Filter.hh>
// AUTO-REMOVED #include <protocols/filters/ScoreCutoffFilter.hh>

#include <string>



// option key includes

// AUTO-REMOVED #include <basic/options/keys/docking.OptionKeys.gen.hh>

#include <utility/vector1.hh>
#include <iostream>



namespace protocols {
namespace docking {

class DockingLowRes : public moves::Mover
{
	//typedef core::Real Real;
public:

	/// @brief Default constructor
	DockingLowRes();

	// destructor
	~DockingLowRes();

	/// @brief Constructor with two arguments.  The first is scorefunction to be used for docking, the second is the
	///		jump to dock over.
	DockingLowRes(
		core::scoring::ScoreFunctionCOP scorefxn,
		core::Size const rb_jump=1
	);

	/// @brief Constructor with two arguments.  The first is scorefunction to be used for docking, the second is the
	///		DockJumps.
	DockingLowRes(
		core::scoring::ScoreFunctionCOP scorefxn,
		DockJumps const movable_jumps
	);

	protocols::moves::MoverOP clone() const;

	/// @brief Assigns default values to primitive members
	void set_default();
	void set_scorefxn( core::scoring::ScoreFunctionCOP scorefxn );

	/// @brief Instantiates non-primitive members based on the value of the primitive members
	void sync_objects_with_flags();

	moves::MonteCarloOP get_mc();

	// protocol functions
	virtual void apply( core::pose::Pose & pose );
	virtual std::string get_name() const;

	/// @brief Perform a cycle of rigid-body Monte Carlo moves
	void rigid_body_trial( core::pose::Pose & pose );

	// option setters
	void set_inner_cycles( core::Size inner_cycles ) { inner_cycles_=inner_cycles; }
	void set_outer_cycles( core::Size outer_cycles ) { outer_cycles_=outer_cycles; }
	void set_ensemble1( DockingEnsembleOP ensemble1 );
	void set_ensemble2( DockingEnsembleOP ensemble2 );

	void show( std::ostream & out=std::cout );
	friend std::ostream & operator<<(std::ostream& out, const DockingLowRes & dp );

private:
	// protocol stuff
	core::scoring::ScoreFunctionCOP scorefxn_;
	core::kinematics::MoveMapOP movemap_;
	protocols::moves::MonteCarloOP mc_;
	protocols::moves::SequenceMoverOP docking_lowres_protocol_;
	protocols::moves::RigidBodyPerturbNoCenterMoverOP rb_mover_;
	protocols::moves::ConformerSwitchMoverOP ensemble1_mover_;
	protocols::moves::ConformerSwitchMoverOP ensemble2_mover_;

	bool flags_and_objects_are_in_sync_;
	bool first_apply_with_current_setup_;

	// docking
	core::Size inner_cycles_, outer_cycles_; //rb_jump_
	DockJumps movable_jumps_;
	core::Real trans_magnitude_, rot_magnitude_, accept_rate_;
	bool chi_, bb_, nb_list_;

	core::Real temperature_;

	/// @brief Sets up the instance of DockingLowRes and initializes all members based on values passed in at construction
	///		or via the command line.
	void init(
		DockJumps const movable_jumps,
		core::scoring::ScoreFunctionCOP scorefxn
	);

	/// @brief Performs the portion of setup of non-primitive members that requires a pose - called on apply
	void finalize_setup( core::pose::Pose & pose);
};

} // docking
} // protocols

#endif
