// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// (C) 199x-2009 Rosetta Commons participating institutions and developers.
// For more information, see http://www.rosettacommons.org/.

/// @file TopologyBroker
/// @brief  top-class (Organizer) of the TopologyBroker mechanism
/// @detailed responsibilities:
/// @author Oliver Lange


#ifndef INCLUDED_protocols_topology_broker_FibrilModelingClaimer_hh
#define INCLUDED_protocols_topology_broker_FibrilModelingClaimer_hh


// Unit Headers
#include <protocols/topology_broker/FibrilModelingClaimer.fwd.hh>

// Package Headers
#include <protocols/topology_broker/DofClaim.fwd.hh>
#include <protocols/topology_broker/TopologyClaimer.hh>

// Project Headers
#include <core/pose/Pose.hh>
#include <core/conformation/symmetry/SymmetryInfo.hh>
#include <protocols/loops/Loops.hh>

#include <utility/pointer/ReferenceCount.hh>

#include <utility/vector1.hh>


// AUTO-REMOVED #include <utility/vector1.hh>

// option key includes


namespace protocols {
namespace topology_broker {

class FibrilModelingClaimer : public TopologyClaimer {
	typedef TopologyClaimer Parent;

public:

	//c'stor
	FibrilModelingClaimer();
	FibrilModelingClaimer( core::pose::Pose const& input_pose, loops::Loops rigid, int shift = 0 );
	FibrilModelingClaimer( core::pose::Pose const& input_pose, loops::Loops rigid, loops::Loops input_rigid  );

	void
  make_fibril( core::pose::Pose & pose );

	//clone
	virtual TopologyClaimerOP clone() const {
		return new FibrilModelingClaimer( *this );
	}

	///@brief type() is specifying the output name of the TopologyClaimer
	virtual std::string type() const {
		return _static_type_name();
	}

	static std::string _static_type_name() {
		return "FibrilModelingClaimer";
	}

	virtual void add_mover(
    moves::RandomMover& random_mover,
		core::pose::Pose const& pose,
		abinitio::StageID stageID,
		core::scoring::ScoreFunction const& scorefxn,
		core::Real progress
	);

	virtual void initialize_dofs( core::pose::Pose&,
																DofClaims const& init_claims,
																DofClaims& /*failed_to_init*/ );

  virtual void generate_claims( DofClaims& new_claims );

	///@brief has to decline foreign BB claims for slave regions
	virtual bool allow_claim( DofClaim const& /*foreign_claim*/ );

protected:

	virtual bool read_tag( std::string tag, std::istream& is );

private:

	///@brief monomer pose
  core::pose::Pose input_pose_;

	///@brief regions that can be used for rigid core
	loops::Loops rigid_core_, input_rigid_core_;

	///@brief align to the monomer before create symmetry
	bool bAlign_;
	int sequence_shift_;

	///@brief symmetry information
	core::conformation::symmetry::SymmetryInfoOP symminfo_;

};

}
}

#endif
