// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/moves/MergePDBatOverlapMoverCreator.hh
/// @brief This class combines two pdbs with a known overlap
/// @author TJ Brunette (tjbrunette@gmail.com)

#ifndef INCLUDED_protocols_pose_creation_MergePDBatOverlapMover_fwd_hh
#define INCLUDED_protocols_pose_creation_MergePDBatOverlapMover_fwd_hh

#include <utility/pointer/owning_ptr.hh>

namespace protocols {
namespace pose_creation {

class MergePDBatOverlapMover;

typedef utility::pointer::shared_ptr< MergePDBatOverlapMover > MergePDBatOverlapMoverOP;
typedef utility::pointer::shared_ptr< MergePDBatOverlapMover const > MergePDBatOverlapMoverCOP;

} // pose_creation
} // protocols

#endif // INCLUDED_protocols_pose_creation_MergePDBatOverlapMoverCreator_fwd_hh

