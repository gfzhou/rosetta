// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/stepwise/monte_carlo/rna/RNA_FromScratchMover.hh
/// @brief
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu


#ifndef INCLUDED_protocols_stepwise_monte_carlo_rna_RNA_FromScratchMover_HH
#define INCLUDED_protocols_stepwise_monte_carlo_rna_RNA_FromScratchMover_HH

#include <core/types.hh>
#include <protocols/moves/Mover.hh>
#include <core/pose/Pose.fwd.hh>
#include <protocols/stepwise/monte_carlo/rna/RNA_FromScratchMover.fwd.hh>
#include <protocols/stepwise/sampling/rna/StepWiseRNA_Modeler.fwd.hh>

using namespace core;

namespace protocols {
namespace stepwise {
namespace monte_carlo {
namespace rna {

	class RNA_FromScratchMover: public protocols::moves::Mover {

	public:

		//constructor
		RNA_FromScratchMover();

		//destructor
		~RNA_FromScratchMover();

	public:

		void
		apply( core::pose::Pose & pose,
					 utility::vector1<Size> const & residues_to_instantiate_in_full_model_numbering ) const;

		/// @brief Apply the minimizer to one pose
		virtual void apply( core::pose::Pose & pose_to_visualize );
		virtual std::string get_name() const;

		void set_stepwise_rna_modeler( protocols::stepwise::sampling::rna::StepWiseRNA_ModelerOP stepwise_rna_modeler );

	private:

		void
		update_full_model_info_and_switch_focus_to_new_pose( pose::Pose & pose, pose::Pose & new_pose, utility::vector1< Size > const & resnum ) const;

		void
		sample_by_swa( pose::Pose & pose, Size const sample_res ) const;

	private:

		protocols::stepwise::sampling::rna::StepWiseRNA_ModelerOP stepwise_rna_modeler_;

	};

} //rna
} //monte_carlo
} //stepwise
} //protocols

#endif
