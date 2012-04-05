// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer, email:license@u.washington.edu

/// @file protocols/antibody2/Ab_H3_perturb_ccd_build.hh
/// @brief Build a homology model of an antibody2
/// @detailed
///
///
/// @author Jianqing Xu (xubest@gmail.com)



#ifndef INCLUDED_protocols_antibody2_Ab_H3_perturb_ccd_build_hh
#define INCLUDED_protocols_antibody2_Ab_H3_perturb_ccd_build_hh


#include <protocols/moves/Mover.hh>
#include <protocols/moves/MoverContainer.fwd.hh>
#include <protocols/loops/Loop.hh>
#include <protocols/loops/Loops.hh>
#include <core/pose/Pose.hh>
#include <core/pack/task/TaskFactory.fwd.hh>
#include <core/fragment/FragSet.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <protocols/antibody2/AntibodyInfo.fwd.hh>
#include <protocols/antibody2/Ab_H3_perturb_ccd_build.fwd.hh>
#include <protocols/moves/MonteCarlo.fwd.hh>
#include <protocols/moves/PyMolMover.fwd.hh>






namespace protocols {
namespace antibody2 {
        
    
    
class Ab_H3_perturb_ccd_build: public moves::Mover {
            
            
public:
    
    /// @brief default constructor
	Ab_H3_perturb_ccd_build();
    
	/// @brief constructor with arguments
    Ab_H3_perturb_ccd_build(
                            bool is_camelid, 
                            AntibodyInfoOP & antibody_in);
    
    /// @brief constructor with arguments
    Ab_H3_perturb_ccd_build(bool current_loop_is_H3, 
                            bool is_camelid, 
                            AntibodyInfoOP & antibody_in);
        
    virtual protocols::moves::MoverOP clone() const;
    
	/// @brief default destructor
	~Ab_H3_perturb_ccd_build();
    


    virtual void apply( core::pose::Pose & pose );
    
    virtual std::string get_name() const {
        return "Ab_H3_perturb_ccd_build";
    }
    
    void read_and_store_fragments( core::pose::Pose & pose );
    
    void turn_on_and_pass_the_pymol(moves::PyMolMoverOP pymol){
        use_pymol_diy_ = true;
        pymol_ = pymol;
    }
    
    void turn_on_pymol_diy(){
        use_pymol_diy_ = true;
    }
    
    void pass_the_loop(loops::Loop & input_loop){
        input_loop_ = input_loop;
    }
    
    void turn_off_H3_filter(){
        H3_filter_ = false;
    }
    
    
private:

    AntibodyInfoOP ab_info_;
    
    loops::Loop input_loop_;
    
    bool user_defined_;
    bool is_camelid_;
    bool use_pymol_diy_;
    
    protocols::moves::PyMolMoverOP pymol_;

    
    void set_default();
    void init( bool current_loop_is_H3, bool is_camelid, AntibodyInfoOP & antibody_in);

    void finalize_setup( core::pose::Pose & pose );

    
    /// @brief Build centroid mode CDR H3 loop
	void build_centroid_loop( core::pose::Pose & pose );
    
    void scored_frag_close(
                           core::pose::Pose & pose_in,
                           loops::Loop const trimmed_cdr_h3 );

        
    
    /// @brief size of loop above which 9mer frags are used
	core::Size cutoff_9_; // default 16
    
	/// @brief size of loop above which 3mer frags are used
	core::Size cutoff_3_; // default 6
    
    core::scoring::ScoreFunctionOP lowres_scorefxn_;

    core::Real cen_cst_;
    
    /// @brief flag indicating that current loop being modeled is CDR H3
	bool current_loop_is_H3_;
    
    /// @brief actually enables H3 filter for H3 operations
	bool H3_filter_;
    
    core::Size num_cycles1_;
    core::Size max_ccd_cycles_;
    
    utility::vector1< core::fragment::FragSetOP > cdr_h3_frags_;
    
    core::Real h3_fraction_;
    core::Real ccd_threshold_;
    core::Real Temperature_;
    
    protocols::moves::MonteCarloOP mc_, outer_mc_;
    
};
    
    
    
} // namespace antibody2
} // namespace protocols

#endif




