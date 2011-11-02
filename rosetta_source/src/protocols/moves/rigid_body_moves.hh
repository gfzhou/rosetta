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
/// @author


#ifndef INCLUDED_protocols_moves_rigid_body_moves_hh
#define INCLUDED_protocols_moves_rigid_body_moves_hh


// Package headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/kinematics/MoveMap.fwd.hh>

#include <utility/vector1.hh>



// ObjexxFCL Headers
// #include <ObjexxFCL/FArray3A.hh>
// #include <ObjexxFCL/FArray4D.hh>
// #include "triplet/triplet.h"

// // C++ Headers
// #include <cassert>
// #include <cmath>
// #include <cstdlib>
// #include <iostream>
// #include <map>
// #include <string>


// //Utility Headers
// #include "utility/basic_sys_util.h"

// Forward declarations

namespace protocols {
namespace moves {

int
gaussian_jump_move(
	core::pose::Pose & pose,
	core::kinematics::MoveMap const & mm,
	core::Real const translation_magnitude,
	core::Real const rotation_magnitude,
	int const dir = 0 // --> choose randomly
);


void
gaussian_jump_move(
	core::pose::Pose & pose,
	int const jump_number,
	core::Real const translation_magnitude,
	core::Real const rotation_magnitude,
	int dir = 0 // --> choose randomly
);


} // moves
} // rosetta

#endif
