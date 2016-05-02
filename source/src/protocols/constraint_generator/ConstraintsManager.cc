// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/constraint_generator/ConstraintsManager.cc
/// @brief Manages lists of constraints generated by ConstraintGenerators
/// @author Tom Linsky (tlinsky@uw.edu)

#include <protocols/constraint_generator/ConstraintsManager.hh>
#include <basic/Tracer.hh>

static THREAD_LOCAL basic::Tracer TR( "protocols.constraint_generator.ConstraintsManager" );


namespace protocols {
namespace constraint_generator {

ConstraintsManager::ConstraintsManager():
	utility::SingletonBase< ConstraintsManager >(),
	cst_map_()
{
}

ConstraintsManager::~ConstraintsManager(){}

ConstraintsManager *
ConstraintsManager::create_singleton_instance()
{
	return new ConstraintsManager;
}

bool
ConstraintsManager::constraints_exist( std::string const & name ) const
{
	return ( cst_map_.find( name ) != cst_map_.end() );
}

core::scoring::constraints::ConstraintCOPs const &
ConstraintsManager::retrieve_constraints( std::string const & name ) const
{
	ConstraintsMap::const_iterator csts = cst_map_.find( name );
	if ( csts == cst_map_.end() ) {
		std::stringstream msg;
		msg << "ConstraintsManager::retrieve_constraints(): Tried to access a stored constraint set that doesn't exist: '" << name << "'" << std::endl;
		print_valid_names( msg );
		utility_exit_with_message( msg.str() );
	}
	return csts->second;
}

void
ConstraintsManager::remove_constraints( std::string const & name )
{
	ConstraintsMap::iterator csts = cst_map_.find( name );
	if ( csts == cst_map_.end() ) {
		std::stringstream msg;
		msg << "ConstraintsManager:retrieve_constraints(): Tried to remove a stored constraint set that doesn't exist: '" << name << "'" << std::endl;
		print_valid_names( msg );
		utility_exit_with_message( msg.str() );
	}
	cst_map_.erase( csts );
}

void
ConstraintsManager::store_constraints( std::string const & name, core::scoring::constraints::ConstraintCOPs const & csts )
{
	cst_map_[ name ] = csts;
}

void
ConstraintsManager::print_valid_names( std::ostream & os ) const
{
	os << "Valid constraint set names are: ";
	for ( ConstraintsMap::const_iterator c=cst_map_.begin(); c!=cst_map_.end(); ++c ) {
		os << c->first << " ";
	}
	os << std::endl;
}

} //protocols
} //constraint_generator

// Singleton instance and mutex static data members
namespace utility {

using protocols::constraint_generator::ConstraintsManager;

#if defined MULTI_THREADED && defined CXX11
template<> std::mutex utility::SingletonBase< ConstraintsManager >::singleton_mutex_{};
template<> std::atomic< ConstraintsManager * > utility::SingletonBase< ConstraintsManager >::instance_( NULL );
#else
template<> ConstraintsManager * utility::SingletonBase< ConstraintsManager >::instance_( NULL );
#endif

} // namespace utility
