// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file protocols/simple_filters/SymUnsatHbondFilter.hh
/// @brief definition of filter class SymUnsatHbondFilter.
/// @author Sarel Fleishman (sarelf@u.washington.edu), Jacob Corn (jecorn@u.washington.edu)

#ifndef INCLUDED_devel_matdes_SymUnsatHbondFilter_hh
#define INCLUDED_devel_matdes_SymUnsatHbondFilter_hh

#include <devel/matdes/SymUnsatHbondFilter.fwd.hh>

#include <protocols/filters/Filter.hh>
#include <utility/tag/Tag.fwd.hh>
#include <protocols/moves/DataMap.fwd.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <core/pose/Pose.fwd.hh>

namespace devel {
namespace matdes {

/// @brief filters based on an upper bound # of buried unsatisfied polar residues
class SymUnsatHbondFilter : public protocols::filters::Filter {
public:

  typedef protocols::filters::Filter Super;
  typedef protocols::filters::Filter Filter;
  typedef protocols::filters::FilterOP FilterOP;
  typedef core::Real Real;
  typedef core::pose::Pose Pose;

  typedef utility::tag::TagPtr TagPtr;
  typedef protocols::filters::Filters_map Filters_map;
  typedef protocols::moves::DataMap DataMap;
  typedef protocols::moves::Movers_map Movers_map;

public:
	// @brief default constructor
	SymUnsatHbondFilter();

	// @brief constructor with arguments
	SymUnsatHbondFilter( core::Size const upper_cutoff, core::Size const jump, std::string const sym_dofs,  bool verb, bool write );

	// @brief copy constructor
	SymUnsatHbondFilter( SymUnsatHbondFilter const & rval );

	virtual ~SymUnsatHbondFilter(){}

public:// virtual constructor

	// @brief make clone
  virtual protocols::filters::FilterOP clone() const;

  // @brief make fresh instance
  virtual protocols::filters::FilterOP fresh_instance() const;

public:// accessor
  // @brief get name of this filter
  virtual std::string name() const { return "SymUnsatHbond"; }

public:// setters
	void upper_threshold( core::Size const upper_cutoff );
	void jump_num( core::Size const jump );
	void sym_dof_names( std::string const sym_dofs );
	void verbose( bool const verb );
	void write2pdb( bool const write );

public:// getters
	core::Size upper_threshold() const;
	core::Size jump_num() const;
	std::string sym_dof_names() const;
	bool verbose() const;
	bool write2pdb() const;

public:// parser

  virtual void parse_my_tag( TagPtr const tag,
    DataMap &,
    protocols::filters::Filters_map const &,
    Movers_map const &,
    Pose const & );
	void parse_def( utility::lua::LuaObject const & def,
					utility::lua::LuaObject const & score_fxns,
					utility::lua::LuaObject const & tasks );

public:// virtual main operation

	// @brief returns true if given pose passes the filter, false otherwise.
	virtual bool apply( core::pose::Pose const & pose ) const;

	// @brief report information to scorefile and std::out
	virtual core::Real report_sm( core::pose::Pose const & pose ) const;
	virtual void report( std::ostream & out, core::pose::Pose const & pose ) const;

	// @brief calc number of unsatisfied hydrogen bonds
	core::Real compute( core::pose::Pose const & pose, bool const & verb, bool const & write ) const;
	void write_to_pdb( std::string const residue_name, core::Size const residue, std::string const atom_name ) const;
	void write_pymol_string_to_pdb( std::string const pymol_selection ) const;

private:

	core::Size upper_threshold_;
	core::Size jump_num_;
	std::string sym_dof_names_;
	bool verbose_;
	bool write2pdb_;

};

}
}
#endif
