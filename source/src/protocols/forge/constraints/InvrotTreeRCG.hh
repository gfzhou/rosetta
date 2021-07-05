// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/forge/constraints/InvrotTreeRCG.hh
///
/// @brief
/// @author Florian Richter, floric@u.washington.edu, may 2012
/// @modified Tom Linsky, tlinsky@uw.edu, nov 2012

#ifndef INCLUDED_protocols_forge_constraints_InvrotTreeRCG_hh
#define INCLUDED_protocols_forge_constraints_InvrotTreeRCG_hh

//unit headers
#include <protocols/forge/constraints/InvrotTreeRCG.fwd.hh>

//package headers
#include <protocols/forge/remodel/RemodelConstraintGenerator.hh>

//project headers
#include <core/pose/Pose.fwd.hh>

#include <protocols/toolbox/match_enzdes_util/AlignPoseToInvrotTreeMover.fwd.hh>
#include <protocols/toolbox/match_enzdes_util/AllowedSeqposForGeomCst.fwd.hh>
#include <protocols/toolbox/match_enzdes_util/EnzConstraintIO.fwd.hh>
#include <protocols/toolbox/match_enzdes_util/InvrotTree.fwd.hh>

// C++headers

namespace protocols {
namespace forge {
namespace constraints {

/// @brief a RemodelConstraintGenerator wrapper that makes
/// the constraints generated by an InvrotTree available in
/// VLB or XML
class InvrotTreeRCG : public remodel::RemodelConstraintGenerator
{

public:

	InvrotTreeRCG();

	InvrotTreeRCG( InvrotTreeRCG const & rval );

	InvrotTreeRCG(
		toolbox::match_enzdes_util::InvrotTreeOP invrot_tree,
		toolbox::match_enzdes_util::AllowedSeqposForGeomCstOP geomcst_seqpos
	);

	~InvrotTreeRCG() override;

	void
	parse_my_tag( TagCOP tag,
		basic::datacache::DataMap & data
	) override;


	protocols::moves::MoverOP
	fresh_instance() const override;

	protocols::moves::MoverOP
	clone() const override;

	void
	apply( core::pose::Pose & pose ) override;

	void
	generate_remodel_constraints( core::pose::Pose const & pose ) override;

	/// @brief sets up the allowed sequence positions for enzdes geometric constraints
	void
	init( core::pose::Pose const & pose ) override;

public:
	// non-virtual member functions

	/// @brief tells the mover whether it should add the ligand to the pose
	void set_add_ligand_to_pose( bool const add_lig );

	/// @brief sets up the invrot_tree_ and enzcst_io_ from an enzdes constraint filename.
	void set_cstfile( std::string const & cstfilename );

	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


protected:

private:
	bool add_ligand_to_pose_;
	toolbox::match_enzdes_util::InvrotTreeOP invrot_tree_;
	toolbox::match_enzdes_util::EnzConstraintIOCOP enzcst_io_;
	toolbox::match_enzdes_util::AllowedSeqposForGeomCstOP geomcst_seqpos_;
	toolbox::match_enzdes_util::AlignPoseToInvrotTreeMoverOP setup_align_pose_;
	toolbox::match_enzdes_util::AlignPoseToInvrotTreeMoverOP run_align_pose_;
}; //class InvrotTreeRCG


} //namespace remodel
} //namespace forge
} //namespace protocols

#endif // INCLUDED_protocols_forge_remodel_InvrotTreeRCG_HH
