// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file PoseInputStream.fwd.hh
/// @brief
/// @author James Thompson

#ifndef INCLUDED_core_import_pose_pose_stream_PoseInputStream_FWD_HH
#define INCLUDED_core_import_pose_pose_stream_PoseInputStream_FWD_HH

#include <utility/pointer/owning_ptr.fwd.hh>

namespace core {
namespace import_pose {
namespace pose_stream {

class PoseInputStream;
typedef utility::pointer::owning_ptr< PoseInputStream > PoseInputStreamOP;

} // pose_input_stream
} // impose_pose
} // core

#endif
