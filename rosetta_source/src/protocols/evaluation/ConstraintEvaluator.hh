// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file ClashEvaluator.hh
/// @brief
/// @detailed
///
///
///
/// @author Oliver Lange



#ifndef INCLUDED_protocols_evaluation_ConstraintEvaluator_hh
#define INCLUDED_protocols_evaluation_ConstraintEvaluator_hh

#include <protocols/evaluation/ConstraintEvaluator.fwd.hh>


// Unit Headers

// Package Headers
#include <protocols/evaluation/PoseEvaluator.hh>

// Project Headers
#include <core/pose/Pose.fwd.hh>
// AUTO-REMOVED #include <core/scoring/constraints/Constraint.hh>

#ifdef WIN32	// for visual studio
#else
#include <core/scoring/constraints/Constraint.fwd.hh>
#endif
// AUTO-REMOVED #include <core/scoring/constraints/ConstraintSet.hh>

#include <core/io/silent/silent.fwd.hh>


// ObjexxFCL Headers

// Utility headers
#include <utility/pointer/ReferenceCount.hh>

#include <core/scoring/constraints/ConstraintSet.fwd.hh>
#include <utility/vector1.hh>


// AUTO-REMOVED #include <utility/vector1.hh>

//// C++ headers
// AUTO-REMOVED #include <list>

namespace protocols {
namespace evaluation {

class ConstraintEvaluator : public evaluation::PoseEvaluator {
public:
  ConstraintEvaluator( std::string tag, core::scoring::constraints::ConstraintSet const&, Size viol_level = 0, core::Real threshold = 1.0, Size max_seq_sep = 0 );
  ConstraintEvaluator( std::string tag, core::scoring::constraints::ConstraintCOPs const&, Size viol_level = 0, core::Real threshold = 1.0, Size max_seq_sep = 0 );

	//constructor for a late input of constraints --- in case pose is not known at construction
	// ( will read only once with first pose that comes )
	ConstraintEvaluator( std::string tag, std::string filename, Size viol_level = 0, core::Real threshold = 1.0, Size max_seq_sep = 0 );

	//sets xxx_cst and xxx_viol columns
  virtual void apply( core::pose::Pose& pose, std::string tag, core::io::silent::SilentStruct &pss) const;

	//returns constraint score
	virtual core::Real apply( core::pose::Pose& pose ) const;

	virtual core::Size size() const { return 1; }
	virtual std::string name( core::Size i ) const;

	void set_combine_ratio( core::Size setting ) {
		constraints_combine_ratio_ = setting;
	}

private:
	//add constraint set to pose --- ready for scoring
	void prepare_pose( core::pose::Pose const& pose_in, core::pose::Pose& pose ) const;

	std::string name_;
	mutable core::scoring::constraints::ConstraintSetOP constraints_; //mutable for late reading if file-name is provided
	mutable core::scoring::constraints::ConstraintSetOP fa_constraints_; //mutable for late reading if file-name is provided
	mutable bool tried_fullatom_pose_;
	mutable bool tried_centroid_pose_;
	std::string file_name_;
	Size viol_level_;
	core::Real threshold_;
	Size max_seq_sep_;
	Size constraints_combine_ratio_;
};


}
}

#endif
