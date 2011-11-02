// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/moves/MoverFactory.hh
/// @brief
/// @author ashworth

#ifndef INCLUDED_protocols_moves_MoverFactory_hh
#define INCLUDED_protocols_moves_MoverFactory_hh

// Unit Headers
#include <protocols/moves/MoverFactory.fwd.hh>
#include <protocols/moves/MoverCreator.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <protocols/moves/DataMap.fwd.hh>
#include <protocols/filters/Filter.fwd.hh> // Filters_map (typedef)

#include <core/pose/Pose.fwd.hh>

// Utility Headers
#include <utility/factory/WidgetRegistrator.hh>
#include <utility/pointer/ReferenceCount.hh>
#include <utility/tag/Tag.fwd.hh>

// c++ headers
#include <map>

#include <utility/vector1.hh>


namespace protocols {
namespace moves {

/// @brief This templated class will register an instance of an
/// MoverCreator (class T) with the MoverFactory.  It will ensure
/// that no MoverCreator is registered twice, and, centralizes
/// this registration logic so that thread safety issues can be handled in
/// one place
template < class T >
class MoverRegistrator : public utility::factory::WidgetRegistrator< MoverFactory, T >
{
public:
	typedef utility::factory::WidgetRegistrator< MoverFactory, T > parent;
public:
	MoverRegistrator() : parent() {}
};


class MoverFactory : public utility::pointer::ReferenceCount
{
public:
	typedef std::map< std::string, MoverCreatorOP > MoverMap;
	typedef utility::tag::Tag Tag;
	typedef utility::tag::TagPtr TagPtr;
	typedef core::pose::Pose Pose;

public:
	virtual ~MoverFactory();

	static
	MoverFactory * get_instance();

	void factory_register( MoverCreatorOP creator );

	/// @brief Create a mover given its identifying string
	MoverOP newMover( std::string const & );

	///@brief return new Mover by Tag parsing; the identifying string for the Mover is in the Tag
	MoverOP
	newMover(
		TagPtr const,
		DataMap &,
		protocols::filters::Filters_map const &,
		Movers_map const &,
		Pose const &
	);

private:
	MoverFactory();

	// Unimplemented -- uncopyable
	MoverFactory( MoverFactory const & );
	MoverFactory const & operator = ( MoverFactory const & );

private:
	static MoverFactory * instance_;

	MoverMap mover_creator_map_;

};

} //namespace moves
} //namespace protocols

#endif
