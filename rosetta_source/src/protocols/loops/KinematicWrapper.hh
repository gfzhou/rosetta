// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file /protocols/loops/KinematicWrapper.hh
/// @brief wrapper for KinematicMover - useful when only apply() is available
/// @author Steven Lewis

#ifndef INCLUDED_protocols_loops_KinematicWrapper_hh
#define INCLUDED_protocols_loops_KinematicWrapper_hh

// Unit Headers
#include <protocols/loops/KinematicWrapper.fwd.hh>
#include <protocols/moves/KinematicMover.fwd.hh>

// Project Headers
#include <core/pose/Pose.fwd.hh>

#include <core/kinematics/MoveMap.fwd.hh>

#include <protocols/moves/Mover.hh>

#include <protocols/loops/Loops.fwd.hh>

// Utility Headers
#include <core/types.hh>
// AUTO-REMOVED #include <utility/vector1.hh>

#include <utility/vector1.hh>


namespace protocols {
namespace loops {

///@details This class extends KinematicMover by composition (has-a KinematicMover).  The idea is to make KinematicMover useful when ONLY the apply() function is available (as in MoverContainers, etc.).  It will reapply KinematicMover until the KinematicMover reports a success.  This class uses searches through loop begin/middle/end points while searching for solutions.  You can pass a MoveMap; it will refuse to put pivots where the MoveMap is not mobile.
class KinematicWrapper : public protocols::moves::Mover {

public:
	///@brief ctor with Loop
	KinematicWrapper(
		protocols::moves::KinematicMoverOP kinmover_in,
		protocols::loops::Loop loop,
		core::Size cycles = 0 //0 is sentinel value to check options system
	);

	///@brief ctor with explicit loop begin/end
	KinematicWrapper(
		protocols::moves::KinematicMoverOP kinmover_in,
		core::Size loop_begin,
		core::Size loop_end,
		core::Size cycles = 0 //0 is sentinel value to check options system
	);

	virtual ~KinematicWrapper();

	///@brief re-applies KinematicMover with different pivots until success
	virtual void apply( core::pose::Pose & pose );
	virtual std::string get_name() const;

	///@brief this function derives the allowed_positions_ vector from mm and the loop begin/end
	void respect_this_movemap( core::kinematics::MoveMapCOP mm );

private:

	///@brief two ctors have same functions, just different initialization lists; de-duplicated here
	void ctor();

	///@brief initializes allowed positions - for constructors and resetting
	void init_allowed_pos();

	///@brief OP for KinematicMover
	protocols::moves::KinematicMoverOP kinmover_;

	///@brief the loop this object-instance remodels
	core::Size const loop_begin_;
	///@brief the loop this object-instance remodels
	core::Size const loop_end_;

	///@brief a limit on the number of cycles the mover will run
	core::Size const limit_;

	///@brief vector of positions allowed as pivots
	utility::vector1<core::Size> allowed_positions_;
};//end KinematicWrapper

}//namespace loops
}//namespace protocols

#endif // INCLUDED_protocols_loops_KinematicWrapper_HH
