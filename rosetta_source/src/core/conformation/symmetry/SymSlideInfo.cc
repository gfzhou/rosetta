// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is made available under the Rosetta Commons license.
// See http://www.rosettacommons.org/license
// (C) 199x-2007 University of Washington
// (C) 199x-2007 University of California Santa Cruz
// (C) 199x-2007 University of California San Francisco
// (C) 199x-2007 Johns Hopkins University
// (C) 199x-2007 University of North Carolina, Chapel Hill
// (C) 199x-2007 Vanderbilt University

/// @brief  Class to store ingformation about symmetrical dofs
/// @file   core/conformation/symmetry/SymSlideInfo.cc
/// @author Ingemar Andre

// Unit headers
#include <core/conformation/symmetry/SymSlideInfo.hh>

// Utility header
// AUTO-REMOVED #include <utility/exit.hh>
// AUTO-REMOVED #include <utility/io/izstream.hh>
// AUTO-REMOVED #include <utility/string_util.hh>

#include <utility/vector1.hh>


namespace core {
namespace conformation {
namespace symmetry {

SymSlideInfo::SymSlideInfo()
{
	slide_type_ = RANDOM;
	score_criteria_ = CEN_DOCK_SCORE;
	SlideCriteriaVal_ = "AUTOMATIC";
}

SymSlideInfo::SymSlideInfo( SymSlideInfo const & src )
{
	slide_type_ = src.slide_type_;
  score_criteria_ = src.score_criteria_;
  SlideCriteriaVal_ = src.SlideCriteriaVal_;
	slide_order_ = src.slide_order_;
}

	SymSlideInfo &
  SymSlideInfo::operator=( SymSlideInfo const & src ) {
		slide_type_ = src.slide_type_;
		score_criteria_ = src.score_criteria_;
		SlideCriteriaVal_ = src.SlideCriteriaVal_;
		slide_order_ = src.slide_order_;
	return *this;
	}

	SymSlideInfo::~SymSlideInfo() {}

	// setter functions
	void SymSlideInfo::set_slide_type( SlideType slide_type )
	{
		slide_type_ = slide_type;
	}

	void SymSlideInfo::set_SlideCriteriaType( SlideCriteriaType score_criteria )
	{
		score_criteria_ = score_criteria;
	}

	void SymSlideInfo::set_SlideCriteriaVal( std::string SlideCriteriaVal )
	{
		SlideCriteriaVal_ = SlideCriteriaVal;
	}

	void SymSlideInfo::set_slide_order( std::vector<core::Size> slide_order )
	{
		slide_order_ = slide_order;
	}

// get functions
	SlideType
	SymSlideInfo::get_slide_type() const
	{
		return slide_type_;
	}

	SlideCriteriaType
	SymSlideInfo::get_SlideCriteriaType() const
	{
		return score_criteria_;
	}

	std::string
	SymSlideInfo::get_SlideCriteriaVal() const
	{
		return SlideCriteriaVal_;
	}

	std::vector<core::Size>
	SymSlideInfo::get_slide_order() const
	{
		return slide_order_;
	}

} // symmetry
} // conformation
} // core
