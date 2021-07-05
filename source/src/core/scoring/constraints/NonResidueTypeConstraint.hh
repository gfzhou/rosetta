// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/constraints/ResidueConstraint.hh
///
/// @brief implements favor_non_native_residue constraints, assigning an energy bonus
/// to a residue if it matches the identity within the constraint object
/// @author Sarel Fleishman


#ifndef INCLUDED_core_scoring_constraints_NonResidueTypeConstraint_HH
#define INCLUDED_core_scoring_constraints_NonResidueTypeConstraint_HH

#include <core/scoring/constraints/NonResidueTypeConstraint.fwd.hh>
#include <core/scoring/constraints/Constraint.hh>

#include <core/id/AtomID.hh>


#include <utility/vector1.hh>


#ifdef    SERIALIZATION
// Cereal headers
#include <cereal/access.fwd.hpp>
#include <cereal/types/polymorphic.fwd.hpp>
#endif // SERIALIZATION


namespace core {
namespace scoring {
namespace constraints {

/// @brief This class favors a particular residue identity at a particular position by reducing its res_type energy.
class NonResidueTypeConstraint : public Constraint
{
public:
	typedef core::Real Real;
public:

	NonResidueTypeConstraint(
		pose::Pose const & pose,
		Size seqpos,
		Real non_native_residue_bonus
	);


	NonResidueTypeConstraint(
		pose::Pose const & pose,
		Size seqpos,
		std::string const & AAname,
		Real favor_non_native_bonus
	);

	NonResidueTypeConstraint(
		Size seqpos,
		std::string const & aa_in,
		std::string const & name3_in,
		core::Real bonus_in
	);

	~NonResidueTypeConstraint() override;

	Size
	natoms() const override { return 0; }

	AtomID const &
	atom( Size const ) const override {
		utility_exit_with_message("NonResidueTypeConstraint is not atom-based!.");
		return core::id::GLOBAL_BOGUS_ATOM_ID;  // required for compilation on Windows
	}

	utility::vector1< core::Size >
	residues() const override;

	void
	show( std::ostream & out ) const override;

	ConstraintOP
	remap_resid( core::id::SequenceMapping const &seqmap ) const override;

	// Needed to get the base class overloads
	using Constraint::score;
	using Constraint::dist;

	void
	score( func::XYZ_Func const & xyz_func, EnergyMap const & weights, EnergyMap & emap ) const override;

	/// @details Return 1.0 if constraint will get a penalty, 0.0 if not
	core::Real
	dist( core::scoring::func::XYZ_Func const & xyz ) const override;

	void
	fill_f1_f2(
		AtomID const & atom,
		func::XYZ_Func const & xyz,
		Vector & F1,
		Vector & F2,
		EnergyMap const & weights
	) const override;

	ConstraintOP
	clone() const override;

	bool operator == ( Constraint const & rhs ) const override;
	bool same_type_as_me( Constraint const & other ) const override;

private:
	Size seqpos_;
	std::string AAname;
	std::string rsd_type_name3_;
	core::Real favor_non_native_bonus_;

#ifdef    SERIALIZATION
protected:
	friend class cereal::access;
	NonResidueTypeConstraint();

public:
	template< class Archive > void save( Archive & arc ) const;
	template< class Archive > void load( Archive & arc );
#endif // SERIALIZATION

}; // RotamerConstraint


} // namespace constraints
} // namespace scoring
} // namespace core

#ifdef    SERIALIZATION
CEREAL_FORCE_DYNAMIC_INIT( core_scoring_constraints_NonResidueTypeConstraint )
#endif // SERIALIZATION


#endif // INCLUDED_core_scoring_constraints_NonResidueTypeConstraint_HH
