// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   rosetta/benchmark/OptionCollection.bench.cc
///
/// @brief  Access options in the option system
/// @author Matthew O'Meara


#include <apps/benchmark/benchmark.hh>

#include <utility/keys/KeyLookup.hh>
#include <basic/options/option.hh>

class OptionCollectionBenchmark : public Benchmark
{
public:

	OptionCollectionBenchmark(std::string name) : Benchmark(name) {};

	virtual void setUp() {
	};

	virtual void run(core::Real scaleFactor) {
		using namespace utility::keys;
		using namespace basic::options;

		for(int i=0; i<200000*scaleFactor; i++) {
			for( KeyLookup< OptionKey >::ConstIterator
						 k = KeyLookup< OptionKey >::begin(),
						 ke = KeyLookup< OptionKey >::end();
					 k != ke; ++k) {
				if( option.has(*k) ){
					option( *k ).user();
				}
			}
		}
	};

	virtual void tearDown() {};
};
