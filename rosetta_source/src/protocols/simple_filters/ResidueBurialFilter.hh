// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/simple_filters/ResidueBurialFilter.hh
/// @brief definition of filter class ResidueBurialFilter.
/// @author Sarel Fleishman (sarelf@u.washington.edu), Jacob Corn (jecorn@u.washington.edu)

#ifndef INCLUDED_protocols_simple_filters_ResidueBurialFilter_hh
#define INCLUDED_protocols_simple_filters_ResidueBurialFilter_hh

#include <protocols/simple_filters/ResidueBurialFilter.fwd.hh>
#include <core/types.hh>
#include <protocols/filters/Filter.hh>
#include <utility/tag/Tag.fwd.hh>
#include <protocols/moves/DataMap.fwd.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <core/pack/task/TaskFactory.fwd.hh>

namespace protocols {
namespace simple_filters {

class ResidueBurialFilter : public filters::Filter
{
public:
	ResidueBurialFilter() : Filter( "ResidueBurial"  ), target_residue_( 0 ), neighbors_( 1 ), distance_threshold_( 8.0 ), task_factory_( NULL ) {}
	ResidueBurialFilter( core::Size const target_residue, core::Size const neighbors, core::Real const distance_threshold ) :
		Filter( "ResidueBurial" ), target_residue_( target_residue ), neighbors_( neighbors ), distance_threshold_( distance_threshold ), task_factory_( NULL ) {}
	bool apply( core::pose::Pose const & pose ) const;
	void report( std::ostream & out, core::pose::Pose const & pose ) const;
	core::Real report_sm( core::pose::Pose const & pose ) const;
	core::Size compute( core::pose::Pose const & pose ) const;
	filters::FilterOP clone() const {
		return new ResidueBurialFilter( *this );
	}
	filters::FilterOP fresh_instance() const{
		return new ResidueBurialFilter();
	}

	core::pack::task::TaskFactoryOP task_factory() const;
	void task_factory( core::pack::task::TaskFactoryOP tf );

	virtual ~ResidueBurialFilter();
	void parse_my_tag( utility::tag::TagPtr const tag, protocols::moves::DataMap &, filters::Filters_map const &, protocols::moves::Movers_map const &, core::pose::Pose const & );
private:
	core::Size target_residue_;
	core::Size neighbors_;
	core::Real distance_threshold_;
	core::pack::task::TaskFactoryOP task_factory_; /// used to determine which residues to check for burial dynamically. All designable residues will be checked, and if any of them is buried, returns true

};

}
}

#endif
