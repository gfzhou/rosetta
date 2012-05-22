// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/features/ProtocolFeatures.hh
/// @brief  report Orbital geometry and scores to features Statistics Scientific Benchmark
/// @author Matthew O'Meara

#ifndef INCLUDED_protocols_features_ProtocolFeatures_hh
#define INCLUDED_protocols_features_ProtocolFeatures_hh

// Unit Headers
#include <protocols/features/FeaturesReporter.hh>
#include <protocols/features/ProtocolFeatures.fwd.hh>

// Project Headers
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>
// AUTO-REMOVED #include <utility/sql_database/DatabaseSessionManager.hh>
#include <utility/vector1.fwd.hh>

// C++ Headers
#include <string>

#include <utility/vector1.hh>


namespace protocols{
namespace features{

class ProtocolFeatures : public protocols::features::FeaturesReporter {
public:
	ProtocolFeatures();

	ProtocolFeatures( ProtocolFeatures const & src );

	virtual ~ProtocolFeatures();

	///@brief return string with class name
	std::string
	type_name() const;

	///@brief generate the table schemas and write them to the database
	virtual void
	write_schema_to_db(utility::sql_database::sessionOP db_session) const;


	///@brief return the set of features reporters that are required to
	///also already be extracted by the time this one is used.
	utility::vector1<std::string>
	features_reporter_dependencies() const;

	///@brief return sql statements that setup helpful indices on the tables
	std::string
	indices() const;

	///@brief return sql statments that add indexes check invariants etc.
	//std::string cleanup() const;

	///@brief collect all the feature data for the pose
	///if protocol_id is 0 autoincrement the protocol_id
	core::Size
	report_features(
		core::Size protocol_id,
		utility::sql_database::sessionOP db_session
	);

};

} // features namespace
} // protocols namespace

#endif // include guard
