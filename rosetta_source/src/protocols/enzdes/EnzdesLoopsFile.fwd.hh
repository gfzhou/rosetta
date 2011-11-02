// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/enzdes/EnzdesLoopsFile.fwd.hh
///
/// @brief
/// @author Florian Richter, floric@u.washington.edu, april 2009




#ifndef INCLUDED_protocols_enzdes_EnzdesLoopsFile_fwd_hh
#define INCLUDED_protocols_enzdes_EnzdesLoopsFile_fwd_hh


// AUTO-REMOVED #include <utility/pointer/ReferenceCount.hh>

#include <utility/pointer/owning_ptr.hh>


namespace protocols{
namespace enzdes{

class EnzdesLoopsFile;
typedef utility::pointer::owning_ptr< EnzdesLoopsFile > EnzdesLoopsFileOP;
typedef utility::pointer::owning_ptr< EnzdesLoopsFile const > EnzdesLoopsFileCOP;

class EnzdesLoopInfo;
typedef utility::pointer::owning_ptr< EnzdesLoopInfo > EnzdesLoopInfoOP;
typedef utility::pointer::owning_ptr< EnzdesLoopInfo const > EnzdesLoopInfoCOP;

} //namespace enzdes
} //namespace protocols
#endif //
