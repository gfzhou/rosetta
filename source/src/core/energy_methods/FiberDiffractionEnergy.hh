// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/fiber_diffraction/FiberDiffractionEnergy.hh
/// @brief  FiberDiffraction scoring and derivatives (all-atom)
/// @author Wojciech Potrzebowski and Ingemar Andre


#ifndef INCLUDED_core_scoring_fiber_diffraction_FiberDiffractionEnergy_hh
#define INCLUDED_core_scoring_fiber_diffraction_FiberDiffractionEnergy_hh

#include <core/scoring/EnergyMap.fwd.hh>
#include <core/scoring/methods/WholeStructureEnergy.hh>
#include <core/scoring/ScoreFunction.fwd.hh>

#include <core/pose/Pose.fwd.hh>
#include <core/id/AtomID.hh>
#include <utility/vector0.hh>
#include <utility/vector1.hh>
#include <numeric/xyzVector.hh>



namespace core {
namespace energy_methods {

class FiberDiffractionEnergy : public core::scoring::methods::WholeStructureEnergy  {
public:
	typedef core::scoring::methods::WholeStructureEnergy  parent;

public:


	FiberDiffractionEnergy();


	FiberDiffractionEnergy( FiberDiffractionEnergy const & src ) :
		parent(src),
		I( src.I ),
		dchi2_d(src.dchi2_d),
		dchi2_d_cross_R(src.dchi2_d_cross_R),
		chi2_(src.chi2_),
		scale_factor_(src.scale_factor_)
	{}

	core::scoring::methods::EnergyMethodOP clone() const override;

	void finalize_total_energy(pose::Pose & pose, core::scoring::ScoreFunction const &, core::scoring::EnergyMap & totals) const override;

	void setup_for_scoring( pose::Pose & pose, core::scoring::ScoreFunction const & scorefxn ) const override;

	void setup_for_derivatives( pose::Pose & pose, core::scoring::ScoreFunction const & sf) const override;

	void
	eval_atom_derivative(
		id::AtomID const & id,
		pose::Pose const & pose,
		kinematics::DomainMap const &, // domain_map,
		core::scoring::ScoreFunction const & sfxn,
		core::scoring::EnergyMap const & weights,
		Vector & F1,
		Vector & F2
	) const override;

	void indicate_required_context_graphs(utility::vector1< bool > & /*context_graphs_required*/
	) const override {}

private:
	//////
	// PRIVATE DATA becuase the functions in this class are const we need to make variables mutable
	// store calculated intensities
	mutable utility::vector0 < utility::vector1 < core::Real > > I;
	// precomputed derivatives
	mutable utility::vector1< numeric::xyzVector< core::Real > > dchi2_d;
	mutable utility::vector1< numeric::xyzVector< core::Real > > dchi2_d_cross_R;
	mutable std::map<  core::id::AtomID, core::Size > AtomID_to_atomnbr_;
	// saved chi2, c scaling factor
	mutable core::Real chi2_;
	mutable core::Real scale_factor_;
	mutable core::Real square_obs_, sum_obs_;
	mutable core::Real Rscale_factor;
	mutable core::Real Rsum_obs;
	mutable core::Real p_, c_;
	mutable core::Real a_, b_;
	core::Size version() const override;
};

}
}

#endif
