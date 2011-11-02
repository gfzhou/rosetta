// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file
/// @brief
/// @author Frank DiMaio

#ifndef INCLUDED_protocols_rbsegment_moves_FragInsertAndAlignMover_hh
#define INCLUDED_protocols_rbsegment_moves_FragInsertAndAlignMover_hh

// AUTO-REMOVED #include <protocols/rbsegment_moves/RBSegmentMover.hh>

// Rosetta Headers
 // FIX THESE #INCLUDES! EVIL!
// AUTO-REMOVED #include <core/pose/Pose.hh>
// AUTO-REMOVED #include <core/conformation/Residue.hh>
#include <core/fragment/Frame.hh>
// AUTO-REMOVED #include <core/scoring/Ramachandran.hh>
// AUTO-REMOVED #include <core/kinematics/MoveMap.hh>
// AUTO-REMOVED #include <basic/basic.hh>

// Utility Headers
// AUTO-REMOVED #include <utility/vector1.hh>

#include <string>

#include <protocols/moves/Mover.hh>
#include <protocols/rbsegment_moves/RBSegment.fwd.hh>
#include <utility/vector1.hh>


namespace protocols {
namespace rbsegment_moves {

class FragInsertAndAlignMover;
typedef utility::pointer::owning_ptr< FragInsertAndAlignMover >  FragInsertAndAlignMoverOP;

//////////////////////////////////////////////////////////
///@brief Performs a rigid-body movement on a segment of a protein
///       Derived classes must implement 'getCoordinateTransform' and 'apply'
/////////////////////////////////////////////////////////
class FragInsertAndAlignMover : public protocols::moves::Mover {
public:
	/// @brief constructor
	FragInsertAndAlignMover();
	~FragInsertAndAlignMover();

	FragInsertAndAlignMover(
		utility::vector1< protocols::rbsegment_moves::RBSegment > const &rbsegs,
		core::pose::Pose const &ref_pose,
		core::Real randomness=0.0 );

	/// @brief clone this object
	virtual protocols::moves::MoverOP clone() const {
		return protocols::moves::MoverOP(new FragInsertAndAlignMover(*this));
	}

	/// @brief take a CA-only pose and insert idealized fragments close to the trace
	void bootstrapCATrace( core::pose::Pose & start_pose );

	/// @brief insert one RB fragment + realign
	void apply( core::pose::Pose & pose );

	virtual std::string get_name() const;

	/// @brief insert one RB fragment + realign
	void apply( core::pose::Pose & pose, int idx, bool idealize=false);

private:
	void initialize_rb_fragments(
		utility::vector1< protocols::rbsegment_moves::RBSegment > const &rbsegs,
		core::pose::Pose const &ref_pose,
		core::Real randomness=0.0 );

	utility::vector1< core::fragment::FrameOP > frames_;
};

}
}

#endif
