// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/select/residue_selector/ResiduePDBInfoHasLabelSelector.cc
/// @brief  The ResiduePDBInfoHasLabelSelector selects residues using a string containing residue names
/// @author Tom Linsky (tlinsky@uw.edu))

// Unit headers
#include <core/select/residue_selector/ResiduePDBInfoHasLabelSelector.hh>
#include <core/select/residue_selector/ResidueSelectorCreators.hh>

// Basic Headers
#include <basic/datacache/DataMap.hh>

// Package headers
#include <core/pose/PDBInfo.hh>

// Utility Headers
#include <basic/Tracer.hh>
#include <utility/tag/Tag.hh>

// C++ headers
#include <utility/assert.hh>

static THREAD_LOCAL basic::Tracer TR( "core.select.residue_selector.ResiduePDBInfoHasLabelSelector" );

namespace core {
namespace select {
namespace residue_selector {

ResiduePDBInfoHasLabelSelector::ResiduePDBInfoHasLabelSelector():
	ResidueSelector(),
	label_( "" )
{
}

ResiduePDBInfoHasLabelSelector::ResiduePDBInfoHasLabelSelector( std::string const & label_str ):
	ResidueSelector(),
	label_( label_str )
{
}

ResiduePDBInfoHasLabelSelector::~ResiduePDBInfoHasLabelSelector() {}

ResidueSubset
ResiduePDBInfoHasLabelSelector::apply( core::pose::Pose const & pose ) const
{
	debug_assert( ! label_.empty() );

	ResidueSubset subset( pose.total_residue(), false );
	// quit here if there are no residues in the pose
	if ( pose.total_residue() == 0 ) {
		return subset;
	}

	if ( ! pose.pdb_info() ) {
		TR << "Not selecting any residues because there is no pdb info associated with the pose!" << std::endl;
		return subset;
	}

	for ( core::Size i=1; i<=pose.total_residue(); ++i ) {
		subset[ i ] = pose.pdb_info()->res_haslabel( i, label_ );
	}

	return subset;
}

void
ResiduePDBInfoHasLabelSelector::parse_my_tag(
	utility::tag::TagCOP tag,
	basic::datacache::DataMap &)
{
	if ( tag->hasOption( "property" ) ) {
		set_label( tag->getOption< std::string >( "property" ) );
	}
	if ( label_.empty() ) {
		std::stringstream err_msg;
		err_msg << "ResiduePDBInfoHasLabel selector requires 'property' to be specified. From ResiduePDBInfoHasLabelSelector::parse_my_tag." << std::endl;
		throw utility::excn::EXCN_RosettaScriptsOption( err_msg.str() );
	}
}

void
ResiduePDBInfoHasLabelSelector::set_label( std::string const & label_str )
{
	label_ = label_str;
}

std::string ResiduePDBInfoHasLabelSelector::get_name() const {
	return ResiduePDBInfoHasLabelSelector::class_name();
}

std::string ResiduePDBInfoHasLabelSelector::class_name() {
	return "ResiduePDBInfoHasLabel";
}

ResidueSelectorOP
ResiduePDBInfoHasLabelSelectorCreator::create_residue_selector() const {
	return ResidueSelectorOP( new ResiduePDBInfoHasLabelSelector );
}

std::string
ResiduePDBInfoHasLabelSelectorCreator::keyname() const {
	return ResiduePDBInfoHasLabelSelector::class_name();
}

} //namespace residue_selector
} //namespace select
} //namespace core
