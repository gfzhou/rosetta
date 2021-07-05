// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/energy_methods/CovalentLabelingLabelingEnergy.cc
/// @author Melanie Aprahamian (aprahamian.4@osu.edu)

#include <core/energy_methods/CovalentLabelingEnergy.hh>
#include <core/energy_methods/CovalentLabelingEnergyCreator.hh>
#include <core/conformation/Residue.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/EnergyMap.hh>
#include <core/scoring/ContextGraphTypes.hh>
#include <core/scoring/methods/EnergyMethodOptions.hh>


#include <utility/io/izstream.hh>
#include <utility/vector1.hh>
#include <numeric/NumericTraits.hh>

#include <basic/options/keys/OptionKeys.hh> // AUTO IWYU For OptionKeys,

namespace core {
namespace energy_methods {


/// @details This must return a fresh instance of the CovalentLabelingEnergy class,
/// never an instance already in use
core::scoring::methods::EnergyMethodOP
CovalentLabelingEnergyCreator::create_energy_method(
	core::scoring::methods::EnergyMethodOptions const & options
) const {
	return utility::pointer::make_shared< CovalentLabelingEnergy >( options );
}

core::scoring::ScoreTypes
CovalentLabelingEnergyCreator::score_types_for_method() const {
	using namespace core::scoring;
	ScoreTypes sts;
	sts.push_back(covalent_labeling);
	return sts;
}

void
CovalentLabelingEnergy::setup_for_scoring(
	pose::Pose & pose, core::scoring::ScoreFunction const &
) const {
	pose.update_residue_neighbors();

}

// constructor
CovalentLabelingEnergy::CovalentLabelingEnergy( core::scoring::methods::EnergyMethodOptions const & options ) :
	parent( utility::pointer::make_shared< CovalentLabelingEnergyCreator >() ),
	covalent_labeling_input_file_( options.covalent_labeling_input() )
{
	init_from_file();
}

CovalentLabelingEnergy::CovalentLabelingEnergy( CovalentLabelingEnergy const & src ):
	parent( src ),
	covalent_labeling_input_file_( src.covalent_labeling_input_file_ )
{
	init_from_file();
}

/// clone
core::scoring::methods::EnergyMethodOP
CovalentLabelingEnergy::clone() const {
	return utility::pointer::make_shared< CovalentLabelingEnergy >( *this );
}

/////////////////////////////////////////////////////////////////////////////
// scoring
/////////////////////////////////////////////////////////////////////////////

void
CovalentLabelingEnergy::residue_energy(
	core::conformation::Residue const & residue,
	core::pose::Pose const & pose,
	core::scoring::EnergyMap & emap
) const {

	core::Size const number_residues (pose.total_residue());
	core::Real neighbor_count (0.0);
	core::Real nc_from_file (0.0);
	core::Real distance (0.0);
	core::Real angle (0.0);

	for ( core::Size j=1; j <= input_nc_.size(); j++ ) {
		if ( input_nc_[j].first == residue.seqpos() ) {
			nc_from_file = input_nc_[j].second;
			for ( core::Size res_count_neighbor = 1; res_count_neighbor <= number_residues; res_count_neighbor++ ) {
				if ( residue.seqpos() != pose.residue(res_count_neighbor).seqpos() ) {
					distance = residue.xyz("CA").distance(pose.residue(res_count_neighbor).xyz("CEN"));
					numeric::xyzVector<core::Real> norm_CA_CEN_vector = (residue.xyz("CEN") - residue.xyz("CA"))/(residue.xyz("CEN").distance(residue.xyz("CA")));
					numeric::xyzVector<core::Real> norm_neighbor_vector = (pose.residue(res_count_neighbor).xyz("CEN")-residue.xyz("CA"))/(pose.residue(res_count_neighbor).xyz("CEN").distance(residue.xyz("CA")));
					angle = std::acos(norm_CA_CEN_vector.dot(norm_neighbor_vector));
					neighbor_count += 1.0/(1.0 + std::exp(1.0*(distance-9.0)))*1.0/(1.0 + std::exp(numeric::NumericTraits< float >::pi()*2.0*(angle-numeric::NumericTraits< float >::pi())));
				}
			}
			emap[ core::scoring::covalent_labeling] += -1.0/(1.0 + std::exp(10.0*(std::abs(neighbor_count - nc_from_file)-2.0)));
		}
	}
}

void
CovalentLabelingEnergy::indicate_required_context_graphs( utility::vector1< bool > & context_graphs_required ) const
{
	context_graphs_required[ core::scoring::twelve_A_neighbor_graph ] = false;
}


core::Size
CovalentLabelingEnergy::version() const
{
	return 1;
}

void CovalentLabelingEnergy::init_from_file() {
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	utility::io::izstream input(covalent_labeling_input_file_);
	if ( !input.good() ) {
		std::string const msg( "Error opening file: " + covalent_labeling_input_file_ );
		utility_exit_with_message( msg );
	}

	std::string line;
	getline(input,line);

	while ( getline(input,line) ) {
		std::istringstream ss(line);
		core::Size resi;
		core::Real nc;

		ss >> resi >> nc;

		input_nc_.push_back( std::pair< core::Size, core::Real >(resi, nc) );
	}

}

} // scoring
} // core
