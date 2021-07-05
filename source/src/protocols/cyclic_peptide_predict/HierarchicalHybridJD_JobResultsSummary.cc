// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/cyclic_peptide_predict/HierarchicalHybridJD_JobResultsSummary.cc
/// @brief A small helper class used by the SimpleCycpepPredict_MPI class.
/// @author Vikram K. Mulligan, Baker laboratory (vmullig@uw.edu)

// Unit Headers
#include <protocols/cyclic_peptide_predict/HierarchicalHybridJD_JobResultsSummary.hh>

// Package Headers
#include <core/types.hh>

// option key includes

//numeric headers

// Utility headers
#include <basic/Tracer.hh>

// C++ headers

static basic::Tracer TR( "protocols.cyclic_peptide_predict.HierarchicalHybridJD_JobResultsSummary" );

namespace protocols {
namespace cyclic_peptide_predict {


/// @brief Constructor with options
///
HierarchicalHybridJD_JobResultsSummary::HierarchicalHybridJD_JobResultsSummary(
	int const originating_node_MPI_rank,
	core::Size const jobindex_on_originating_node,
	core::Real const &pose_energy,
	core::Real const &rmsd,
	core::Size const hbonds,
	core::Size const cis_peptide_bonds
) :
	HierarchicalHybridJD_ResultsSummaryBase( originating_node_MPI_rank, jobindex_on_originating_node ),
	pose_energy_( pose_energy ),
	rmsd_(rmsd),
	hbonds_(hbonds),
	cis_peptide_bonds_(cis_peptide_bonds)
{}


/// @brief Clone operator: make a copy of this object and return an OP to the copy.
HierarchicalHybridJD_ResultsSummaryBaseOP
HierarchicalHybridJD_JobResultsSummary::clone() const {
	return utility::pointer::make_shared< HierarchicalHybridJD_JobResultsSummary >( *this );
}

/// @brief Print a summary of this object to a stream.
///
void
HierarchicalHybridJD_JobResultsSummary::show(
	std::ostream & out
) const {
	out << "MPI rank: " << originating_node_MPI_rank() << "\tJob index: " << jobindex_on_originating_node() << "\tEnergy: " << pose_energy_ << "\tRMSD: " << rmsd_ << "\tHbonds: " << hbonds_ << "\tCisPepBonds: " << cis_peptide_bonds_ << "\tHandled_by: ";
	for ( core::Size i=1, imax=MPI_ranks_handling_message().size(); i<=imax; ++i ) {
		out << MPI_ranks_handling_message()[i];
		if ( i<imax ) out << ",";
	}
	out << ".\n" << std::endl;
}


/// @brief Set the energy of the pose generated by this job.
///
void
HierarchicalHybridJD_JobResultsSummary::set_pose_energy(
	core::Real const &val
) {
	pose_energy_ = val;
}

/// @brief Set the RMSD of the pose generated by this job to native.
///
void
HierarchicalHybridJD_JobResultsSummary::set_rmsd(
	core::Real const &val
) {
	rmsd_ = val;
}

/// @brief Set the number of hbonds of the pose generated by this job.
///
void
HierarchicalHybridJD_JobResultsSummary::set_hbonds(
	core::Size const val
) {
	hbonds_ = val;
}

/// @brief Set the number of cis-peptide bonds of the pose generated by this job.
///
void
HierarchicalHybridJD_JobResultsSummary::set_cis_peptide_bonds(
	core::Size const val
) {
	cis_peptide_bonds_ = val;
}


} //cyclic_peptide_predict
} //protocols

