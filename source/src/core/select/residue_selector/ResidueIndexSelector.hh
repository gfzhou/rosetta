// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/select/residue_selector/ResidueIndexSelector.hh
/// @brief  The ResidueIndexSelector selects residues using a string containing pose indices
/// @author Robert Lindner (rlindner@mpimf-heidelberg.mpg.de)

#ifndef INCLUDED_core_select_residue_selector_ResidueIndexSelector_HH
#define INCLUDED_core_select_residue_selector_ResidueIndexSelector_HH

// Unit headers
#include <core/select/residue_selector/ResidueIndexSelector.fwd.hh>

// Package headers
#include <core/types.hh>
#include <core/select/residue_selector/ResidueSelector.hh>
#include <core/pose/Pose.hh>

// Utility Headers
#include <utility/tag/Tag.fwd.hh>
#include <utility/vector1.hh>

// C++ headers
#include <set>

namespace core {
namespace select {
namespace residue_selector {

/// @brief The ResidueIndexSelector returns a ResidueSubset, i.e. a utility::vector1< bool > containing
/// 'true' for residue positions which match the given residue index. The index is read as comma-separated
/// list of either Rosetta indices (e.g. 10) or PDB numbers (e.g. 10A, residue 10 of chain A). Detection
/// and mapping from PDB to Rosetta residue numbers is done internally.
class ResidueIndexSelector : public ResidueSelector {
public:
	// derived from base class
	ResidueIndexSelector();

	/// @brief Copy constructor
	///
	ResidueIndexSelector( ResidueIndexSelector const &src);

	/// @brief Clone operator.
	/// @details Copy this object and return an owning pointer to the new object.
	virtual ResidueSelectorOP clone() const;

	ResidueIndexSelector( std::string const & index_str );
	virtual ~ResidueIndexSelector();

	virtual ResidueSubset apply( core::pose::Pose const & pose ) const;
	virtual void parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap &
	);

	virtual
	std::string
	get_name() const;

	static std::string class_name();


	//unit-specific
	/**
	* @brief sets the string by which residues are selected
	*/
	void set_index( std::string const & index_str );

private: // data members
	std::string index_str_;
};

} //namespace residue_selector
} //namespace select
} //namespace core


#endif
