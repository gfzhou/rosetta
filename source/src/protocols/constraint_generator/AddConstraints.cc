// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/constraint_generator/AddConstraints.cc
/// @brief Adds constraints generated by ConstraintGenerators to a pose
/// @author Tom Linsky (tlinsky@uw.edu)

#include <protocols/constraint_generator/AddConstraints.hh>
#include <protocols/constraint_generator/AddConstraintsCreator.hh>

// Protocol headers
#include <protocols/constraint_generator/ConstraintGenerator.hh>
#include <protocols/constraint_generator/ConstraintGeneratorFactory.hh>

// Core headers
#include <core/pose/Pose.hh>

// Basic/Utility headers
#include <basic/datacache/DataMap.hh>
#include <basic/Tracer.hh>
#include <utility/tag/Tag.hh>

static THREAD_LOCAL basic::Tracer TR( "protocols.constraint_generator.AddConstraints" );

namespace protocols {
namespace constraint_generator {

AddConstraints::AddConstraints():
	protocols::moves::Mover( AddConstraintsCreator::mover_name() ),
	generators_()
{
}

AddConstraints::AddConstraints( ConstraintGeneratorCOPs const & generators ):
	protocols::moves::Mover( AddConstraintsCreator::mover_name() ),
	generators_( generators )
{
}

AddConstraints::~AddConstraints(){}

void
AddConstraints::parse_my_tag(
	utility::tag::TagCOP tag,
	basic::datacache::DataMap & data,
	protocols::filters::Filters_map const & ,
	protocols::moves::Movers_map const & ,
	core::pose::Pose const & )
{
	for ( utility::tag::Tag::tags_t::const_iterator subtag=tag->getTags().begin(); subtag!=tag->getTags().end(); ++subtag ) {
		ConstraintGeneratorOP new_cg = ConstraintGeneratorFactory::get_instance()->new_constraint_generator( (*subtag)->getName(), *subtag, data );
		add_generator( new_cg );
		TR << "Added constraint generator " << new_cg->id() << "." << std::endl;
		data.add( "ConstraintGenerators", new_cg->id(), new_cg );
	}
}

protocols::moves::MoverOP
AddConstraints::clone() const
{
	return protocols::moves::MoverOP( new AddConstraints( *this ) );
}

protocols::moves::MoverOP
AddConstraints::fresh_instance() const
{
	return protocols::moves::MoverOP( new AddConstraints );
}

std::string
AddConstraints::get_name() const
{
	return AddConstraintsCreator::mover_name();
}

void
AddConstraints::apply( core::pose::Pose & pose )
{
	for ( ConstraintGeneratorCOPs::const_iterator cg=generators_.begin(); cg!=generators_.end(); ++cg ) {
		debug_assert( *cg );
		core::scoring::constraints::ConstraintCOPs const csts = (*cg)->apply( pose );
		(*cg)->clear_stored_constraints();
		(*cg)->store_constraints( csts );

		if ( csts.size() == 0 ) continue;

		TR << "Adding " << csts.size() << " constraints generated by ConstraintGenerator named " << (*cg)->id() << std::endl;

		pose.add_constraints( csts );
	}
}

void
AddConstraints::add_generator( ConstraintGeneratorCOP cst_generator )
{
	generators_.push_back( cst_generator );
}

/////////////// Creator ///////////////

protocols::moves::MoverOP
AddConstraintsCreator::create_mover() const
{
	return protocols::moves::MoverOP( new AddConstraints );
}

std::string
AddConstraintsCreator::keyname() const
{
	return AddConstraintsCreator::mover_name();
}

std::string
AddConstraintsCreator::mover_name()
{
	return "AddConstraints";
}

} //protocols
} //constraint_generator

