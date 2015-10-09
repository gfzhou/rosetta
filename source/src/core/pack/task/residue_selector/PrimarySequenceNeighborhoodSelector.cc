// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/pack/task/residue_selector/PrimarySequenceNeighborhoodSelector.hh
/// @brief  The PrimarySequenceNeighborhoodSelector selects residues using a string containing residue names
/// @author Tom Linsky (tlinsky@uw.edu))

// Unit headers
#include <core/pack/task/residue_selector/PrimarySequenceNeighborhoodSelector.hh>
#include <core/pack/task/residue_selector/PrimarySequenceNeighborhoodSelectorCreator.hh>

// Protocol Headers

// Core headers
#include <core/conformation/Residue.hh>
#include <core/pack/task/residue_selector/ResidueSelectorFactory.hh>
#include <core/pose/selection.hh>
#include <core/pose/util.hh>

// Basic Headers
#include <basic/datacache/DataMap.hh>
#include <basic/Tracer.hh>

// Utility Headers
#include <utility/tag/Tag.hh>

// C++ headers
#include <utility/assert.hh>

static THREAD_LOCAL basic::Tracer TR( "core.pack.task.residue_selector.PrimarySequenceNeighborhoodSelector" );

namespace core {
namespace pack {
namespace task {
namespace residue_selector {

PrimarySequenceNeighborhoodSelector::PrimarySequenceNeighborhoodSelector() :
	lower_residues_( 1 ),
	upper_residues_( 1 ),
	selector_()
{
}

PrimarySequenceNeighborhoodSelector::PrimarySequenceNeighborhoodSelector(
	core::Size const lower_residues,
	core::Size const upper_residues,
	core::pack::task::residue_selector::ResidueSelectorCOP const selector ) :
	lower_residues_( lower_residues ),
	upper_residues_( upper_residues ),
	selector_( selector )
{
}


PrimarySequenceNeighborhoodSelector::~PrimarySequenceNeighborhoodSelector() {}

ResidueSubset
PrimarySequenceNeighborhoodSelector::apply( core::pose::Pose const & pose ) const
{
	runtime_assert( selector_ );
	ResidueSubset subset = selector_->apply( pose );
	debug_assert( subset.size() == pose.total_residue() );

	typedef std::set< std::pair< core::Size, core::Size > > IntervalSet;
	IntervalSet intervals;
	core::Size intervalstart = 0;
	for ( core::Size i=1; i<=subset.size(); ++i ) {
		if ( subset[i] && !intervalstart ) {
			intervalstart = i;
		}
		if ( !subset[i] && intervalstart ) {
			intervals.insert( std::make_pair( intervalstart, i-1 ) );
			intervalstart = 0;
		}
	}

	core::pack::task::residue_selector::ResidueSubset retval( pose.total_residue(), false );
	TR << "Intervals: [";
	for ( IntervalSet::const_iterator i = intervals.begin(); i != intervals.end(); ++i ) {
		TR << " " << i->first << "->" << i->second << ",";
		core::Size start = i->first;
		core::Size end = i->second;
		core::Size count = 0;
		while ( ( count < lower_residues_ ) && !core::pose::is_lower_terminus( pose, start ) ) {
			++count;
			--start;
		}
		TR.Debug << count << " residues added to lower terminus" << std::endl;
		count = 0;
		while ( ( count < upper_residues_ ) && !core::pose::is_upper_terminus( pose, end ) ) {
			++count;
			++end;
		}
		TR.Debug << count << " residues added to upper terminus" << std::endl;
		for ( core::Size r=start; r<=end; ++r ) {
			subset[r] = true;
		}
	}
	TR << "]" << std::endl;
	return subset;
}

void
PrimarySequenceNeighborhoodSelector::parse_my_tag(
	utility::tag::TagCOP tag,
	basic::datacache::DataMap & data )
{
	if ( tag->hasOption( "lower" ) ) {
		set_lower_residues( tag->getOption< core::Size >( "lower" ) );
	}

	if ( tag->hasOption( "upper" ) ) {
		set_upper_residues( tag->getOption< core::Size >( "upper" ) );
	}

	if ( tag->hasOption( "selector" ) ) {
		std::string const selectorname = tag->getOption< std::string >( "selector" );
		try {
			set_selector( data.get_ptr< core::pack::task::residue_selector::ResidueSelector const >( "ResidueSelector", selectorname ) );
		} catch ( utility::excn::EXCN_Msg_Exception e ) {
			std::stringstream error_msg;
			error_msg << "Failed to find ResidueSelector named '" << selectorname << "' from the Datamap from DisulfidizeMover.\n";
			error_msg << e.msg();
			throw utility::excn::EXCN_RosettaScriptsOption( error_msg.str() );
		}
		debug_assert( selector_ );
		TR << "Using residue selector " << selectorname << std::endl;
	} else if ( tag->getTags().size() ) {
		for ( utility::vector0< utility::tag::TagCOP >::const_iterator t = tag->getTags().begin();
				t != tag->getTags().end(); ++t ) {
			ResidueSelectorCOP rs = ResidueSelectorFactory::get_instance()->new_residue_selector(
				(*t)->getName(),
				(*t),
				data );
			if ( rs ) {
				debug_assert( !selector_ );
				set_selector( rs );
			}
		}
	} else {
		std::stringstream ss;
		ss << "PrimarySequenceNeighborhood requires a selector to be set either through the \"selector\" option or by placing a residue selector as a subtag." << std::endl;
		throw utility::excn::EXCN_RosettaScriptsOption( ss.str() );
	}
	debug_assert( selector_ );
}

void
PrimarySequenceNeighborhoodSelector::set_selector(
	core::pack::task::residue_selector::ResidueSelectorCOP const selector )
{
	selector_ = selector;
}

void
PrimarySequenceNeighborhoodSelector::set_lower_residues( core::Size const nres )
{
	lower_residues_ = nres;
}

void
PrimarySequenceNeighborhoodSelector::set_upper_residues( core::Size const nres )
{
	upper_residues_ = nres;
}

std::string PrimarySequenceNeighborhoodSelector::get_name() const {
	return PrimarySequenceNeighborhoodSelector::class_name();
}

std::string PrimarySequenceNeighborhoodSelector::class_name() {
	return "PrimarySequenceNeighborhood";
}

ResidueSelectorOP
PrimarySequenceNeighborhoodSelectorCreator::create_residue_selector() const {
	return ResidueSelectorOP( new PrimarySequenceNeighborhoodSelector );
}

std::string
PrimarySequenceNeighborhoodSelectorCreator::keyname() const {
	return PrimarySequenceNeighborhoodSelector::class_name();
}

} //namespace residue_selector
} //namespace task
} //namespace pack
} //namespace core

