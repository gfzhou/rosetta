// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
//  vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.
/// @author Rhiju Das (rhiju@stanford.edu, Parin Sripakdeevong (sripakap@stanford.edu)

#ifndef INCLUDED_protocols_swa_rna_StepWiseRNA_RotamerGenerator_fwd_hh
#define INCLUDED_protocols_swa_rna_StepWiseRNA_RotamerGenerator_fwd_hh

#include <utility/pointer/owning_ptr.hh>

namespace protocols{
namespace swa{
namespace rna{

	class StepWiseRNA_RotamerGenerator;
	typedef utility::pointer::owning_ptr< StepWiseRNA_RotamerGenerator > StepWiseRNA_RotamerGeneratorOP;
	typedef utility::pointer::owning_ptr< StepWiseRNA_RotamerGenerator const > StepWiseRNA_RotamerGeneratorCOP;

}
}
}

#endif
