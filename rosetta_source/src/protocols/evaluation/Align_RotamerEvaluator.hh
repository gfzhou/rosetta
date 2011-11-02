// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file Align_RotamerEvaluator
/// @author James Thompson

#ifndef INCLUDED_protocols_evaluation_Align_RotamerEvaluator_hh
#define INCLUDED_protocols_evaluation_Align_RotamerEvaluator_hh

#include <protocols/evaluation/AlignEvaluator.hh>
// AUTO-REMOVED #include <protocols/evaluation/util.hh>

#include <core/sequence/SequenceAlignment.fwd.hh>
#include <core/io/silent/silent.fwd.hh>
#include <core/pose/Pose.fwd.hh>

#include <core/types.hh>

#include <utility/vector1.hh>


namespace protocols {
namespace evaluation {

class Align_RotamerEvaluator : public AlignEvaluator {

public:
	Align_RotamerEvaluator(
		core::pose::PoseCOP native_pose,
		std::string tag = "rot_",
		core::Real const chi_dev = 30,
		core::sequence::SequenceAlignmentOP aln = 0
	);

	~Align_RotamerEvaluator();

	virtual void apply(
		core::pose::Pose & pose,
		std::string tag,
		core::io::silent::SilentStruct & ss
	) const;

	virtual core::Real apply(
		core::pose::Pose & /*pose*/
	) const {
		return 0;
	}

	core::Real chi_dev() const;

private:
	core::pose::PoseCOP native_pose_;
	std::string tag_;
	core::Real chi_dev_;
	core::sequence::SequenceAlignmentOP aln_;
}; // Align_RotamerEvaluator

} // evaluation
} // protocols

#endif
