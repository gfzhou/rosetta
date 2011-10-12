// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/features/GeometricSolvationFeatures.cc
/// @brief  report geometric solvation energy for each polar site to a features database
/// @author Matthew O'Meara

// Unit Headers
#include <protocols/features/GeometricSolvationFeatures.hh>

// Platform Headers
#include <core/pose/Pose.hh>
#include <core/scoring/geometric_solvation/ExactOccludedHbondSolEnergy.hh>
#include <core/types.hh>

// Utility Headers
#include <utility/sql_database/DatabaseSessionManager.hh>
#include <utility/vector1.hh>

// Basic Headers
#include <basic/options/option.hh>
#include <basic/options/keys/inout.OptionKeys.gen.hh>
#include <basic/database/sql_utils.hh>


// External Headers
#include <cppdb/frontend.h>
#include <cppdb/errors.h>

// C++ Headers
#include <string>

namespace protocols{
namespace features{

using std::string;
using core::Size;
using core::Real;
using core::pose::Pose;
using core::scoring::geometric_solvation::ExactOccludedHbondSolEnergy;
using utility::vector1;
using utility::sql_database::sessionOP;
using cppdb::statement;
using cppdb::result;

GeometricSolvationFeatures::GeometricSolvationFeatures() :
	geo_sol_energy_(ExactOccludedHbondSolEnergy())
{

	//I would like to simply assert that this has been called, but that
	//currently is not possible.
	//geo_sol_energy_.setup_for_scoring(pose, scfxn_);

}

GeometricSolvationFeatures::GeometricSolvationFeatures(
	GeometricSolvationFeatures const & src ) :
	geo_sol_energy_(src.geo_sol_energy_),
	FeaturesReporter()
{}



string
GeometricSolvationFeatures::type_name() const { return "GeometricSolvationFeatures"; }

string
GeometricSolvationFeatures::schema() const {
	string db_mode(basic::options::option[basic::options::OptionKeys::inout::database_mode]);

	if(db_mode == "sqlite3")
	{
		return
			"CREATE TABLE IF NOT EXISTS geometric_solvation (\n"
			"	struct_id INTEGER,\n"
			"	hbond_site_id TEXT,\n"
			"	geometric_solvation_exact REAL,\n"
			"	FOREIGN KEY (struct_id, hbond_site_id)\n"
			"		REFERENCES hbond_sites(struct_id, site_id)\n"
			"		DEFERRABLE INITIALLY DEFERRED,\n"
			"	PRIMARY KEY(struct_id, hbond_site_id));";
	}else if(db_mode == "mysql")
	{
		return
			"CREATE TABLE IF NOT EXISTS geometric_solvation (\n"
			"	struct_id INTEGER,\n"
			"	hbond_site_id INTEGER,\n"
			"	geometric_solvation_exact TEXT,\n"
			"	FOREIGN KEY (struct_id, hbond_site_id) REFERENCES hbond_sites (struct_id, site_id),\n"
			"	PRIMARY KEY(struct_id, hbond_site_id));";
	}else
	{
		return "";
	}

}

Size
GeometricSolvationFeatures::report_features(
	Pose const & pose,
	vector1< bool > const &,
	Size struct_id,
	sessionOP db_session
){

	statement stmt = (*db_session) <<
		"SELECT\n"
		"	site.site_id,\n"
		"	site.resNum,\n"
		"	site.atmNum\n"
		"FROM\n"
		"	hbond_sites as site\n"
		"WHERE\n"
		"	site.struct_id = ?;" << struct_id;

	result res(basic::database::safely_read_from_database(stmt));

	while(res.next()){
		Size site_id, resNum, atmNum;
		res >> site_id >> resNum >> atmNum;

		Real const geometric_solvation_exact(
		  geo_sol_energy_.compute_polar_group_sol_energy(
			pose,
			pose.residue(resNum),
			atmNum));


		statement stmt = (*db_session)
			<< "INSERT INTO geometric_solvation VALUES (?,?,?)"
			<< struct_id
			<< site_id
			<< geometric_solvation_exact;
		basic::database::safely_write_to_database(stmt);
	}

	// locate the polar sites from the hbond_sites table

	return 0;
}

} //namesapce
} //namespace
