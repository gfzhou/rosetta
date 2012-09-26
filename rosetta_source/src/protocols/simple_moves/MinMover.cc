// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file MinMover.cc
/// @brief
/// @author ashworth

// Unit headers
#include <protocols/simple_moves/MinMover.hh>
#include <protocols/simple_moves/MinMoverCreator.hh>

#include <protocols/moves/DataMap.hh>

// Package headers

#include <core/kinematics/MoveMap.hh>
#include <core/optimization/AtomTreeMinimizer.hh>
#include <core/optimization/MinimizerOptions.hh>
#include <core/optimization/CartesianMinimizer.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh> // getScoreFunction
#include <core/pose/Pose.fwd.hh>
// AUTO-REMOVED #include <core/pose/Pose.hh>
#include <basic/prof.hh>
#include <utility/string_util.hh>
#include <utility/tag/Tag.hh>
#include <iostream>
#include <protocols/rosetta_scripts/util.hh>
#include <protocols/elscripts/util.hh>

// Boost Headers
#include <boost/foreach.hpp>
#include <basic/Tracer.hh>

#include <utility/vector0.hh>
#include <utility/vector1.hh>

#define foreach BOOST_FOREACH

namespace protocols {
namespace simple_moves {

using namespace core;
using namespace kinematics;
using namespace optimization;
using namespace scoring;

static basic::Tracer TR("protocols.simple_moves.MinMover");

std::string
MinMoverCreator::keyname() const
{
	return MinMoverCreator::mover_name();
}

protocols::moves::MoverOP
MinMoverCreator::create_mover() const {
	return new MinMover;
}

std::string
MinMoverCreator::mover_name()
{
	return "MinMover";
}

// default constructor
// proper lightweight default constructor
MinMover::MinMover()
	: protocols::moves::Mover("MinMover"),
		movemap_(0),
		scorefxn_(0),
		min_options_(0),
		cartesian_(false)
		//		threshold_(1000000.0) // TODO: line can be deleted?
{
	min_options_ = new MinimizerOptions( "linmin", 0.01, true, false, false );
}

MinMover::MinMover( std::string const & name )
	: protocols::moves::Mover(name),
		movemap_(0),
		scorefxn_(0),
		min_options_(0),
		cartesian_(false)
		//		threshold_(1000000.0) // TODO: line can be deleted?
{
	min_options_ = new MinimizerOptions( "linmin", 0.01, true, false, false );
}

MinMover::~MinMover(){}

// constructor with arguments
MinMover::MinMover(
	MoveMapOP movemap_in,
	ScoreFunctionCOP scorefxn_in,
	std::string const & min_type_in,
	Real tolerance_in,
	bool use_nb_list_in,
	bool deriv_check_in /* = false */,
	bool deriv_check_verbose_in /* = false */
) : protocols::moves::Mover("MinMover"),
		movemap_( movemap_in ),
		scorefxn_( scorefxn_in ),
		min_options_(0),
		threshold_(1000000.0), // TODO: line can be deleted?
		cartesian_(false)
{
	min_options_ = new MinimizerOptions(
		min_type_in, tolerance_in, use_nb_list_in, deriv_check_in, deriv_check_verbose_in );
}

/// @brief allow non-const access to the internal minimizer options object
MinimizerOptionsOP
MinMover::min_options() {
	return min_options_;
}

/// @brief allow const access to the internal minimizer options object
MinimizerOptionsCOP
MinMover::min_options() const {
	return min_options_;
}

void
MinMover::movemap( MoveMapCOP movemap_in )
{
	runtime_assert( movemap_in );
	movemap_ = new MoveMap( *movemap_in );
}

MoveMapCOP
MinMover::movemap() const
{
	return movemap_;
}

void
MinMover::score_function( ScoreFunctionCOP scorefxn_in )
{
	runtime_assert( scorefxn_in );
	scorefxn_ = scorefxn_in;
}

void
MinMover::score_function( ScoreFunction const & scorefxn_in )
{
	scorefxn_ = scorefxn_in.clone();
}

ScoreFunctionCOP
MinMover::score_function() const
{
	return scorefxn_;
}

void MinMover::min_type( std::string min_type_in ) { min_options_->min_type( min_type_in ); }
std::string MinMover::min_type() const { return min_options_->min_type(); }


void MinMover::tolerance( Real tolerance_in ) { min_options_->minimize_tolerance( tolerance_in ); }
Real MinMover::tolerance() const { return min_options_->minimize_tolerance(); }


void MinMover::nb_list( bool nb_list_in ) { min_options_->use_nblist( nb_list_in ); }
bool MinMover::nb_list() const { return min_options_->use_nblist(); }


void MinMover::deriv_check( bool deriv_check_in ) { min_options_->deriv_check( deriv_check_in ); }
bool MinMover::deriv_check() const { return min_options_->deriv_check(); }

void
MinMover::apply( pose::Pose & pose_ )
{
	// lazy default initialization
	if ( ! movemap_ ) movemap_ = new MoveMap;
	if ( ! scorefxn_ ) scorefxn_ = getScoreFunction(); // get a default (INITIALIZED!) ScoreFunction

	PROF_START( basic::MINMOVER_APPLY );
	if (!cartesian( )) {
		AtomTreeMinimizer minimizer;
		(*scorefxn_)(pose_);
		minimizer.run( pose_, *movemap_, *scorefxn_, *min_options_ );
	} else {
		CartesianMinimizer minimizer;
		(*scorefxn_)(pose_);
		minimizer.run( pose_, *movemap_, *scorefxn_, *min_options_ );
	}
	PROF_STOP( basic::MINMOVER_APPLY );

  // emit statistics
  scorefxn_->show(TR.Debug, pose_);
  TR.Debug << std::endl;
}

std::string
MinMover::get_name() const {
	return MinMoverCreator::mover_name();
}

protocols::moves::MoverOP MinMover::clone() const { return new protocols::simple_moves::MinMover( *this ); }
protocols::moves::MoverOP MinMover::fresh_instance() const { return new MinMover; }

void MinMover::parse_def_opts( utility::lua::LuaObject const & def,
	utility::lua::LuaObject const & score_fxns,
	utility::lua::LuaObject const & tasks,
	protocols::moves::MoverCacheSP cache ) {
	if( def["scorefxn"] ) {
		score_function( protocols::elscripts::parse_scoredef( def["scorefxn"], score_fxns ) );
	} else {
		score_function( score_fxns["score12"].to<ScoreFunctionSP>()->clone()  );
	}

	if ( ! movemap_ ) movemap_ = new MoveMap;
	if( def["jump"] ) {
		for (utility::lua::LuaIterator i=def["jump"].begin(), end; i != end; ++i) {
			if( (*i).to<int>() == -1 ) {
				movemap_->set_jump( true );
				break;
			} else if( (*i).to<core::Size>() == 0 ) {
				movemap_->set_jump( false );
				break;
			} else {
				TR << "Setting min on jump " << (*i).to<core::Size>() << std::endl;
				movemap_->set_jump( (*i).to<core::Size>(), true );
			}
		}
	}

	min_type( def["type"] ? def[ "type" ].to<std::string>() : "dfpmin_armijo_nonmonotone" );
	tolerance( def["tolerance"] ? def[ "tolerance" ].to<core::Real>() : 0.01 );
	cartesian( def["cartesian"] ? def[ "cartesian" ].to<bool>() : false );
	//fpd if cartesian default to lbfgs minimization
	if ( cartesian() && ! def["type"] ) {
		min_type( "lbfgs_armijo_nonmonotone" );
	}
}

void MinMover::parse_def( utility::lua::LuaObject const & def,
				utility::lua::LuaObject const & score_fxns,
				utility::lua::LuaObject const & tasks,
				protocols::moves::MoverCacheSP cache ) {
	if ( ! movemap_ ) movemap_ = new MoveMap;

	parse_def_opts( def, score_fxns, tasks, cache );

	if( def["chi"] )
		movemap_->set_chi(def["chi"].to<bool>());
	if( def["bb"] )
		movemap_->set_chi(def["bb"].to<bool>());


	if( def["movemap"] )
		protocols::elscripts::parse_movemapdef( def["movemap"], movemap_ );
}

void MinMover::parse_my_tag(
	TagPtr const tag,
	protocols::moves::DataMap & data,
	Filters_map const & filters,
	protocols::moves::Movers_map const & movers,
	Pose const & pose )
{
	if ( ! movemap_ ) movemap_ = new MoveMap;
	parse_opts( tag, data, filters, movers, pose );
	parse_chi_and_bb( tag );

/// parse_movemap will reset the movemap to minimize all if nothing is stated on it. The following section ensures that the protocol specifies a MoveMap before going that way
//	utility::vector1< TagPtr > const branch_tags( tag->getTags() );
//	utility::vector1< TagPtr >::const_iterator tag_it;
	protocols::rosetta_scripts::parse_movemap( tag, pose, movemap_, data, false );
}

void MinMover::parse_opts(
	TagPtr const tag,
	protocols::moves::DataMap & data,
	Filters_map const &,
	protocols::moves::Movers_map const &,
	Pose const & )
{
	std::string const scorefxn_name( tag->getOption< std::string >( "scorefxn", "score12" ) );
	score_function( data.get< ScoreFunction * >( "scorefxns", scorefxn_name ) );
	if ( tag->hasOption("jump") ) {
		if ( ! movemap_ ) movemap_ = new MoveMap;
		utility::vector1<std::string> jumps = utility::string_split( tag->getOption<std::string>( "jump" ), ',' );
		// string 'ALL' makes all jumps movable
		if (jumps.size() == 1 && (jumps[1] == "ALL" || jumps[1] == "All" || jumps[1] == "all" || jumps[1] == "*") ) {
			movemap_->set_jump( true );
		} else if( tag->getOption< core::Size > ( "jump" ) == 0 ) {
			movemap_->set_jump( false );
		} else {
			foreach(std::string jump, jumps){
				Size const value = std::atoi( jump.c_str() ); // convert to C string, then convert to integer, then set a Size (phew!)
				TR << "Setting min on jump " << value << std::endl;
				movemap_->set_jump( value, true );
			}
		}
	}
	min_type( tag->getOption< std::string >( "type", "dfpmin_armijo_nonmonotone" ) );
	tolerance( tag->getOption< core::Real >( "tolerance", 0.01 ) );

	cartesian( tag->getOption< core::Real >( "cartesian", false ) );

	//fpd if cartesian default to lbfgs minimization
	if ( cartesian() && !tag->hasOption("type") ) {
		min_type( "lbfgs_armijo_nonmonotone" );
	}
}

void MinMover::parse_chi_and_bb( TagPtr const tag )
{
	if ( ! movemap_ ) movemap_ = new MoveMap;
	bool const chi( tag->getOption< bool >( "chi" ) ), bb( tag->getOption< bool >( "bb" ) );
	movemap_->set_chi( chi );
	movemap_->set_bb( bb );
	TR<<"Options chi, bb: "<<chi<<", "<<bb<<std::endl;
	if ( tag->hasOption("bondangle") ) {
		bool const value( tag->getOption<bool>("bondangle") );
		movemap_->set( core::id::THETA, value );
	}
	if ( tag->hasOption("bondlength") ) {
		bool const value( tag->getOption<bool>("bondlength") );
		movemap_->set( core::id::D, value );
	}
}


std::ostream &operator<< (std::ostream &os, MinMover const &mover)
{
	moves::operator<<(os, mover);
	os << "Minimization type:\t" << mover.min_type() << "\nScorefunction:\t\t";
	if ( mover.score_function() != 0 ) {
		os  << mover.score_function()->get_name() << std::endl;
	}
	else { os << "none" << std::endl; }
	os << "Score tolerance:\t" << mover.tolerance() << "\nNb list:\t\t" << (mover.nb_list() ? "True" : "False") << 
			"\nDeriv check:\t\t" << (mover.deriv_check() ? "True" : "False") << std::endl << "Movemap:" << std::endl;
	if (mover.movemap() != 0) {
		mover.movemap()->show();
	}
	return os;
}


} // moves
} // protocols
