// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   src/protocols/qsar/scoring_grid/AtrRepGrid.hh
/// @author Sam DeLuca

#ifndef INCLUDED_protocols_qsar_scoring_grid_RepGrid_hh
#define INCLUDED_protocols_qsar_scoring_grid_RepGrid_hh

#include <protocols/qsar/scoring_grid/RepGrid.fwd.hh>
#include <protocols/qsar/scoring_grid/SingleGrid.hh>

#include <utility/vector1.hh>
#include <utility/tag/XMLSchemaGeneration.fwd.hh>

namespace protocols {
namespace qsar {
namespace scoring_grid {

class RepGrid : public SingleGrid
{
public:
	RepGrid();
	/// @brief Make a copy of the grid, respecting the subclassing.
	GridBaseOP clone() const override;
	void refresh(core::pose::Pose const & pose, core::Vector const & center, core::Size const & ligand_chain_id_to_exclude) override;
	void refresh(core::pose::Pose const & pose, core::Vector const & center) override;
	void refresh(core::pose::Pose const & pose, core::Vector const & center,utility::vector1<core::Size> ligand_chain_ids_to_exclude) override;
	/// @brief serialize the SingleGrid to a json_spirit object
	utility::json_spirit::Value serialize() const override;
	/// @brief deserialize a json_spirit object to a SingleGrid
	void deserialize(utility::json_spirit::mObject data) override;
	void parse_my_tag(utility::tag::TagCOP tag) override;

	static std::string grid_name();
	static void provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );

	std::string hash_fingerprint() const override;

private:
	core::Real radius_;
	core::Real bb_; // score for a clash with a backbone atom
	core::Real sc_; // score for a clash with a side-chain atom
	core::Real ligand_; // score for a clash with a non-protein atom
};


}
}
}
#endif /* ATRREPGRID_HH_ */
