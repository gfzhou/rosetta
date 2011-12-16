// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/scoring/methods/RNA_ScoringInfo.hh
/// @brief  Statistically derived rotamer pair potential class implementation
/// @author Phil Bradley
/// @author Andrew Leaver-Fay

#ifndef INCLUDED_core_scoring_rna_RNA_ScoringInfo_hh
#define INCLUDED_core_scoring_rna_RNA_ScoringInfo_hh

#include <core/types.hh>

// Project headers
#include <core/pose/Pose.fwd.hh>
#include <core/conformation/Residue.fwd.hh>
#include <core/kinematics/Stub.fwd.hh>

#include <core/scoring/rna/RNA_CentroidInfo.hh>
#include <core/scoring/rna/RNA_RawBaseBaseInfo.hh>
#include <core/scoring/rna/RNA_FilteredBaseBaseInfo.hh>
#include <core/scoring/rna/RNA_DataInfo.hh>

#include <basic/datacache/CacheableData.hh>

// Utility headers
// AUTO-REMOVED #include <utility/vector1.hh>

// Numceric Headers
#include <numeric/xyzVector.fwd.hh>

#include <utility/vector1.hh>


// C++

namespace core {
namespace scoring {
namespace rna {

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Keep track of RNA centroid, useful atom, base-base info inside the pose.
class RNA_ScoringInfo : public basic::datacache::CacheableData {

public:

	RNA_ScoringInfo();

  RNA_ScoringInfo( RNA_ScoringInfo const & src );

  basic::datacache::CacheableDataOP
  clone() const
  {
    return new RNA_ScoringInfo( *this );
  }

    // Undefinded, comented out to make python bindings complile
	//void
	//update( pose::Pose const & pose );

  Size
  size() const {
    return rna_centroid_info_.size();
  }

  bool
  calculated() const
  {
    return ( rna_centroid_info_.calculated() &&
						 rna_raw_base_base_info_.calculated() &&
						 rna_filtered_base_base_info_.calculated() );
  }
	utility::vector1< utility::vector1< Size > > const &
	atom_numbers_for_vdw_calculation() const { return atom_numbers_for_vdw_calculation_; }
	
	utility::vector1< utility::vector1< Size > > &
	nonconst_atom_numbers_for_vdw_calculation() { return atom_numbers_for_vdw_calculation_; }

	RNA_CentroidInfo &
	rna_centroid_info() { return rna_centroid_info_; }

	RNA_CentroidInfo const &
	rna_centroid_info() const { return rna_centroid_info_; }

	rna::RNA_RawBaseBaseInfo &
	rna_raw_base_base_info() { return rna_raw_base_base_info_; }

	rna::RNA_RawBaseBaseInfo const &
	rna_raw_base_base_info() const { return rna_raw_base_base_info_; }

	rna::RNA_FilteredBaseBaseInfo &
	rna_filtered_base_base_info() { return rna_filtered_base_base_info_; }

	rna::RNA_FilteredBaseBaseInfo const &
	rna_filtered_base_base_info() const { return rna_filtered_base_base_info_; }

	rna::RNA_DataInfo &
	rna_data_info() { return rna_data_info_; }

	rna::RNA_DataInfo const &
	rna_data_info() const { return rna_data_info_; }

	//ObjexxFCL::FArray2D< Size > &
	//atom_numbers_for_vdw_calculation() { return atom_numbers_for_vdw_calculation_; }

	//ObjexxFCL::FArray2D< Size > const &
	//atom_numbers_for_vdw_calculation() const { return atom_numbers_for_vdw_calculation_; }

	//ObjexxFCL::FArray2D< Size > &
	//atom_numbers_for_backbone_score_calculations() { return atom_numbers_for_backbone_score_calculations_; }

	//ObjexxFCL::FArray2D< Size > const &
	//atom_numbers_for_backbone_score_calculations() const { return atom_numbers_for_backbone_score_calculations_; }

private:

	rna::RNA_CentroidInfo rna_centroid_info_;
	rna::RNA_RawBaseBaseInfo rna_raw_base_base_info_;
	rna::RNA_FilteredBaseBaseInfo rna_filtered_base_base_info_;
	rna::RNA_DataInfo rna_data_info_;
	
	utility::vector1< utility::vector1< Size > > atom_numbers_for_vdw_calculation_;

	//ObjexxFCL::FArray2D< Size > atom_numbers_for_vdw_calculation_;
	//ObjexxFCL::FArray2D< Size > atom_numbers_for_backbone_score_calculations_;

};


RNA_ScoringInfo const &
rna_scoring_info_from_pose( pose::Pose const & pose );

RNA_ScoringInfo &
nonconst_rna_scoring_info_from_pose( pose::Pose & pose );


}
}
}
#endif
