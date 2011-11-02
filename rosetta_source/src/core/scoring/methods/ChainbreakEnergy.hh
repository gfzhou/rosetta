// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/scoring/methods/EtableEnergy.hh
/// @brief  Etable energy method class declaration
/// @author Phil Bradley
/// @author Andrew Leaver-Fay

#ifndef INCLUDED_core_scoring_methods_ChainbreakEnergy_hh
#define INCLUDED_core_scoring_methods_ChainbreakEnergy_hh

// Unit headers
#include <core/scoring/methods/ChainbreakEnergy.fwd.hh>

// Package headers
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/scoring/EnergyMap.fwd.hh>
#include <core/scoring/methods/WholeStructureEnergy.hh>

// Project headers
#include <core/pose/Pose.fwd.hh>

#include <utility/vector1.hh>



//#include <ObjexxFCL/FArray3D.hh>

namespace core {
namespace scoring {
namespace methods {

/// @brief ChainbreakEnergy class iterates across all residues in finalize()
/// and determines the penalty between residues i and i+1 by how much their
/// psueduo atoms do not align.
class ChainbreakEnergy : public WholeStructureEnergy  {
public:
	typedef WholeStructureEnergy  parent;

public:

	ChainbreakEnergy();

	/// clone
	virtual
	EnergyMethodOP
	clone() const {
		return new ChainbreakEnergy;
	}

	/// called at the end of energy evaluation
	virtual
	void
	finalize_total_energy(
		pose::Pose & pose,
		ScoreFunction const &,
		EnergyMap & totals
	) const;


	/// called during gradient-based minimization inside dfunc
	/**
		 F1 and F2 are not zeroed -- contributions from this atom are
		 just summed in
	**/
	virtual
	void
	eval_atom_derivative(
		id::AtomID const & id,
		pose::Pose const & pose,
		kinematics::DomainMap const & domain_map,
		ScoreFunction const & sfxn,
		EnergyMap const & weights,
		Vector & F1,
		Vector & F2
	) const;

	virtual
	void
	indicate_required_context_graphs( utility::vector1< bool > & ) const;

	virtual
	core::Size version() const;


};

} // methods
} // scoring
} // core


#endif
