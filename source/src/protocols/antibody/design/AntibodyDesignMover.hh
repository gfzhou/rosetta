// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/antibody/design/AntibodyDesignMover.hh
/// @brief Handles the Antibody Design Protocol.
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

#ifndef INCLUDED_protocols_antibody_design_AntibodyDesignMover_hh
#define INCLUDED_protocols_antibody_design_AntibodyDesignMover_hh

#include <protocols/antibody/design/AntibodyDesignMover.fwd.hh>
#include <protocols/antibody/design/AntibodyDesignModeler.hh>
#include <protocols/antibody/database/AntibodyDatabaseManager.hh>
#include <protocols/antibody/design/AntibodyGraftDesignMover.hh>
#include <protocols/antibody/design/AntibodySeqDesignMover.hh>

#include <protocols/antibody/database/CDRSetOptions.hh>
#include <protocols/antibody/design/CDRGraftDesignOptions.hh>
#include <protocols/antibody/design/CDRSeqDesignOptions.hh>

#include <protocols/antibody/design/util.hh>

#include <protocols/antibody/AntibodyInfo.hh>

#include <protocols/moves/Mover.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/ScoreFunction.hh>

#include <utility/vector1.hh>
namespace protocols {
namespace antibody {
namespace design {

using namespace protocols::antibody;
using namespace protocols::antibody::clusters;

using std::string;
using utility::vector1;


///@brief Main AntibodyDesignMover, tieing together individual movers and classes.  Main mover for application.
///
class AntibodyDesignMover : public protocols::moves::Mover {
	
public:

	AntibodyDesignMover();

	virtual ~AntibodyDesignMover();
	
	virtual string 
	get_name() const;
	
	virtual protocols::moves::MoverOP clone() const;

	
	////////////////////////////////////////////////////////////////////////////
	// Optional Custom Settings
	//
	//
	
	void
	set_scorefxn(ScoreFunctionOP scorefxn);
	
	void
	set_design_scorefxn(ScoreFunctionOP design_scorefxn);
	
	void
	set_ab_info(AntibodyInfoOP ab_info);

	
	////////////////////////////////////////////////////////////////////////////
	// Algorithm Settings
	//
	//
	
	///@brief Use low-resolution graft designer for structural sampling.  Default true.
	void
	set_use_graft_designer(bool setting);
	
	///@brief Use high-resolution sequence designer for sequence sampling. Default true.
	void
	set_use_sequence_designer(bool setting);
	
	///@brief Run dock/min modeling step after the graft design step if run.
	void
	set_do_post_graft_design_modeling(bool setting);
	
	///@brief Run dock/min modeling step after sequence design if run.
	void
	set_do_post_design_modeling(bool setting);
	
	protocols::moves::MoverOP
	fresh_instance() const;

	////////////////////////////////////////////////////////////////////////////
	// Main Method.  All options read from cmd-line or set through individual classes.
	//
	//
	
	virtual void 
	apply( core::pose::Pose & pose );
	
private:
	
	void
	read_cmd_line_options();
	
	///@brief Sets command line driven CDR design.   This is mainly for testing, but makes the whole algorithm controllable without a specific instruction file.
	void
	setup_options_classes();
	
	void
	setup_design_classes();
	
	///@brief Set constraint and chainbreak score on scorefunction if not already set.
	void
	setup_scorefxns();
	
	void
	setup_constraints(core::pose::Pose & pose);
	
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	
	
	///@brief Post-graft step modeling.  If no graft step, no need to do post-graft modeling.  Default false.
	void
	model_post_graft(core::pose::Pose & pose);
	
	///@brief Post-design step modeling.  Less aggressive, more high resolution.  Default false.
	void
	model_post_design(core::pose::Pose & pose);
	
	
	void
	init_on_new_input( core::pose::Pose const & pose );
	
	///@brief Used to output ongoing current ensembles during the protocol.  Specify a range in the vector to output
	void
	output_ensemble( vector1< core::pose::PoseOP > ensemble, Size range_start, Size range_end, std::string prefix);
	
	///@brief add cluster info to the pose.  Needs to go into pose and not jd2 due to Ensemble generation.
	void
	add_cluster_comments_to_pose(core::pose::Pose & pose);
	
	void
	setup_epitope_residues(core::pose::Pose const & pose);
	
        void
	reorder_poses(utility::vector1<core::pose::PoseOP> & poses);
        
private:
	
	AntibodyGraftDesignMoverOP graft_designer_;
	AntibodySeqDesignMoverOP seq_designer_;
	AntibodyDesignModelerOP modeler_;
	
	AntibodyCDRSetOptions cdr_set_options_;
	AntibodyCDRGraftDesignOptions cdr_graft_design_options_;
	AntibodyCDRSeqDesignOptions cdr_seq_design_options_;
	
	ScoreFunctionOP scorefxn_;
	ScoreFunctionOP design_scorefxn_;
	
	AntibodyInfoOP ab_info_;

	//AntibodyDatabaseManagerOP cdr_db_parser_;

	bool run_graft_designer_;
	bool run_sequence_designer_;
	bool run_post_graft_modeling_;
	bool run_post_design_modeling_;
	bool post_graft_ensemble_output_;
	
	
	utility::vector1<bool> paratope_cdrs_;
	utility::vector1<PDBNumbering > epitope_residues_; //Vector of resnum, chain pairs.
	
	core::Real interface_distance_;
	
};
} //design
} //antibody
} //protocols

#endif //INCLUDED_protocols_antibody_design_AntibodyDesignMover_hh
