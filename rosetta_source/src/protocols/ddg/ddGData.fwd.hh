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

#ifndef INCLUDED_protocols_ddG_ddGData_fwd_hh
#define INCLUDED_protocols_ddG_ddGData_fwd_hh

#include <utility/pointer/owning_ptr.hh>

// AUTO-REMOVED #include <string>
// AUTO-REMOVED #include <map>

// Package headers

namespace protocols {
namespace ddG {

class PositionDdGInfo;
typedef utility::pointer::owning_ptr< PositionDdGInfo > PositionDdGInfoOP;
typedef utility::pointer::owning_ptr< PositionDdGInfo const >PositionDdGInfoCOP;

} // ddG
} // protocols

#endif
