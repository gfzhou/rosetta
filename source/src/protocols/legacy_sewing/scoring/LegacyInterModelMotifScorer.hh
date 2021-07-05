// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file LegacyInterModelMotifScorer.hh
///
/// @brief
/// @author Tim Jacobs

#ifndef INCLUDED_protocols_legacy_sewing_scoring_LegacyInterModelMotifScorer_hh
#define INCLUDED_protocols_legacy_sewing_scoring_LegacyInterModelMotifScorer_hh

//Unit headers
#include <protocols/legacy_sewing/scoring/LegacyInterModelMotifScorer.fwd.hh>
#include <protocols/legacy_sewing/scoring/LegacyMotifScorer.hh>

//Package headers
#include <protocols/legacy_sewing/conformation/Assembly.fwd.hh>

//Core headers

//Utility headers

namespace protocols {
namespace legacy_sewing  {
namespace scoring {

class LegacyInterModelMotifScorer : public LegacyMotifScorer {

public:

	///@brief default construct
	LegacyInterModelMotifScorer();

	~LegacyInterModelMotifScorer() override{}

	core::Real
	score(
		AssemblyCOP assembly
	) override;

	core::Real
	full_motif_score(
		AssemblyCOP assembly
	);

	//core::Real
	//norm_motif_score(
	// AssemblyCOP assembly
	//);

private:

};


} //scoring namespace
} //legacy_sewing namespace
} //protocols namespace

#endif
