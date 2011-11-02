// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is made available under the Rosetta Commons license.
// See http://www.rosettacommons.org/license
// (C) 199x-2007 University of Washington
// (C) 199x-2007 University of California Santa Cruz
// (C) 199x-2007 University of California San Francisco
// (C) 199x-2007 Johns Hopkins University
// (C) 199x-2007 University of North Carolina, Chapel Hill
// (C) 199x-2007 Vanderbilt University

/// @file   core/scoring/methods/pHEnergy.hh
/// @brief  pH Score (kT = 0.59 at 298 K) based on the chemical potential of the
///         protonated/deprotonated residues. It indirectly contributes to a change
///         in rotamer probabilities of protonated/deprotonated residues in the
///         Dunbrack energy depending on the value of pH
/// @author Krishna


#ifndef INCLUDED_core_scoring_methods_pHEnergy_hh
#define INCLUDED_core_scoring_methods_pHEnergy_hh

// Unit Headers
#include <core/scoring/methods/pHEnergy.fwd.hh>

// Package headers
#include <core/scoring/methods/ContextIndependentOneBodyEnergy.hh>
#include <core/scoring/ScoreFunction.fwd.hh>

// Project headers
#include <core/pose/Pose.fwd.hh>
// AUTO-REMOVED #include <basic/options/option.hh>
// AUTO-REMOVED #include <basic/options/keys/OptionKeys.hh>
#include <core/id/TorsionID.fwd.hh>
#include <core/id/DOF_ID.fwd.hh>

#include <utility/vector1.hh>


namespace core {
namespace scoring {
namespace methods {

///
class pHEnergy : public ContextIndependentOneBodyEnergy {

public:
	typedef ContextIndependentOneBodyEnergy parent;

public:
	///
	pHEnergy();

	/// clone
	virtual
	EnergyMethodOP
	clone() const;

	/////////////////////////////////////////////////////////////////////////////
	// scoring
	/////////////////////////////////////////////////////////////////////////////

	virtual
	void
	residue_energy(
		conformation::Residue const & rsd,
		pose::Pose const & pose,
		EnergyMap & emap
	) const;


        virtual
	Real
	eval_dof_derivative(
		id::DOF_ID const & dof_id, id::TorsionID const & tor_id, pose::Pose const & pose,
		ScoreFunction const & sfxn, EnergyMap const & weights
	) const;


	virtual
	void indicate_required_context_graphs( utility::vector1< bool > & ) const;

        static void set_pH ( core::Real );


private:

	static core::Real pH_;
virtual
core::Size version() const;


};


}
}
}

#endif // INCLUDED_core_scoring_ScoreFunction_HH
