// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file /protocols/moves/RollMover.hh
/// @brief
/// @author
#ifndef INCLUDED_protocols_moves_RollMover_hh
#define INCLUDED_protocols_moves_RollMover_hh
// Unit Headers
#include <protocols/moves/RollMover.fwd.hh>
// Project Headers
#include <core/pose/Pose.fwd.hh>
#include <protocols/moves/Mover.hh>
// AUTO-REMOVED #include <numeric/xyz.io.hh>
// AUTO-REMOVED #include <numeric/xyz.functions.hh>

// Utility Headers
#include <core/types.hh>
// AUTO-REMOVED #include <utility/tag/Tag.hh>

#include <utility/vector0.hh>
#include <utility/vector1.hh>
#include <numeric/xyzVector.hh>

namespace protocols {
namespace moves {

///@details
class RollMover : public protocols::moves::Mover {

public:

	///@brief
	RollMover(
	);

	RollMover( core::Size start_res, core::Size stop_res, core::Real min_angle, core::Real max_angle, numeric::xyzVector< core::Real > axis, numeric::xyzVector< core::Real > translate );

	virtual ~RollMover();

	virtual void apply( core::pose::Pose & pose );
	void set_min_max_angles( core::Real min_angle, core::Real max_angle );
	virtual std::string get_name() const;

	///@brief required in the context of the parser/scripting scheme
	virtual MoverOP fresh_instance() const;

	///@brief required in the context of the parser/scripting scheme
	virtual MoverOP clone() const;

	virtual 
	void 
	parse_my_tag(
		TagPtr const /*tag*/,
		DataMap & /*data*/,
		Filters_map const & /*filters*/,
		Movers_map const & /*movers*/,
		Pose const & /*pose*/);



private:

	core::Size start_res_;
	core::Size stop_res_;
	core::Real angle_;
	core::Real min_angle_;
	core::Real max_angle_;
	numeric::xyzVector< core::Real > axis_;
	numeric::xyzVector< core::Real > translate_;
};//end RollMover


}//namespace protocols
}//namespace moves

#endif // INCLUDED_protocols_moves_RollMover_HH
