// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   basic/options/option.cc
/// @brief  Program options global and initialization function
/// @author Stuart G. Mentzer (Stuart_Mentzer@objexx.com)


// Unit headers
#include <basic/options/option.hh>
#include <basic/options/option.cc.include.gen.hh>              // for add_al...

#include <utility/options/OptionCollection.hh>                 // for Option...

#include <basic/Tracer.hh>                                     // for Tracer
#include <basic/options/keys/carbohydrates.OptionKeys.gen.hh>  // for glycam...
#include <basic/options/keys/chemical.OptionKeys.gen.hh>       // for overri...
#include <basic/options/keys/in.OptionKeys.gen.hh>             // for altern...
#include <basic/options/keys/out.OptionKeys.gen.hh>            // for path, pdb
#include <basic/options/keys/score.OptionKeys.gen.hh>          // for no_pro...
#include <basic/options/keys/cryst.OptionKeys.gen.hh>
#include <basic/options/keys/testing.OptionKeys.gen.hh>

#include <basic/options/option.cc.gen.hh>                      // for add_al...

#include <iosfwd>                                              // for string
#include <string>                                              // for allocator
#include <utility/options/BooleanOption.hh>                    // for Boolea...
#include <utility/options/PathOption.hh>                       // for PathOp...
#include <utility/options/PathVectorOption.hh>                 // for PathVe...
#include <utility/options/StringVectorOption.hh>               // for String...
#include <utility/options/VectorOption_T_.hh>                  // for Vector...
#include <utility/vector1.hh>                                  // for vector1

namespace basic {
namespace options {

static THREAD_LOCAL basic::Tracer TR( "basic.options" );

/// @details OptionCollection global
utility::options::OptionCollection option;


/// @brief Named verbosity levels
int const silent  ( 0 ); // No messages output
int const quiet   ( 1 );
int const standard( 2 );
int const inform  ( 4 );
int const chat    ( 6 );
int const yap     ( 7 );
int const gush    ( 8 );
int const verbose ( 9 ); // All messages output


/// @brief Initialize the options
OptionCollection &
initialize()
{

	using namespace utility::options;
	using namespace basic::options::OptionKeys;
	using utility::file::PathName;
	// Load the built-in options
	option.add_built_in_options();
	// Include options generated by Python script
	add_all_rosetta_options( option );
	// Check for problems in the option specifications
	option.check_specs();
	return option;
}


/// @brief Process the specified options - do this prior to
/// @note  Do more complex value setup and checks here than the option system provides.
OptionCollection &
pre_tracer_process() {

	// WARNING - Do not access the Tracer system here, directly or indirectly.

	// Integration test meta option
	{
		using namespace basic::options::OptionKeys;
		if ( option[ testing::INTEGRATION_TEST ].value() ) {
			// Test for user() such that individual tests can override this if they really want.
			if ( ! option[ run::constant_seed ].user() ) {
				option[ run::constant_seed ].value( true );
			}
			if ( ! option[ run::nodelay ].user() ) {
				option[ run::nodelay ].value( true );
			}
			if ( ! option[ run::no_prof_info_in_silentout ].user() ) {
				option[ run::no_prof_info_in_silentout ].value( true );
			}
#ifdef USEMPI
			if ( ! option[ out::mpi_tracer_to_file ].user() ) {
				option[ out::mpi_tracer_to_file ].value( "mpi_log" );
				option[ out::mpi_tracer_to_file ].activate(); // So that `user()` triggers correctly
			}
#endif
		}
	}

	return option;
}

/// @brief Process the specified options.
/// @note  Do more complex value setup and checks here than the option system provides.
OptionCollection &
process()
{
	using namespace utility::options;

	{ // Input paths
		using namespace basic::options::OptionKeys::in::path;

		option[ fragments ].default_to( option[ path ] );
		option[ pdb ].default_to( option[ path ] );
		option[ database ].default_to( option[ path ] );
	}

	{ // Output paths
		using namespace basic::options::OptionKeys::out::path;

		option[ pdb ].default_to( option[ path ] );
		option[ score ].default_to( option[ path ] );
	}

	{  // Carbohydrate-related umbrella options to make life easier for users
		using namespace basic::options::OptionKeys;

		if ( option[ in::include_sugars ] ) {
			option[ score::no_pro_close_ring_closure ].value( true );
			//option[ score::ring_close_shadow_constraint ].value( 0.1 );
		}
		if ( option[ carbohydrates::glycam_pdb_format ] ) {
			utility::vector1< std::string > alt_codes_list;
			if ( option[ in::alternate_3_letter_codes ].active() ) {
				alt_codes_list.append( option[ in::alternate_3_letter_codes ]() );
			}
			alt_codes_list.push_back( "glycam.codes" );
			option[ in::alternate_3_letter_codes ].value( alt_codes_list );
		}
	}

	//fd crystal stuff
	{
		using namespace basic::options::OptionKeys;

		if ( option[ symmetry::symmetry_definition ].user() ) {
			if ( option[ symmetry::symmetry_definition ]() == "CRYST1" ) {
				option[ in::preserve_crystinfo ].value( true );
			}
		}
	}

	TR.flush();
	return option;
}


} // namespace options
} // namespace basic
