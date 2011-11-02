// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   /git/src/protocols/ligand_docking/scoring_grid/VdwGrid.hh
/// @author Sam DeLuca

#ifndef INCLUDED_protocols_qsar_scoring_grid_VdwGrid_hh
#define INCLUDED_protocols_qsar_scoring_grid_VdwGrid_hh

#include <protocols/qsar/scoring_grid/VdwGrid.fwd.hh>
#include <protocols/qsar/scoring_grid/GridBase.hh>
#include <numeric/interpolation/spline/SplineGenerator.hh>

#include <utility/vector1.hh>



namespace protocols {
namespace qsar {
namespace scoring_grid {

class VdwGrid : public GridBase
{
public:

	VdwGrid();
	VdwGrid(core::Real weight);
	virtual void refresh(core::pose::Pose const & pose, core::Vector const & center, core::Size const & );
	virtual void refresh(core::pose::Pose const & pose, core::Vector const & center);
	virtual void refresh(core::pose::Pose const & pose, core::Vector const & center, utility::vector1<core::Size> );

	void parse_my_tag(utility::tag::TagPtr const tag);

	core::Real score(core::conformation::Residue const & residue, core::Real const max_score, qsarMapOP qsar_map);
private:


	numeric::interpolation::spline::SplineGenerator lj_spline_;
	core::Real cutoff_;

};

}
}
}

#endif /* ATRGRID_CC_ */
