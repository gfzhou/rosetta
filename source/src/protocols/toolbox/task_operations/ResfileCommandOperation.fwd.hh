// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/toolbox/task_operations/ResfileCommandOperation.fwd.hh
/// @brief Applies the equivalent of a resfile line (without the resnums) to residues specified in a residue selector.
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

#ifndef INCLUDED_protocols_toolbox_task_operations_ResfileCommandOperation_fwd_hh
#define INCLUDED_protocols_toolbox_task_operations_ResfileCommandOperation_fwd_hh

// Utility headers
#include <utility/pointer/owning_ptr.hh>


// Forward
namespace protocols {
namespace toolbox {
namespace task_operations {

class ResfileCommandOperation;

typedef utility::pointer::shared_ptr< ResfileCommandOperation > ResfileCommandOperationOP;
typedef utility::pointer::shared_ptr< ResfileCommandOperation const > ResfileCommandOperationCOP;

} //protocols
} //toolbox
} //task_operations

#endif //INCLUDED_protocols_toolbox_task_operations_ResfileCommandOperation_fwd_hh
