// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file src/core/scoring/constraints/FadeFunc.hh
/// @brief Definition for functions used in definition of constraints.
/// @author Rhiju Das


#include <core/scoring/constraints/FadeFunc.hh>

#include <core/types.hh>

#include <utility/pointer/ReferenceCount.hh>

// AUTO-REMOVED #include <numeric/random.functions.hh>
// AUTO-REMOVED #include <ObjexxFCL/format.hh>

// C++ Headers
#include <iostream>

#include <utility/vector1.hh>


namespace core {
namespace scoring {
namespace constraints {

FuncOP
FadeFunc::clone() const
{
	return new FadeFunc( cutoff_lower_, cutoff_upper_,
											 fade_zone_, well_depth_,
											 well_offset_ );
}

Real
FadeFunc::func( Real const z ) const
{

	Real fade_value( 1.0 );

	if (z < cutoff_lower_ || z > cutoff_upper_ ){
		fade_value = 0.0;
	} else if ( z < cutoff_lower_ + fade_zone_ ) {
		//Check little strip near lower cutoff.
		Real const b = -1.0 * ( z - (cutoff_lower_ + fade_zone_) )/ fade_zone_;
		Real const b2 = b*b;
		Real const b3 = b2*b;
		fade_value = ( 2 * b3 - 3 * b2 + 1 );
		//		fade_deriv = -1.0 * (6 * b2 - 6 * b ) / fade_zone_;
	} else if ( z > cutoff_upper_ - fade_zone_ ) {
		//Check little strip near upper cutoff.
		Real const b =  ( z - (cutoff_upper_ - fade_zone_) )/ fade_zone_;
		Real const b2 = b*b;
		Real const b3 = b2*b;
		fade_value = ( 2 * b3 - 3 * b2 + 1 );
		//		fade_deriv = (6 * b2 - 6 * b ) / fade_zone_;
	}


	return well_depth_ * fade_value + well_offset_;
}

Real
FadeFunc::dfunc( Real const z ) const
{

	Real fade_deriv( 0.0 );

	if (z < cutoff_lower_ || z > cutoff_upper_ ){
		fade_deriv = 0.0;
	} else if ( z < cutoff_lower_ + fade_zone_ ) {
		//Check little strip near lower cutoff.
		Real const b = -1.0 * ( z - (cutoff_lower_ + fade_zone_) )/ fade_zone_;
		Real const b2 = b*b;
		//		Real const b3 = b2*b;
		//fade_value = ( 2 * b3 - 3 * b2 + 1 );
		fade_deriv = -1.0 * (6 * b2 - 6 * b ) / fade_zone_;
	} else if ( z > cutoff_upper_ - fade_zone_ ) {
		//Check little strip near upper cutoff.
		Real const b =  ( z - (cutoff_upper_ - fade_zone_) )/ fade_zone_;
		Real const b2 = b*b;
		//		Real const b3 = b2*b;
		//		fade_value = ( 2 * b3 - 3 * b2 + 1 );
		fade_deriv = (6 * b2 - 6 * b ) / fade_zone_;
	}

	return well_depth_ * fade_deriv;
}

void
FadeFunc::read_data( std::istream& in ) {
	in >> cutoff_lower_ >> cutoff_upper_ >> fade_zone_ >> well_depth_ ;

	well_offset_ = 0.0;
	if( in.good() )  in >> well_offset_;

}

void
FadeFunc::show_definition( std::ostream &out ) const {
	out << "FADE " <<
		' ' << cutoff_lower_ <<
		' ' << cutoff_upper_ <<
		' ' << fade_zone_ <<
		' ' << well_depth_ <<
		' ' << well_offset_ << std::endl;
}

Size
FadeFunc::show_violations( std::ostream& out, Real x, Size verbose_level, Real threshold) const {
	if (verbose_level > 100 ) {
		out << "FADE " <<  ( x < cutoff_lower_ || x > cutoff_upper_ ) << std::endl;
	}
	return Func::show_violations( out, x, verbose_level, threshold);
}


} // namespace constraints
} // namespace scoring
} // namespace core

