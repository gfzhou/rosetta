// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file RNA_DeleteMover.hh
/// @brief
/// @detailed
///
/// @author Rhiju Das


#ifndef INCLUDED_protocols_stepwise_monte_carlo_RNA_DeleteMover_hh
#define INCLUDED_protocols_stepwise_monte_carlo_RNA_DeleteMover_hh

#include <core/pose/Pose.fwd.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/types.hh>
#include <protocols/moves/Mover.hh>
#include <protocols/stepwise/monte_carlo/SWA_Move.hh>
#include <protocols/stepwise/monte_carlo/rna/RNA_DeleteMover.fwd.hh>
#include <protocols/stepwise/sampling/rna/StepWiseRNA_Modeler.fwd.hh>
#include <protocols/stepwise/monte_carlo/rna/StepWiseRNA_MonteCarloOptions.fwd.hh>

namespace protocols {
namespace stepwise {
namespace monte_carlo {
namespace rna {

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
class RNA_DeleteMover: public protocols::moves::Mover {
public:

	//constructor
	RNA_DeleteMover();

	//destructor
	~RNA_DeleteMover();

	using protocols::moves::Mover::apply;

  void
	apply( core::pose::Pose & pose, Size const res_to_delete_in_full_model_numbering ) const;

  void
  apply( core::pose::Pose & pose, utility::vector1< Size > const & residues_to_delete_in_full_model_numbering ) const;

	/// @brief Apply the minimizer to one pose
	virtual void apply( core::pose::Pose & pose_to_visualize );
	virtual std::string get_name() const;

	bool
  decide_to_keep_pose( pose::Pose const & pose ) const;

  void
  remove_singletons_and_update_pose_focus( core::pose::Pose & pose,
																					 core::pose::PoseOP sliced_out_pose_op,
																					 bool & keep_remainder_pose,
																					 bool & keep_sliced_out_pose ) const;

	void
	wipe_out_moving_residues( core::pose::Pose & pose );

	void minimize_after_delete( core::pose::Pose & pose ) const;
	void set_minimize_after_delete( bool const setting ){ minimize_after_delete_ = setting; }

	void set_stepwise_rna_modeler( protocols::stepwise::sampling::rna::StepWiseRNA_ModelerOP stepwise_rna_modeler );

	void
	set_options( StepWiseRNA_MonteCarloOptionsCOP options );

private:

	protocols::stepwise::sampling::rna::StepWiseRNA_ModelerOP stepwise_rna_modeler_;
	StepWiseRNA_MonteCarloOptionsCOP options_;
	bool minimize_after_delete_;

};

} //rna
} //monte_carlo
} //stepwise
} //protocols

#endif
