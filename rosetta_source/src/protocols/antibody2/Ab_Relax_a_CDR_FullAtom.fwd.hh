// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet;
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   Ab_LHSnugFit_Mover.fwd.hh
///
/// @brief forward declaration
/// @author Jianqing Xu (xubest@gmail.com)

#ifndef INCLUDED_protocols_antibody2_Ab_Relax_a_CDR_FullAtom_fwd_hh
#define INCLUDED_protocols_antibody2_Ab_Relax_a_CDR_FullAtom_fwd_hh

#include <utility/pointer/owning_ptr.hh>

namespace protocols {
namespace antibody2 {
        
    class Ab_Relax_a_CDR_FullAtom;
    typedef utility::pointer::owning_ptr< Ab_Relax_a_CDR_FullAtom > Ab_Relax_a_CDR_FullAtomOP;
    typedef utility::pointer::owning_ptr< Ab_Relax_a_CDR_FullAtom const > Ab_Relax_a_CDR_FullAtomCOP;
        
}
}

#endif




