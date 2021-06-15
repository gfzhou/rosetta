// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/antibody/AntibodyInfo.fwd.hh
/// @brief  AntibodyInfo class forward declarations header


#ifndef INCLUDED_protocols_antibody_NativeAntibodySeq_fwd_hh
#define INCLUDED_protocols_antibody_NativeAntibodySeq_fwd_hh


// Utility headers
#include <utility/pointer/owning_ptr.hh>

// C++ Headers
namespace protocols {
namespace antibody {
namespace design {


// Forward
class NativeAntibodySeq;

typedef utility::pointer::shared_ptr< NativeAntibodySeq> NativeAntibodySeqOP;
typedef utility::pointer::shared_ptr< NativeAntibodySeq const> NativeAntibodySeqCOP;


}
} //namespace antibody
} //namespace protocols

#endif //INCLUDED_protocols_NativeAntibodySeq_FWD_HH
