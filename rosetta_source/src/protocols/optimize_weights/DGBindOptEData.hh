// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   protocols/optimize_weights/DGBindOptEData.hh
///
/// @brief
/// @author Ian W. Davis


#ifndef INCLUDED_protocols_optimize_weights_DGBindOptEData_hh
#define INCLUDED_protocols_optimize_weights_DGBindOptEData_hh

#include <protocols/optimize_weights/DGBindOptEData.fwd.hh>
#include <protocols/optimize_weights/OptEData.hh>

#include <iostream>
// AUTO-REMOVED #include <fstream>

#include <utility/vector1.hh>


namespace protocols {
namespace optimize_weights {


///@brief Score a "bound" and an "unbound" structure, and minimize the squared error
/// between the difference in their scores and the experimental delta_G of binding.
class DGBindOptEData : public OptEPositionData
{
public:

	DGBindOptEData();
	virtual ~DGBindOptEData();

	// setters
	void deltaG_bind(Real x) { deltaG_bind_ = x; }
	void bound_struct(SingleStructureDataOP x) { bound_ = x; }
	void unbound_struct(SingleStructureDataOP x) { unbound_ = x; }

	// my invention to avoid code duplication
	virtual
	Real
	do_score(
		std::ostream & ostr,
		Multivec const & component_weights,
		Multivec const & vars,
		Multivec & dE_dvars,
		/// Basically, turn over all the private data from OptEMultiFunc
		Size const num_energy_dofs,
		int const num_ref_dofs,
		int const num_total_dofs,
		EnergyMap const & fixed_terms,
		ScoreTypes const & score_list,
		ScoreTypes const & fixed_score_list,
		bool const print
	) const;

	// inherited from OptEPositionData
	virtual
	Real
	get_score(
		Multivec const & component_weights,
		Multivec const & vars,
		Multivec & dE_dvars,
		/// Basically, turn over all the private data from OptEMultiFunc
		Size const num_energy_dofs,
		int const num_ref_dofs,
		int const num_total_dofs,
		EnergyMap const & fixed_terms,
		ScoreTypes const & score_list,
		ScoreTypes const & fixed_score_list
	) const
	{ return do_score(std::cout, component_weights, vars, dE_dvars, num_energy_dofs, num_ref_dofs, num_total_dofs, fixed_terms, score_list, fixed_score_list, false /* don't print */); }

	virtual
	void
	print_score(
		std::ostream & ostr,
		Multivec const & component_weights,
		Multivec const & vars,
		Multivec & dE_dvars,
		/// Basically, turn over all the private data from OptEMultiFunc
		Size const num_energy_dofs,
		int const num_ref_dofs,
		int const num_total_dofs,
		EnergyMap const & fixed_terms,
		ScoreTypes const & score_list,
		ScoreTypes const & fixed_score_list
	) const
	{ do_score(ostr, component_weights, vars, dE_dvars, num_energy_dofs, num_ref_dofs, num_total_dofs, fixed_terms, score_list, fixed_score_list, true /* do print */); }

	virtual
	void
	range(
		ScoreTypes const & free_score_list,
		ScoreTypes const & fixed_score_list,
		EnergyMap & lower_bound,
		EnergyMap & upper_bound
	) const;

	virtual
	Size
	size() const
	{ return 2; }

	virtual
	OptEPositionDataType
	type() const
	{ return dG_binding_correlation; }

	virtual
	void
	write_to_file( std::ofstream & /*outfile*/ ) const
	{}

	virtual
	void
	read_from_file( std::ifstream & /*infile*/ )
	{}

	virtual
	void
	write_to_binary_file( std::ofstream & /*outfile*/ ) const
	{}

	virtual
	void
	read_from_binary_file( std::ifstream & /*infile*/ )
	{}

	virtual
	Size
	memory_use() const;

#ifdef USEMPI
	virtual
	void
	send_to_node( int const destination_node, int const tag ) const;

	virtual
	void
	receive_from_node( int const source_node, int const tag );
#endif

private:
	Real deltaG_bind_;
	SingleStructureDataOP bound_;
	SingleStructureDataOP unbound_;

}; // DGBindOptEData


} // namespace optimize_weights
} // namespace protocols

#endif // INCLUDED_protocols_optimize_weights_DGBindOptEData_HH
