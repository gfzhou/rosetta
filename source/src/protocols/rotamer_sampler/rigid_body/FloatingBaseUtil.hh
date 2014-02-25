// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/rotamer_sampler/rigid_body/FloatingBaseUtil.hh
/// @brief
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu


#ifndef INCLUDED_protocols_rotamer_sampler_rigid_body_FloatingBaseUtil_HH
#define INCLUDED_protocols_rotamer_sampler_rigid_body_FloatingBaseUtil_HH

#include <core/id/AtomID.fwd.hh>
#include <core/conformation/Residue.fwd.hh>
#include <core/kinematics/Stub.fwd.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>
#include <string>

using namespace core;
using namespace core::pose;

namespace protocols {
namespace rotamer_sampler {
namespace rigid_body {

	void
	get_atom_coordinates( utility::vector1< std::pair < id::AtomID, numeric::xyzVector< Real > > > & xyz_list,
												Size const & seq_num,
												conformation::Residue const & rsd_at_origin,
												kinematics::Stub const & moving_res_base_stub );

	void
	get_specific_atom_coordinate( Vector & atom_pos,
																kinematics::Stub const & moving_res_base_stub );

	void
	get_specific_atom_coordinate( Size const atom_index,
																Vector & atom_pos,
																conformation::Residue const & rsd_at_origin,
																kinematics::Stub const & moving_res_base_stub );


	void
	get_specific_atom_coordinate( std::string const & atom_name,
															  Vector & atom_pos,
																conformation::Residue const & rsd_at_origin,
																kinematics::Stub const & moving_res_base_stub );


	/////////////////////////////////////////////////////////////
	// following functions are helpers for RigidBodyRotamer
	Size
	figure_out_reference_res_for_jump( pose::Pose const & pose,
																		 Size const moving_res );

	utility::vector1< Size >
	figure_out_moving_partition_res( pose::Pose const & pose,
																	 Size const moving_res, Size const reference_res );

	void
	set_to_origin( pose::Pose & pose, Size const seq_num, bool verbose );

	kinematics::Stub
	initialize_stub( conformation::Residue const & rsd );

	PoseCOP
	transform_moving_partition_to_origin( pose::Pose const & pose_start,
																				Size const moving_res,
																				utility::vector1< Size > const & moving_partition_res_ );

} //rigid_body
} //rotamer_sampler
} //protocols

#endif
