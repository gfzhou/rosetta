// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file SWA_JobParameters_Setup.hh
/// @brief
/// @detailed
///
/// @author Parin Sripakdeevong
/// @author Rhiju Das


#ifndef INCLUDED_protocols_swa_SWA_JobParameters_Setup_HH
#define INCLUDED_protocols_swa_SWA_JobParameters_Setup_HH

#include <protocols/swa/rna/StepWiseRNA_Util.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/chemical/ResidueTypeSet.fwd.hh>
#include <protocols/swa/rna/StepWiseRNA_JobParameters.fwd.hh>
#include <core/types.hh>
#include <utility/vector1.hh>
#include <protocols/moves/Mover.hh>
#include <ObjexxFCL/FArray1D.fwd.hh>
#include <ObjexxFCL/FArray1D.hh>

#include <string>
#include <map>

namespace protocols {
namespace swa {
namespace rna {



  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  class StepWiseRNA_JobParameters_Setup: public utility::pointer::ReferenceCount {
  public:

    //constructor!
		StepWiseRNA_JobParameters_Setup( utility::vector1< core::Size > const & moving_res_list,
													 std::string const & full_sequence,
													 utility::vector1< core::Size > const & input_res,
													 utility::vector1< core::Size > const & input_res2,
													 utility::vector1< core::Size > const & cutpoint_open,
													 Size const & cutpoint_closed );


    //destructor -- necessary?
    ~StepWiseRNA_JobParameters_Setup();

    virtual void apply();

		StepWiseRNA_JobParametersOP & job_parameters();

		void
		set_fixed_res(utility::vector1 < core::Size > const & fixed_res );

		void
		set_virtual_res(utility::vector1 < core::Size > const & set_virtual_res_list);

		void
		set_terminal_res(utility::vector1 < core::Size > const & terminal_res );

		void
		set_bulge_res(utility::vector1 < core::Size > const & bulge_res );

		void
		set_jump_point_pair_list(utility::vector1< std::string > const & jump_point_pairs_string);

		void
		set_rmsd_res_list(utility::vector1< core::Size > const & setting );

		void
		set_alignment_res(utility::vector1< std::string > const & setting){alignment_res_string_list_=setting; }

		void
		set_filter_user_alignment_res(bool const setting){filter_user_alignment_res_=setting; }

		void
		set_native_alignment_res(utility::vector1< Size > const & native_alignment);

		void
		set_global_sample_res_list( utility::vector1 < core::Size > const & setting );

		void
		set_force_syn_chi_res_list( utility::vector1 < core::Size > const & setting );

		void
		set_force_north_ribose_list( utility::vector1 < core::Size > const & setting );

		void
		set_force_south_ribose_list( utility::vector1 < core::Size > const & setting );

		void
		set_protonated_H1_adenosine_list( utility::vector1 < core::Size > const & setting );

		void
		set_output_extra_RMSDs( bool const setting);

		void
		set_add_virt_res_as_root( bool const setting);

		void
		set_input_tags( utility::vector1< std::string > const & setting){ input_tags_=setting; } //Only called if check_for_previously_closed_cutpoint_with_input_pose is true

		void
		set_silent_files_in( utility::vector1< std::string > const & setting){ silent_files_in_= setting; } //Only called if check_for_previously_closed_cutpoint_with_input_pose  is true

		void
		set_allow_chain_boundary_jump_partner_right_at_fixed_BP( bool const setting){ allow_chain_boundary_jump_partner_right_at_fixed_BP_=setting; }

		void
		set_allow_fixed_res_at_moving_res( bool const setting){ allow_fixed_res_at_moving_res_=setting; }

		void
		set_simple_append_map( bool const setting){ simple_append_map_=setting; }


  private:

		void
		check_moving_res_in_chain( Size const & start_chain, Size const & end_chain,
															 Size const & num_chains, Size & which_chain_has_moving_res  );

		void
		figure_out_working_sequence_and_mapping();

		void
		setup_additional_cutpoint_closed();

		void
		figure_out_chain_boundaries();

		void
		figure_out_jump_partners();

		void
		figure_out_cuts();

		void
		setup_fold_tree();

		void
		figure_out_Prepend_Internal(core::Size const root_res, InternalWorkingResidueParameter const & internal_params);

		void 
		figure_out_working_moving_suite();

		void	
		figure_out_Is_prepend_map();

		bool
		figure_out_Is_residue_prepend(core::Size const seq_num) const;

		core::Size
		input_struct_definition(core::Size const working_seq_num);

		InternalWorkingResidueParameter
		figure_out_partition_definition();

		void
		figure_out_gap_size_and_five_prime_chain_break_res();

		core::Size
		reroot_fold_tree(core::Size const fake_working_moving_suite);

//		void
//		reset_Is_prepend(core::Size const root_res);

		void
		figure_out_best_working_alignment();

		utility::vector1< core::Size >
		get_user_input_alignment_res_list(core::Size const root_res);

		utility::vector1< core::Size >
		get_previously_closed_cutpoint_from_imported_silent_file() const; 


		/////////////////////////////////////////////////////////////////////////////////////

	private:

		Size const moving_res_;
		utility::vector1< core::Size > const moving_res_list_;

		utility::vector1< Size > const cutpoint_open_;
		utility::vector1< Size > added_cutpoint_closed_;
		utility::vector1< Size > fixed_res_;

		ObjexxFCL::FArray1D< bool > is_cutpoint_;
		utility::vector1< std::pair< core::Size, core::Size > > jump_point_pair_list_;
		utility::vector1< std::pair< core::Size, core::Size > > jump_partners_;
		utility::vector1< core::Size > cuts_;

		StepWiseRNA_JobParametersOP job_parameters_;

		utility::vector1< std::string > alignment_res_string_list_;
		bool filter_user_alignment_res_;
		
		utility::vector1< std::string > input_tags_; //for check_for_previously_closed_cutpoint_with_input_pose 
		utility::vector1< std::string > silent_files_in_; //for check_for_previously_closed_cutpoint_with_input_pose 
		bool allow_chain_boundary_jump_partner_right_at_fixed_BP_;
		bool allow_fixed_res_at_moving_res_;
		bool simple_append_map_;

  };

}
} //swa
} // protocols

#endif
