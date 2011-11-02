// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file TopologyBroker
/// @brief  top-class (Organizer) of the TopologyBroker mechanism
/// @detailed responsibilities:
/// @author Oliver Lange


#ifndef INCLUDED_protocols_topology_broker_ConstraintClaimer_hh
#define INCLUDED_protocols_topology_broker_ConstraintClaimer_hh


// Unit Headers
#include <protocols/topology_broker/ConstraintClaimer.fwd.hh>

// Package Headers
#include <protocols/topology_broker/TopologyClaimer.hh>
#include <protocols/topology_broker/DofClaim.fwd.hh>
#include <core/scoring/constraints/ConstraintSet.hh>

// Project Headers
#include <core/pose/Pose.hh>

// ObjexxFCL Headers

// Utility headers
//#include <utility/io/izstream.hh>
//#include <utility/io/ozstream.hh>
//#include <utility/io/util.hh>
//#include <basic/Tracer.hh>
//#include <basic/options/option.hh>

#include <utility/pointer/ReferenceCount.hh>

#include <utility/vector1.hh>


//#include <basic/options/option_macros.hh>

//// C++ headers
//#include <fstream>

// option key includes


namespace protocols {
namespace topology_broker {

class ConstraintClaimer : public TopologyClaimer {
	typedef TopologyClaimer Parent;
public:
	ConstraintClaimer(); //for factory
	ConstraintClaimer( std::string cst_file, std::string tag = "" );
	ConstraintClaimer( bool CmdFlag, bool centroid = true, bool fullatom = false );

	virtual TopologyClaimerOP clone() const {
		return new ConstraintClaimer( *this );
	}

	virtual void generate_claims( DofClaims& );

	///@brief type() is specifying the output name of the TopologyClaimer
	virtual std::string type() const {
		return _static_type_name();
	}

	static std::string _static_type_name() {
		return "ConstraintClaimer";
	}

	virtual void new_decoy();

	virtual bool read_tag( std::string tag, std::istream & );

	virtual void add_constraints( core::pose::Pose& /*pose*/ );


private:
	std::string filename_, fa_filename_;
	std::string cst_filename_;
	std::string tag_;
	mutable core::scoring::constraints::ConstraintSetOP constraints_;
	mutable core::scoring::constraints::ConstraintSetOP fa_constraints_;

	mutable core::pose::Pose constraint_ref_pose_;
	mutable std::string sequence_;

	///@brief true if constraints are active in centroid mode
	bool bCentroid_;

	///@brief true if constraints are active in full-atom mode
	/// if true use fa_constraints if available... normal constraints otherwise
	bool bFullatom_;

	///@brief use constraints defined via command line
	bool bCmdFlag_;


	///@brief combine constraints randomly into Ambiguous Constraints
	core::Size combine_ratio_;  //default 1: no constraint combination
	utility::vector1< bool > combine_exclude_res_;

	///@brief at most one constraint per residue pair ( does not look at bounds... )
	bool skip_redundant_;

	///@brief how many residues left and right do we exclude other "redundant" constraints...
	core::Size skip_redundant_width_;
}; //class ConstraintClaimer

}
}

#endif
