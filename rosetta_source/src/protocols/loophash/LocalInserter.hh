// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/loophash/LocalInserter.hh
/// @brief
/// @author Mike Tyka



#ifndef INCLUDED_protocols_loophash_LocalInserter_hh
#define INCLUDED_protocols_loophash_LocalInserter_hh

#include <protocols/loophash/LocalInserter.fwd.hh>

#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <protocols/loophash/BackboneDB.hh>

#include <core/scoring/ScoreFunction.hh>
#include <core/optimization/MinimizerOptions.hh>
#include <utility/pointer/ReferenceCount.hh>
#include <string>
#include <vector>

#include <utility/vector1.hh>


namespace protocols {
namespace loophash {





// @brief Manages the insertion of an arbitrary length of backbone in a local manner
class LocalInserter : public utility::pointer::ReferenceCount  {
public:
  LocalInserter(){
  }

  virtual
  core::Real
  make_local_bb_change(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  ) = 0;

	// closes gaps between ir and jr
  virtual
  core::Real
  make_local_bb_change_close_gaps(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  ) = 0;
	
	// closes many gaps outside of ir and jr
	// will die if gap exists between ir and jr
  virtual
  core::Real
  make_local_bb_change_include_cut(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  ) = 0;

private:

};

// @brief
class LocalInserter_SimpleMin : public LocalInserter{
public:
  LocalInserter_SimpleMin():
      LocalInserter(),
      options_( "dfpmin", 0.2, true , false ),
      options2_( "dfpmin", 0.02,true , false )
  {
    set_default_score_functions();
  }


  virtual
  core::Real
  make_local_bb_change(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  );

  virtual
  core::Real
  make_local_bb_change_close_gaps(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  );

  virtual
  core::Real
  make_local_bb_change_include_cut(
    core::pose::Pose &start_pose,
    const core::pose::Pose &original_pose,
    const protocols::loophash::BackboneSegment &new_bs,
    core::Size res_pos
  );

private:
  // setup scorefunctions for
  void set_default_score_functions();

  // the scorefunctions themselves
  core::scoring::ScoreFunction scorefxn_rama_cst_;
  core::optimization::MinimizerOptions options_;

  core::scoring::ScoreFunction scorefxn_cen_cst_;
	core::optimization::MinimizerOptions options2_;
};



}

}

#endif

