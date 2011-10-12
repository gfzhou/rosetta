// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/features/AtomAtomPairFeatures.cc
/// @brief  report atom-atom pair geometry and scores to features statistics scientific benchmark
/// @author Matthew O'Meara

// Unit Headers
#include <protocols/features/AtomAtomPairFeatures.hh>

// Project Headers
#include <core/chemical/AA.hh>
#include <core/chemical/ResidueType.fwd.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Conformation.hh>
#include <core/graph/Graph.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/TenANeighborGraph.hh>
#include <core/scoring/Energies.hh>
#include <core/types.hh>
#include <utility/sql_database/DatabaseSessionManager.hh>
#include <utility/vector1.hh>
#include <basic/database/sql_utils.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/FArray5D.hh>

// Numeric Headers
#include <numeric/xyzVector.hh>

// External Headers
#include <cppdb/frontend.h>
#include <cppdb/errors.h>

namespace protocols{
namespace features{

using std::string;
using core::chemical::num_canonical_aas;
using core::chemical::AtomIndices;
using core::pose::Pose;
using core::Size;
using core::Distance;
using core::Vector;
using core::graph::Graph;
using core::conformation::Residue;
using core::scoring::TenANeighborGraph;
using ObjexxFCL::FArray5D;
using utility::sql_database::sessionOP;
using utility::vector1;
using cppdb::statement;

AtomAtomPairFeatures::AtomAtomPairFeatures(){}

AtomAtomPairFeatures::AtomAtomPairFeatures( AtomAtomPairFeatures const & ) :
	FeaturesReporter()
{}

AtomAtomPairFeatures::~AtomAtomPairFeatures(){}

string
AtomAtomPairFeatures::type_name() const { return "AtomAtomPairFeatures"; }

string
AtomAtomPairFeatures::schema() const {
	return
		"CREATE TABLE IF NOT EXISTS atom_pairs (\n"
		"	struct_id INTEGER,\n"
		"	resType1 TEXT,\n"
		"	atmType1 TEXT,\n"
		"	resType2 TEXT,\n"
		"	atmType2 TEXT,\n"
		"	distBin TEXT,\n"
		"	count INTEGER,\n"
		"	FOREIGN KEY (struct_id)\n"
		"		REFERENCES structures (struct_id)\n"
		"		DEFERRABLE INITIALLY DEFERRED,\n"
		"	CONSTRAINT dist_is_nonnegative CHECK (count >= 0),\n"
		"	PRIMARY KEY (struct_id, resType1, atmType1, resType2, atmType2, distBin));";
}
Size
AtomAtomPairFeatures::report_features(
	Pose const & pose,
	vector1< bool > const & relevant_residues,
	Size const struct_id,
	sessionOP db_session
){
	report_atom_pairs(pose, relevant_residues, struct_id, db_session);
	return 0;
}

/// @detail This is very similar in spirit to the potential described in
///
///﻿Lu H, Skolnick J. A distance-dependent atomic knowledge-based potential for improved protein structure selection. Proteins. 2001;44(3):223-32. Available at: http://www.ncbi.nlm.nih.gov/pubmed/11455595.
///
/// However, they use different distance bins.  Here, [0,1), ...,
/// [9,10) are used because they are easy and as they report the the
/// paper, most of the signal comes in the 3.5-6.5 range.  To get the
/// molar fraction of atom types--since the types are unique within
/// each residue type, there is exactly one per residue of that type.
/// Therefore this information can be extracted from the Residues
/// table when needed.  It may make sense to include it here if it
/// turns to to be too cumbersom to get those quantities.
///
/// TODO: Expand for not just canonical residue types


void
AtomAtomPairFeatures::report_atom_pairs(
	Pose const & pose,
	vector1< bool > const & relevant_residues,
	Size const struct_id,
	sessionOP db_session
){

	// assert pose.update_residue_neighbors() has been called:
	runtime_assert(
		 !pose.conformation().structure_moved() &&
		 pose.energies().residue_neighbors_updated());

	Size const max_res(num_canonical_aas);
	Size const max_atm(30); // check this
	Size const dist_bins(15);
	FArray5D< Size > counts;
	counts.dimension(max_res, max_atm, max_res, max_atm, dist_bins, 1);

	TenANeighborGraph const & tenA( pose.energies().tenA_neighbor_graph() );


	for(Size resNum1=1; resNum1 <= pose.total_residue(); ++resNum1){
		Residue res1( pose.residue(resNum1) );
		if(!relevant_residues[resNum1]) continue;

		for ( Graph::EdgeListConstIter
			ir  = tenA.get_node( resNum1 )->const_edge_list_begin(),
			ire = tenA.get_node( resNum1 )->const_edge_list_end();
			ir != ire; ++ir ) {
			Size resNum2( (*ir)->get_other_ind(resNum1) );
			if(!relevant_residues[resNum2]) continue;

			Residue res2( pose.residue(resNum2) );

			for(Size atmNum1=1; atmNum1 <= res1.natoms(); ++atmNum1){
				Vector const & atm1_xyz( res1.xyz(atmNum1) );

				for(Size atmNum2=1; atmNum2 <= res2.natoms(); ++atmNum2){
					Vector const & atm2_xyz( res2.xyz(atmNum2) );

					Size const dist_bin(static_cast<Size>(ceil(atm1_xyz.distance(atm2_xyz))));
					if(dist_bin < 15){
						counts(res1.aa(), atmNum1, res2.aa(), atmNum2, dist_bin) += 1;
					}
				}
			}
		}
	}

	for(Size aa1=1; aa1 <= max_res; ++aa1){
		for(Size aa2=1; aa2 <= max_res; ++aa2){
			for(Size atmNum1=1; atmNum1 <= max_atm; ++atmNum1){
				for(Size atmNum2=1; atmNum2 <= max_atm; ++atmNum2){
					for(Size dist_bin=1; dist_bin <= 15; ++dist_bin){
						Size const count(counts(aa1, atmNum1, aa2, atmNum2, dist_bin));
						statement stmt = (*db_session)
							<< "INSERT INTO atom_pairs VALUES (?,?,?,?,?,?,?);"
							<< struct_id
							<< aa1
							<< atmNum1
							<< aa2
							<< atmNum2
							<< dist_bin
							<< count;
						basic::database::safely_write_to_database(stmt);
					}
				}
			}
		}
	}
}

} // namesapce
} // namespace
