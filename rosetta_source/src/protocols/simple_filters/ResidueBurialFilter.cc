// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/simple_filters/ResidueBurialFilter.cc
/// @brief
/// @author Sarel Fleishman (sarelf@u.washington.edu), Jacob Corn (jecorn@u.washington.edu)

#include <protocols/simple_filters/ResidueBurialFilter.hh>
#include <protocols/simple_filters/ResidueBurialFilterCreator.hh>

#include <core/types.hh>
#include <core/pose/Pose.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Conformation.hh>
#include <utility/tag/Tag.hh>
#include <protocols/moves/DataMap.hh>
#include <protocols/rosetta_scripts/util.hh>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <basic/Tracer.hh>
#include <core/pack/task/TaskFactory.hh>

namespace protocols{
namespace simple_filters {

static basic::Tracer residue_burial_filter_tracer( "protocols.simple_filters.ResidueBurialFilter" );

protocols::filters::FilterOP
ResidueBurialFilterCreator::create_filter() const { return new ResidueBurialFilter; }

std::string
ResidueBurialFilterCreator::keyname() const { return "ResidueBurial"; }

ResidueBurialFilter::~ResidueBurialFilter(){}

void
ResidueBurialFilter::task_factory( core::pack::task::TaskFactoryOP tf ){
	task_factory_ = tf;
}

core::pack::task::TaskFactoryOP
ResidueBurialFilter::task_factory() const{ return task_factory_; }

bool
ResidueBurialFilter::apply( core::pose::Pose const & pose ) const {
	if( task_factory() ){//taskfactory is on, iterate over all designable residues and check whether any one passes the filter
/// This looks like recursion but is really quite limited, b/c the call below to rbf.apply uses the functionality where task_factory() is off, so it goes by a different route, and could never go to a depth of more than 2
		utility::vector1< core::Size > const target_residues( protocols::rosetta_scripts::residue_packer_states( pose, task_factory(), true/*designable*/, false/*packable*/ ) );
		foreach( core::Size const resi, target_residues ){
			ResidueBurialFilter const rbf( resi/*target_residue*/, neighbors_, distance_threshold_ );
			if( rbf.apply( pose ) )
				return true;
		}
		return false;
	}

	core::Size const count_neighbors( compute( pose ) );

	residue_burial_filter_tracer<<"Number of interface neighbors of residue "<<pose.residue( target_residue_ ).name3()<<target_residue_<<" is "<<count_neighbors<<std::endl;
	return( count_neighbors >= neighbors_ );
}

void
ResidueBurialFilter::parse_my_tag( utility::tag::TagPtr const tag, moves::DataMap & data, filters::Filters_map const &, moves::Movers_map const &, core::pose::Pose const & pose )
{
	if( tag->hasOption( "res_num" ) || tag->hasOption( "pdb_num" ) )
		target_residue_ = protocols::rosetta_scripts::get_resnum( tag, pose );
	distance_threshold_ = tag->getOption<core::Real>( "distance", 8.0 );
	neighbors_ = tag->getOption<core::Size>( "neighbors", 1 );
	task_factory( protocols::rosetta_scripts::parse_task_operations( tag, data ) );

	residue_burial_filter_tracer<<"ResidueBurialFilter with distance threshold of "<<distance_threshold_<<" around residue "<<target_residue_<<" with "<<neighbors_<<" neighbors."<<std::endl;
}

void
ResidueBurialFilter::report( std::ostream & out, core::pose::Pose const & pose ) const {
	if( !task_factory() ){
		core::Size const count_neighbors( compute( pose ) );

		out<<"Number of interface neighbors of residue "<<pose.residue( target_residue_ ).name3()<<target_residue_<<" is "<<count_neighbors<<'\n';
	}
}

core::Real
ResidueBurialFilter::report_sm( core::pose::Pose const & pose ) const {
	if( !task_factory() ){
		core::Size const count_neighbors( compute( pose ) );

		return( count_neighbors );
	}
	else return( 0 );
}

/// @details counts the number of residues to target_residue_ across all chains in the pose, other than the one containing target_residue_
core::Size
ResidueBurialFilter::compute( core::pose::Pose const & pose ) const {
	core::Size chain( 1 );
	for( ; chain <= pose.conformation().num_chains(); ++chain )
		if( pose.conformation().chain_begin( chain ) <= target_residue_ && pose.conformation().chain_end( chain ) >= target_residue_ ) break;

	core::Size const chain_begin( pose.conformation().chain_begin( chain ) );
	core::Size const chain_end( pose.conformation().chain_end( chain ) );

residue_burial_filter_tracer<<"chain span "<<chain_begin<< " "<<chain_end<<std::endl;
	core::Size count_neighbors( 0 );
	core::conformation::Residue const res_target( pose.conformation().residue( target_residue_ ) );
	for( core::Size i=1; i<=pose.total_residue(); ++i ){
		if( i>=chain_begin && i<=chain_end ) continue;
		core::conformation::Residue const resi( pose.residue( i ) );
		core::Real const distance( resi.xyz( resi.nbr_atom() ).distance( res_target.xyz( res_target.nbr_atom() ) ) );
		if( distance <= distance_threshold_ ) ++count_neighbors;
	}
	return( count_neighbors);
}

}
}
