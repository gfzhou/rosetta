// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file StepWiseResidueSampler.hh
/// @brief
/// @detailed
///
/// @author Rhiju Das
/// @author Parin Sripakdeevong


#ifndef INCLUDED_protocols_swa_StepWiseDoNothingSampleGenerator_HH
#define INCLUDED_protocols_swa_StepWiseDoNothingSampleGenerator_HH

#include <protocols/swa/StepWisePoseSampleGenerator.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>

//Auto Headers
#include <utility/vector1.hh>

namespace protocols {
namespace swa {

	class StepWiseDoNothingSampleGenerator: public StepWisePoseSampleGenerator {
	public:

    StepWiseDoNothingSampleGenerator();

		void reset();

		bool has_another_sample();

		void get_next_sample( core::pose::Pose & pose );

		Size size() const;

	private:

		Size ready_to_go_;

  };

} //swa
} // protocols

#endif

