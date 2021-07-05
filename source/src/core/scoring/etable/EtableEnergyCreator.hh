// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   src/core/scoring/etable/EtableEnergyCreator.hh
/// @brief  Declaration for the class that connects EtableEnergy with the ScoringManager
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com)

#ifndef INCLUDED_core_scoring_etable_EtableEnergyCreator_hh
#define INCLUDED_core_scoring_etable_EtableEnergyCreator_hh

#include <core/scoring/methods/EnergyMethodCreator.hh>

#include <core/scoring/methods/EnergyMethod.fwd.hh>
#include <core/scoring/methods/EnergyMethodOptions.fwd.hh>



namespace core {
namespace scoring {
namespace etable {

class EtableEnergyCreator : public methods::EnergyMethodCreator
{
public:
	/// @brief Instantiate a new EtableEnergy
	methods::EnergyMethodOP
	create_energy_method(
		methods::EnergyMethodOptions const &
	) const override;

	/// @brief Return the set of score types claimed by the EnergyMethod
	/// this EnergyMethodCreator creates in its create_energy_method() function
	ScoreTypes
	score_types_for_method() const override;

};

class EtableClassicIntraEnergyCreator : public methods::EnergyMethodCreator
{
public:
	/// @brief Instantiate a new EtableEnergy
	methods::EnergyMethodOP
	create_energy_method(
		methods::EnergyMethodOptions const &
	) const override;

	/// @brief Return the set of score types claimed by the EnergyMethod
	/// this EnergyMethodCreator creates in its create_energy_method() function
	ScoreTypes
	score_types_for_method() const override;

};

}
}
}

#endif
