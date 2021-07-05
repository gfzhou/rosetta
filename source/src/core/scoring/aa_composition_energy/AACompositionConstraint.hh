// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file src/core/scoring/aa_composition_energy/AACompositionConstraint.hh
/// @brief Headers for a constraint for constraining sequences to have a desired amino acid composition, analogous to a geometric constraint.
/// @details The corresponding energy term for this constraint is the AACompositionEnergy (aa_composition in wts files).
/// @author Vikram K. Mulligan (vmullig@uw.edu)

#ifndef INCLUDED_core_scoring_aa_composition_energy_AACompositionConstraint_hh
#define INCLUDED_core_scoring_aa_composition_energy_AACompositionConstraint_hh

#include <core/scoring/aa_composition_energy/SequenceConstraint.hh>
#include <core/scoring/aa_composition_energy/AACompositionConstraint.fwd.hh>

#include <core/scoring/constraints/Constraint.hh>
#include <core/select/residue_selector/ResidueSelector.fwd.hh>
#include <core/scoring/aa_composition_energy/AACompositionEnergySetup.fwd.hh>

#include <core/pose/Pose.fwd.hh>

#include <string>


#ifdef    SERIALIZATION
// Cereal headers
#include <cereal/access.fwd.hpp>
#include <cereal/types/polymorphic.fwd.hpp>
#endif // SERIALIZATION


namespace core {
namespace scoring {
namespace aa_composition_energy {

class AACompositionConstraint : public core::scoring::aa_composition_energy::SequenceConstraint {

public: //Constructor, destructor, copy, clone:

	/// @brief Constructor
	AACompositionConstraint();

	/// @brief Copy constructor
	AACompositionConstraint( AACompositionConstraint const & src );

	/// @brief Destructor
	~AACompositionConstraint() override;

	/// @brief Clone operator
	constraints::ConstraintOP clone() const override;

	bool operator == ( constraints::Constraint const & /*other*/ ) const override;

	bool
	same_type_as_me( constraints::Constraint const & other ) const override;


public: //Functions that actually do stuff:

	/// @brief Set the selector to be used by this constraint.
	/// @details Clones the input.
	void set_selector( select::residue_selector::ResidueSelectorCOP selector_in );

	/// @brief Const access to the selector.
	/// @details Returns a null pointer if no selector has been specified.
	select::residue_selector::ResidueSelectorCOP selector() const;

	/// @brief Const access to the AACompositionEnergySetup object.
	AACompositionEnergySetupCOP
	aa_composition_energy_setup() const;

	/// @brief Initialize the AACompositionEnergySetup object from a file.
	void initialize_from_file( std::string const &filename );

	/// @brief Initialize the AACompositionEnergySetup object from the contents of a file.
	/// @details Allows external code to initialize a constriant object without having the
	/// object read directly from disk.
	void initialize_from_file_contents( std::string const &filecontents );

	/// @brief Print info on the constraint
	void show_def (std::ostream &TO, pose::Pose const &pose) const override;

private:
	// Member variables

	/// @brief Owning pointer to a ResidueSelector.
	/// @details Optional; will serve as a mask for this AACompositionConstraint if provided.
	select::residue_selector::ResidueSelectorOP selector_;

	/// @brief AACompositionEnergySetup object stored by this object.
	/// @details Created on object construction
	AACompositionEnergySetupOP aa_comp_setup_;

#ifdef    SERIALIZATION
protected:
	friend class cereal::access;

public:
	template< class Archive > void save( Archive & arc ) const;
	template< class Archive > void load( Archive & arc );
#endif // SERIALIZATION

};

} // aa_composition_energy
} // scoring
} // core

#ifdef    SERIALIZATION
CEREAL_FORCE_DYNAMIC_INIT( core_scoring_aa_composition_energy_AACompositionConstraint )
#endif // SERIALIZATION

#endif
