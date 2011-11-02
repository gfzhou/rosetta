// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   test/core/scoring/methods/ProClosureEnergy.cxxtest.hh
/// @brief  test suite for core::scoring::ProClosureEnergy.cc
/// @author Andrew Leaver-Fay

// Test headers
#include <cxxtest/TestSuite.h>

// Unit headers
#include <core/scoring/methods/ProClosureEnergy.hh>

#include <platform/types.hh>

// Package Headers
#include <test/util/pose_funcs.hh>
#include <test/util/deriv_funcs.hh>
#include <test/core/init_util.hh>

// AUTO-REMOVED #include <core/id/DOF_ID.hh>
// AUTO-REMOVED #include <core/id/TorsionID.hh>
#include <core/kinematics/DomainMap.hh>
// AUTO-REMOVED #include <core/pose/Pose.hh>
// AUTO-REMOVED #include <core/scoring/ScoreFunction.hh>
// AUTO-REMOVED #include <core/io/pdb/pose_io.hh>
// AUTO-REMOVED #include <basic/options/option.hh>

// AUTO-REMOVED #include <core/optimization/MinimizerOptions.hh>
// AUTO-REMOVED #include <core/optimization/AtomTreeMinimizer.hh>

//Auto Headers
#include <utility/vector1.hh>


// --------------- Test Class --------------- //

// using declarations
using namespace core;
using namespace core::id;
using namespace core::kinematics;
using namespace core::pose;
using namespace core::scoring;
using namespace core::scoring::methods;

class ProClosureEnergyTests : public CxxTest::TestSuite {

	public:

	// --------------- Fixtures --------------- //

	// Define a test fixture (some initial state that several tests share)
	// In CxxTest, setUp()/tearDown() are executed around each test case. If you need a fixture on the test
	// suite level, i.e. something that gets constructed once before all the tests in the test suite are run,
	// suites have to be dynamically created. See CxxTest sample directory for example.

	// Shared initialization goes here.
	void setUp() {
		using namespace core;
		core_init();
	}

	// Shared finalization goes here.
	void tearDown() {}

	// --------------- Test Cases --------------- //
	void test_eval_energy()
	{
		Pose trpcage( create_trpcage_ideal_pose() );

		Size before_precision = std::cout.precision();
		std::cout.precision( 16 );

		ProClosureEnergy proclose_energy;
		ScoreFunction sfxn;
		sfxn.set_weight( pro_close, 0.5 );

		/*for ( Size ii = 1; ii <= trpcage.total_residue(); ++ii ) {

			EnergyMap emap;
			std::cout << "{\n"
			"EnergyMap emap;\n";

			proclose_energy.eval_intrares_energy( trpcage.residue( ii ), trpcage, sfxn, emap );
			std::cout << "proclose_energy.eval_intrares_energy( trpcage.residue( " << ii << " ), trpcage, sfxn, emap );\n";
			std::cout << "TS_ASSERT_DELTA( emap[ pro_close ], " << emap[ pro_close ] << ", 1e-12 );\n";
			std::cout << "}\n";

			for ( Size jj = std::max( Size(1), ii-1 ); jj <= std::min( trpcage.total_residue(), ii + 1 ); ++jj ) {

				if ( jj == ii ) continue;

				EnergyMap tbemap;
				std::cout << "{\n"
				"EnergyMap tbemap;\n";

				proclose_energy.residue_pair_energy( trpcage.residue( ii ), trpcage.residue( jj ), trpcage, sfxn, tbemap );
				std::cout << "proclose_energy.residue_pair_energy( trpcage.residue( " << ii << " ), trpcage.residue( " <<  jj <<" ), trpcage, sfxn, tbemap );\n";
				std::cout << "TS_ASSERT_DELTA( tbemap[ pro_close ], " << tbemap[ pro_close ] << ", 1e-12 );\n";
				std::cout << "}\n";

			}
		}*/


		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 1 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 1 ), trpcage.residue( 2 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 2 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 2 ), trpcage.residue( 1 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 2 ), trpcage.residue( 3 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 3 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 3 ), trpcage.residue( 2 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 3 ), trpcage.residue( 4 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 4 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 4 ), trpcage.residue( 3 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 4 ), trpcage.residue( 5 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 5 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 5 ), trpcage.residue( 4 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 5 ), trpcage.residue( 6 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 6 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 6 ), trpcage.residue( 5 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 6 ), trpcage.residue( 7 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 7 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 7 ), trpcage.residue( 6 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 7 ), trpcage.residue( 8 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 8 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 8 ), trpcage.residue( 7 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 8 ), trpcage.residue( 9 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 9 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 9 ), trpcage.residue( 8 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 9 ), trpcage.residue( 10 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 10 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 10 ), trpcage.residue( 9 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 10 ), trpcage.residue( 11 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 11 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 11 ), trpcage.residue( 10 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 11 ), trpcage.residue( 12 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.004436792234172629, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 12 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0.01292460898481068, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 12 ), trpcage.residue( 11 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.004436792234172629, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 12 ), trpcage.residue( 13 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 13 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 13 ), trpcage.residue( 12 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 13 ), trpcage.residue( 14 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 14 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 14 ), trpcage.residue( 13 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 14 ), trpcage.residue( 15 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 15 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 15 ), trpcage.residue( 14 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 15 ), trpcage.residue( 16 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 16 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 16 ), trpcage.residue( 15 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 16 ), trpcage.residue( 17 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 3.287050070076195e-05, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 17 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0.2078919252851279, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 17 ), trpcage.residue( 16 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 3.287050070076195e-05, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 17 ), trpcage.residue( 18 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.008603170724419516, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 18 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0.1050023146212085, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 18 ), trpcage.residue( 17 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.008603170724419516, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 18 ), trpcage.residue( 19 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.07438490402267117, 1e-12 );
		}
		{
		EnergyMap emap;
		proclose_energy.eval_intrares_energy( trpcage.residue( 19 ), trpcage, sfxn, emap );
		TS_ASSERT_DELTA( emap[ pro_close ], 0.2001501300597954, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 19 ), trpcage.residue( 18 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0.07438490402267117, 1e-12 );
		}
		{
		EnergyMap tbemap;
		proclose_energy.residue_pair_energy( trpcage.residue( 19 ), trpcage.residue( 20 ), trpcage, sfxn, tbemap );
		TS_ASSERT_DELTA( tbemap[ pro_close ], 0, 1e-12 );
		}
		std::cout.precision( before_precision );

	}

	void test_proclose_start_score_start_func_match_w_total_flexibility()
	{
		core::pose::Pose pose = create_trpcage_ideal_pose();
		core::scoring::ScoreFunction sfxn;
		sfxn.set_weight( pro_close, 0.5 );
		kinematics::MoveMap movemap( create_movemap_to_allow_all_torsions() );
		AtomDerivValidator adv;
		adv.set_pose( pose );
		adv.set_score_function( sfxn );
		adv.set_movemap( movemap );
		adv.validate_start_func_matches_start_score( 0.3067133582164516 );

	}

	void test_proclose_deriv_check_w_total_flexibility()
	{
		core::pose::Pose pose = create_trpcage_ideal_pose();
		core::scoring::ScoreFunction sfxn;
		sfxn.set_weight( pro_close, 0.5 );
		kinematics::MoveMap movemap( create_movemap_to_allow_all_torsions() );
		AtomDerivValidator adv;
		adv.set_pose( pose );
		adv.set_score_function( sfxn );
		adv.set_movemap( movemap );
		adv.simple_deriv_check( false, 1e-6 );

	}



};


