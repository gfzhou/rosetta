// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

#ifndef INCLUDED_core_scoring_orbitals_OrbitalsScore_hh
#define INCLUDED_core_scoring_orbitals_OrbitalsScore_hh

#include <core/scoring/methods/ContextDependentTwoBodyEnergy.hh>
#include <core/pose/Pose.fwd.hh>
// AUTO-REMOVED #include <numeric/xyzVector.hh>
#include <core/scoring/orbitals/OrbitalsLookup.hh>
#include <core/scoring/orbitals/OrbitalsScore.fwd.hh>
// AUTO-REMOVED #include <core/scoring/etable/EtableEnergy.hh>
// AUTO-REMOVED #include <core/scoring/methods/EnergyMethodOptions.hh>
#include <core/chemical/orbitals/OrbitalTypeMapper.fwd.hh>


#include <core/scoring/ScoreFunction.fwd.hh>
#include <map>
// AUTO-REMOVED #include <list>

#include <core/scoring/methods/EnergyMethodOptions.fwd.hh>
#include <utility/vector1.hh>


namespace core{
namespace scoring{
namespace orbitals{


class OrbitalsScore : public methods::ContextDependentTwoBodyEnergy {
public:
	typedef methods::ContextDependentTwoBodyEnergy parent;


//virtual functions from score functions
public:
	OrbitalsScore(methods::EnergyMethodOptions const &);

	//clone
	virtual methods::EnergyMethodOP clone() const;

	virtual
	void setup_for_scoring(pose::Pose & pose, ScoreFunction const &) const;

	virtual
	void setup_for_derivatives( pose::Pose &pose, ScoreFunction const &  ) const;

	virtual
	void
	eval_residue_pair_derivatives(
		conformation::Residue const & rsd1,
		conformation::Residue const & rsd2,
		ResSingleMinimizationData const &,
		ResSingleMinimizationData const &,
		ResPairMinimizationData const &,
		pose::Pose const &, // provides context
		EnergyMap const &,
		utility::vector1< DerivVectorPair > & r1_atom_derivs,
		utility::vector1< DerivVectorPair > & r2_atom_derivs
	) const;

	virtual
	void eval_intrares_energy(
		core::conformation::Residue const &,
		core::pose::Pose const &,
		core::scoring::ScoreFunction const &,
		core::scoring::EnergyMap &
	) const;

	 virtual
	 bool defines_intrares_energy(const core::scoring::EnergyMap&) const; //false

	 virtual
	 void residue_pair_energy(
		 core::conformation::Residue const & res1,
		 core::conformation::Residue const & res2,
		 core::pose::Pose const &,
		 core::scoring::ScoreFunction const &,
		 EnergyMap & emap
	 ) const;

	virtual
	core::Real atomic_interaction_cutoff() const; //set to default

	virtual
	void indicate_required_context_graphs( utility::vector1< bool > &  ) const;

	virtual
	bool
	minimize_in_whole_structure_context( pose::Pose const & ) const {
		return false;
	}

	virtual
	void
	prepare_rotamers_for_packing(
		pose::Pose const & pose,
		conformation::RotamerSetBase & set
	) const;

	virtual
	void
	update_residue_for_packing(
		pose::Pose &,
		Size resid
	) const;

public:

	void get_E_haro_one_way(
			core::conformation::Residue const & res1,
			core::conformation::Residue const & res2,
			core::Real & HARO_sc_H_sc_orb_E,
			core::Real & HARO_DHO_angle_E
	) const;

	void get_E_hpol_one_way(
			core::conformation::Residue const & res1,
			core::conformation::Residue const & res2,
			core::Real & HPOL_sc_H_sc_orb_E,
			core::Real & HPOL_bb_H_sc_orb_energy,
			core::Real & HPOL_sc_H_bb_orb_energy,
			core::Real & HPOL_DHO_angle_E
	) const;

	void get_orb_H_distance_and_energy(
			core::conformation::Residue const & res1,
			core::Size const & atom_index,
			numeric::xyzVector<core::Real> const & atom_xyz,
			numeric::xyzVector<core::Real> const & H_xyz,
			numeric::xyzVector<core::Real> const & donor_xyz,
			core::Real & sc_energy,
			core::Real & bb_h_energy,
			core::Real & bb_orb_energy,
			core::Real & DHO_angle_E,
			OrbitalsLookup::h_type htype,
			bool bb_h_flag,
			bool bb_orb_flag
	) const;

	void assign_haro_derivs_one_way(
			core::conformation::Residue const & res1,
			core::conformation::Residue const & res2,
			EnergyMap const & weights,
			utility::vector1< DerivVectorPair > & r1_atom_derivs,
			utility::vector1< DerivVectorPair > & r2_atom_derivs
	) const;

	void assign_hpol_derivs_one_way(
			core::conformation::Residue const & res1,
			core::conformation::Residue const & res2,
			EnergyMap const & weights,
			utility::vector1< DerivVectorPair > & r1_atom_derivs,
			utility::vector1< DerivVectorPair > & r2_atom_derivs

	) const;

	void assign_orb_H_derivs(
			core::conformation::Residue const & res1,
			core::conformation::Residue const & res2,
			core::Size const & atom_index,
			numeric::xyzVector<core::Real> const & atom_xyz,
			core::Size const & H_index,
			numeric::xyzVector<core::Real> const & H_xyz,
			OrbitalsLookup::h_type htype,
			EnergyMap const & weights,
			utility::vector1< DerivVectorPair > & r1_atom_derivs,
			utility::vector1< DerivVectorPair > & r2_atom_derivs
	)const;

//virtual private functions
private:
	virtual core::Size version() const;

private:
	OrbitalsLookup const & lookup_table_;
	core::Real const max_orbital_dist_squared_;//defaault 4A or 16A squared. based on statistics
	core::Real nbr_distance_squared_;//default 10A or 100A squared
	std::map<core::chemical::orbitals::orbital_type_enum, core::Real> min_orb_dist_enum_map_;
	core::Real max_dist_squared_; //the maximum distance squared which orbitals are scored. based on atom distance



	//core::scoring::etable::EtableEnergy lookup_Etable_;
	//core::scoring::etable::EtableCAP etable_;
	//mutable utility::vector1< utility::vector1< Vector> > orbital_coords_;






	//


};





}//namespace orbitals
}//namespace scoring
}//namespace core



#endif /* ORBITALSSCORE_HH_ */
