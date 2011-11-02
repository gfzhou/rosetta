// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/match/output/SameRotamerComboGrouper.hh
/// @brief
/// @author Alex Zanghellini (zanghell@u.washington.edu)
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com), porting to mini

#ifndef INCLUDED_protocols_match_output_SameRotamerComboGrouper_hh
#define INCLUDED_protocols_match_output_SameRotamerComboGrouper_hh

// Unit headers
#include <protocols/match/output/SameRotamerComboGrouper.fwd.hh>

// Package headers
#include <protocols/match/output/MatchGrouper.hh>
#include <protocols/match/Hit.fwd.hh>

// Utility headers
// AUTO-REMOVED #include <utility/OrderedTuple.hh>
// AUTO-REMOVED #include <utility/vector1.hh>

// C++ headers
#include <map>

#include <utility/OrderedTuple.fwd.hh>


namespace protocols {
namespace match {
namespace output {

class SameRotamerComboGrouper : public MatchGrouper {
public:
	typedef core::Real Real;
	typedef core::Size Size;
	typedef std::map< utility::OrderedTuple< utility::vector1< Size > >, Size > RotamerComboCountMap;

public:
	SameRotamerComboGrouper();
	SameRotamerComboGrouper( Size ncst );

	virtual
	~SameRotamerComboGrouper();

	virtual
	Size
	assign_group_for_match(
		match const & m
	);

	virtual
	Size
	assign_group_for_match(
		match_dspos1 const & m
	);

	virtual
	void
	reset();

	void
	set_n_geometric_constraints( Size n_csts );

private:

	Size n_geometric_constraints_;
	RotamerComboCountMap rotamer_combo_indexer_;

};

}
}
}

#endif
