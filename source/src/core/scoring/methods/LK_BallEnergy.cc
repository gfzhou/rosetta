// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/scoring/methods/LK_BallEnergy.cc
/// @brief  Orientation dependent variant of the LK Solvation using
/// @author Phil Bradley

#include <core/scoring/methods/ContextDependentLRTwoBodyEnergy.hh>
// #include <core/scoring/GenBornPotential.fwd.hh>


// Unit headers
#include <core/scoring/methods/LK_BallEnergy.hh>
#include <core/scoring/methods/LK_BallEnergyCreator.hh>
#include <core/scoring/methods/LK_BallInfo.hh>

// Package headers
#include <core/scoring/methods/EnergyMethodOptions.hh>
#include <core/scoring/ScoringManager.hh>
#include <core/scoring/NeighborList.hh>
#include <core/scoring/ResidueNeighborList.hh>
#include <core/scoring/MinimizationData.hh>
#include <core/scoring/EnergyGraph.hh>
#include <core/scoring/Energies.hh>
#include <core/scoring/EnergiesCacheableDataType.hh>
#include <core/scoring/DerivVectorPair.hh>
#include <core/scoring/etable/Etable.hh>
#include <core/scoring/etable/count_pair/CountPairFunction.hh>
#include <core/scoring/etable/count_pair/CountPairFactory.hh>
#include <core/scoring/etable/count_pair/types.hh>

// Project headers
#include <core/pose/Pose.hh>
//#include <core/pack/rotamer_set/RotamerSet.hh>
//#include <core/pack/rotamer_set/WaterPackingInfo.hh>
#include <core/conformation/residue_datacache.hh>
#include <core/conformation/RotamerSetBase.hh>
#include <core/conformation/RotamerSetCacheableDataType.hh>
#include <core/chemical/AtomType.hh>
#include <core/pose/datacache/CacheableDataType.hh>
#include <core/scoring/ScoreFunction.hh>
#include <ObjexxFCL/format.hh>
#include <ObjexxFCL/FArray2D.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/ResidueFactory.hh>
// #include <core/io/pdb/pose_io.hh> // HACK
// #include <fstream> // HACK

#include <core/scoring/constraints/AngleConstraint.hh>

#include <basic/options/util.hh> // HACK
#include <basic/options/keys/dna.OptionKeys.gen.hh>
#include <basic/options/keys/score.OptionKeys.gen.hh>

#include <basic/prof.hh>
#include <basic/datacache/BasicDataCache.hh>
#include <basic/Tracer.hh>

#include <numeric/constants.hh>
#include <numeric/xyz.functions.hh>

#include <utility/vector1.functions.hh> // HACK

#ifdef WIN32
#include <core/pack/rotamer_set/WaterAnchorInfo.hh>
#endif

/// LAZY using
//using core::pack::rotamer_set::RotamerSet;

namespace core {
namespace scoring {
namespace methods {


/// @details This must return a fresh instance of the LK_hack class,
/// never an instance already in use
methods::EnergyMethodOP
LK_BallEnergyCreator::create_energy_method(
	methods::EnergyMethodOptions const & options
) const {
	return methods::EnergyMethodOP( new LK_BallEnergy( options ) );
}

ScoreTypes
LK_BallEnergyCreator::score_types_for_method() const {
	ScoreTypes sts;
	sts.push_back( lk_ball_iso );
	sts.push_back( lk_ball );
	sts.push_back( lk_ball_wtd );
	return sts;
}


static thread_local basic::Tracer TR("core.scoring.methods.LK_BallEnergy");

/// inline retrieval functions here:
inline
LKB_RotamerSetInfo const &
retrieve_lkb_rotamer_set_info( conformation::RotamerSetBase const & set ) {
	return static_cast< LKB_RotamerSetInfo const & >
		( set.data().get( conformation::RotamerSetCacheableDataType::LK_BALL_ROTAMER_SET_INFO ) );
}

inline
LKB_ResidueInfo const &
retrieve_lkb_residue_info( pose::Pose const & pose, Size const seqpos ) {
	debug_assert( seqpos && seqpos <= ( static_cast< LKB_PoseInfo const & >
		( pose.data().get( pose::datacache::CacheableDataType::LK_BALL_POSE_INFO ) )).size() );
	return ( static_cast< LKB_PoseInfo const & >
		( pose.data().get( pose::datacache::CacheableDataType::LK_BALL_POSE_INFO ) )[ seqpos ] );
}

inline
LKB_ResidueInfo &
retrieve_nonconst_lkb_residue_info( pose::Pose & pose, Size const seqpos ) {
	debug_assert( seqpos && seqpos <= ( static_cast< LKB_PoseInfo const & >
		( pose.data().get( pose::datacache::CacheableDataType::LK_BALL_POSE_INFO ) )).size() );
	return ( static_cast< LKB_PoseInfo & >
		( pose.data().get( pose::datacache::CacheableDataType::LK_BALL_POSE_INFO ) )[ seqpos ] );
}


class LKB_ResPairMinData : public basic::datacache::CacheableData
{
public:
	LKB_ResPairMinData();
	virtual ~LKB_ResPairMinData() {}
	virtual basic::datacache::CacheableDataOP clone() const
	{ return basic::datacache::CacheableDataOP( new LKB_ResPairMinData( *this ) ); }

	void
	initialize(
		LKB_ResidueInfoCOP res1_data,
		LKB_ResidueInfoCOP res2_data
	);
	//  void set_res1_data( LKB_ResidueInfoCOP );
	//  void set_res2_data( LKB_ResidueInfoCOP );

	LKB_ResidueInfo const & res1_data() const { return *res1_data_; }
	LKB_ResidueInfo const & res2_data() const { return *res2_data_; }

	bool
	initialized() const { return initialized_; }


private:

	LKB_ResidueInfoCOP res1_data_;
	LKB_ResidueInfoCOP res2_data_;

	bool initialized_;
};

typedef utility::pointer::shared_ptr< LKB_ResPairMinData >       LKB_ResPairMinDataOP;
typedef utility::pointer::shared_ptr< LKB_ResPairMinData const > LKB_ResPairMinDataCOP;

LKB_ResPairMinData::LKB_ResPairMinData():
	initialized_( false )
{}

void
LKB_ResPairMinData::initialize(
	LKB_ResidueInfoCOP res1_data,
	LKB_ResidueInfoCOP res2_data
)
{
	initialized_ = true;
	res1_data_ = res1_data;
	res2_data_ = res2_data;
}


/////////////////////////////////////// mindata retrieval functions
inline
LKB_ResPairMinData &
retrieve_nonconst_lkb_pairdata(
	ResPairMinimizationData & pairdata
)
{
	LKB_ResPairMinDataOP lkb_pairdata(0);
	if ( pairdata.get_data( lkb_respair_data ) ) {
		debug_assert( utility::pointer::dynamic_pointer_cast< LKB_ResPairMinData > ( pairdata.get_data( lkb_respair_data )));
		lkb_pairdata = utility::pointer::static_pointer_cast< LKB_ResPairMinData > ( pairdata.get_data( lkb_respair_data ));
	} else {
		lkb_pairdata = LKB_ResPairMinDataOP( new LKB_ResPairMinData );
		pairdata.set_data( lkb_respair_data, lkb_pairdata );
	}
	return *lkb_pairdata;
}

/////////////////////////////////////// mindata retrieval functions
inline
LKB_ResPairMinData const &
retrieve_lkb_pairdata(
	ResPairMinimizationData const & pairdata
)
{
	debug_assert( utility::pointer::dynamic_pointer_cast< LKB_ResPairMinData const > ( pairdata.get_data( lkb_respair_data )));
	return ( static_cast< LKB_ResPairMinData const & > ( pairdata.get_data_ref( lkb_respair_data ) ) );
}
/////////////////////////////////////// mindata retrieval functions
inline
LKB_ResidueInfo &
retrieve_nonconst_lkb_resdata(
	ResSingleMinimizationData & resdata
)
{
	LKB_ResidueInfoOP lkb_resdata( 0 );
	if ( resdata.get_data( lkb_res_data ) ) {
		debug_assert( utility::pointer::dynamic_pointer_cast< LKB_ResidueInfo > ( resdata.get_data( lkb_res_data )));
		lkb_resdata = utility::pointer::static_pointer_cast< LKB_ResidueInfo > ( resdata.get_data( lkb_res_data ) );
	} else {
		lkb_resdata = LKB_ResidueInfoOP( new LKB_ResidueInfo );
		resdata.set_data( lkb_res_data, lkb_resdata );
	}
	return *lkb_resdata;
}
/////////////////////////////////////// mindata retrieval functions
inline
LKB_ResidueInfo const &
retrieve_lkb_resdata(
	ResSingleMinimizationData const & resdata
)
{
	debug_assert( utility::pointer::dynamic_pointer_cast< LKB_ResidueInfo const > ( resdata.get_data( lkb_res_data )));
	return ( static_cast< LKB_ResidueInfo const & > ( resdata.get_data_ref( lkb_res_data ) ) );
}
/////////////////////////////////////// mindata retrieval functions
inline
LKB_ResidueInfoCOP
retrieve_lkb_resdata_ptr(
	ResSingleMinimizationData const & resdata
)
{
	debug_assert( utility::pointer::dynamic_pointer_cast< LKB_ResidueInfo const > ( resdata.get_data( lkb_res_data )));
	return ( utility::pointer::static_pointer_cast< LKB_ResidueInfo const > ( resdata.get_data( lkb_res_data ) ) );
}


/// HACKING //////////////////////////
// void
// LK_BallEnergy::setup_hack()
// {
//  using namespace options;
//  using namespace OptionKeys::dna::specificity::lk_ball_hack;
//  if ( option[ lk_ball_positions ].user() ) {
//   positions_ = option[ lk_ball_positions ]();
//  }
//  include_all_dna_ = false; // wait on this until we can do some dna relaxes and estimate reference energies
// }

// bool
// LK_BallEnergy::include_residue( conformation::Residue const & rsd ) const {
//  return ( ( include_all_dna_ && rsd.is_DNA() ) ||
//       ( utility::has_element( positions_, rsd.seqpos() ) ) );
// }

// void
// LK_BallEnergy::add_my_score_types()
// {
//  add_score_type( lk_ball_iso );
//  add_score_type( lk_ball );
//  add_score_type( lk_polar );
//  add_score_type( lk_polar_nw );
//  add_score_type( lk_nonpolar );
//  add_score_type( lk_charged );

//  add_score_type( lk_ball_xd );
//  add_score_type( lk_polar_xd );
//  add_score_type( lk_polar_nw_xd );
//  add_score_type( lk_nonpolar_xd );


//  add_score_type( lk_ball_dd );
//  add_score_type( lk_polar_dd );
//  add_score_type( lk_polar_nw_dd );
//  add_score_type( lk_nonpolar_dd );
// }

LK_BallEnergy::LK_BallEnergy( EnergyMethodOptions const & options ):
	parent             ( EnergyMethodCreatorOP( new LK_BallEnergyCreator ) ),
	etable_            ( ScoringManager::get_instance()->etable( options ).lock() ),
	solv1_             ( ScoringManager::get_instance()->etable( options ).lock()->solv1()),
	solv2_             ( ScoringManager::get_instance()->etable( options ).lock()->solv2()),
	dsolv1_            ( ScoringManager::get_instance()->etable( options ).lock()->dsolv1()),
	safe_max_dis2_     ( ScoringManager::get_instance()->etable( options ).lock()->get_safe_max_dis2() ),
	etable_bins_per_A2_( ScoringManager::get_instance()->etable( options ).lock()->get_bins_per_A2() ),
	slim_etable_       ( basic::options::option[ basic::options::OptionKeys::score::analytic_etable_evaluation ] ),
	use_intra_dna_cp_crossover_4_( true ),
	ramp_width_A2_     ( basic::options::option[ basic::options::OptionKeys::dna::specificity::lk_ball_ramp_width_A2 ]() ),
	multi_water_fade_  ( basic::options::option[ basic::options::OptionKeys::dna::specificity::lk_ball_water_fade ]() )
{
	setup_d2_bounds();
	if ( !slim_etable_ ) { runtime_assert( solv1_.size()>0 ); }
}

// LK_BallEnergy::LK_BallEnergy( etable::Etable const & etable_in):
//  etable_(etable_in),
//  solv1_(etable_in.solv1()),
//  solv2_(etable_in.solv2()),
//  dsolv1_( etable_in.dsolv1() ),
//  safe_max_dis2_( etable_in.get_safe_max_dis2() ),
//  etable_bins_per_A2_( etable_in.get_bins_per_A2())
// //  d2_low_ ( 2.0 * 2.0 ),
// //  d2_high_( 3.1 * 3.1 )
// {
//  //setup_hack();
//  add_my_score_types();

//  setup_d2_bounds();
// //  add_score_type( lk_ball_iso );
// //  add_score_type( lk_ball_iso_nw );
// }


Distance
LK_BallEnergy::atomic_interaction_cutoff() const
{
	return etable_->max_dis();
}


/// clone
EnergyMethodOP
LK_BallEnergy::clone() const
{
	return EnergyMethodOP( new LK_BallEnergy( *this ) );
}


LK_BallEnergy::LK_BallEnergy( LK_BallEnergy const & src ):
	ContextIndependentTwoBodyEnergy( src ),
	etable_(src.etable_),
	solv1_( src.solv1_ ),
	solv2_( src.solv2_ ),
	dsolv1_( src.dsolv1_ ),
	safe_max_dis2_( src.safe_max_dis2_ ),
	etable_bins_per_A2_( src.etable_bins_per_A2_ ),
	slim_etable_( src.slim_etable_ ),
	use_intra_dna_cp_crossover_4_( src.use_intra_dna_cp_crossover_4_ ),
	ramp_width_A2_     ( src.ramp_width_A2_ ),
	multi_water_fade_  ( src.multi_water_fade_ )

{
	setup_d2_bounds();
}


void
compute_and_store_pose_waters(
	pose::Pose & pose
)
{
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
	// using namespace core::pack::rotamer_set; // WaterPackingInfo
	LKB_PoseInfoOP info( new LKB_PoseInfo() );
	for ( Size i=1; i<= pose.total_residue(); ++i ) {
		info->append( LKB_ResidueInfoOP( new LKB_ResidueInfo( pose.residue(i) ) ) );
	}
	pose.data().set( pose::datacache::CacheableDataType::LK_BALL_POSE_INFO, info );
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// New minimization interface code ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
LK_BallEnergy::minimize_in_whole_structure_context( pose::Pose const &pose ) const
{
	//return false;
	return pose.energies().use_nblist_auto_update();
}


void
LK_BallEnergy::setup_for_minimizing_for_residue(
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	ScoreFunction const &, // scorefxn,
	kinematics::MinimizerMapBase const &, // min_map,
	ResSingleMinimizationData & resdata
) const
{
	if ( pose.energies().use_nblist_auto_update() ) return;

	LKB_ResidueInfo & info( retrieve_nonconst_lkb_resdata( resdata ) );
	info.initialize( rsd.type() );
	info.build_waters( rsd );
}

void
LK_BallEnergy::setup_for_minimizing_for_residue_pair(
	conformation::Residue const &, // rsd1,
	conformation::Residue const &, // rsd2,
	pose::Pose const & pose,
	ScoreFunction const &, //scorefxn,
	kinematics::MinimizerMapBase const &, // min_map,
	ResSingleMinimizationData const & res1data,
	ResSingleMinimizationData const & res2data,
	ResPairMinimizationData & pairdata
) const
{
	if ( pose.energies().use_nblist_auto_update() ) return;

	LKB_ResPairMinData & lkb_pairdata( retrieve_nonconst_lkb_pairdata( pairdata ) );
	lkb_pairdata.initialize( retrieve_lkb_resdata_ptr( res1data ),
		retrieve_lkb_resdata_ptr( res2data ) );
}


bool
LK_BallEnergy::requires_a_setup_for_scoring_for_residue_opportunity( pose::Pose const & ) const
{
	return true;
}

void
LK_BallEnergy::setup_for_scoring_for_residue(
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	ScoreFunction const &, // sfxn,
	ResSingleMinimizationData & resdata
) const
{
	if ( pose.energies().use_nblist_auto_update() ) return;

	LKB_ResidueInfo & info( retrieve_nonconst_lkb_resdata( resdata ) );
	if ( !info.matches_residue_type( rsd.type() ) ) {
		std::cout << "LK_BallEnergy::setup_for_scoring_for_residue:: lkb-info mismatch: " << info.residue_type().name() << ' ' <<
			rsd.type().name() << std::endl;
		info.initialize( rsd.type() );
	}
	info.build_waters( rsd ); // already initialized in setup for minimizing for rsd
}

bool
LK_BallEnergy::requires_a_setup_for_derivatives_for_residue_opportunity( pose::Pose const &  ) const
{
	return true;
}

void
LK_BallEnergy::setup_for_derivatives_for_residue(
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	ScoreFunction const & sfxn,
	ResSingleMinimizationData & min_data
) const
{
	/// just compute water locations
	setup_for_scoring_for_residue( rsd, pose, sfxn, min_data );
}


void
LK_BallEnergy::setup_for_packing( pose::Pose & pose, utility::vector1< bool > const &, utility::vector1< bool > const & ) const
{
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
	pose.update_residue_neighbors();
	compute_and_store_pose_waters( pose ); // could check task and do only some
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
}

void
LK_BallEnergy::update_residue_for_packing(
	pose::Pose & pose,
	Size resid
) const
{
	/// update waters for residue that has changed during packing, eg in rotamer trials
	/// need to double-check the current logic on this...
	// retrieve_nonconst_lkb_residue_info( pose, resid ).build_waters( pose.residue( resid ) );
	LKB_ResidueInfo & info( retrieve_nonconst_lkb_residue_info( pose, resid ) );
	conformation::Residue const & rsd( pose.residue( resid ) );
	if ( !info.matches_residue_type( rsd.type() ) ) {
		std::cout << "LK_BallEnergy::update_residue_for_packing:: lkb-info mismatch: " << info.residue_type().name() << ' ' <<
			rsd.type().name() << std::endl;
		info.initialize( rsd.type() );
	}
	info.build_waters( rsd );
}


void
LK_BallEnergy::setup_for_scoring(
	pose::Pose & pose,
	ScoreFunction const & //scfxn
) const
{
	// silly hack:
	// if ( std::abs( scfxn.get_weight( lk_ball_wtd ) ) > 1e-3 ) {
	//  runtime_assert( basic::options::option[ basic::options::OptionKeys::dna::specificity::no_SP3_acceptor_waters ] );
	// }

	pose.update_residue_neighbors();
	compute_and_store_pose_waters( pose );
}

void
LK_BallEnergy::prepare_rotamers_for_packing(
	pose::Pose const &, //pose,
	conformation::RotamerSetBase & rotamer_set
) const
{
	//TR.Trace << "prepare_rotamers_for_packing: " << rotamer_set.num_rotamers() << std::endl;

	/// create a rotamer set info object
	LKB_RotamerSetInfoOP info( new LKB_RotamerSetInfo );

	for ( Size n=1; n<= rotamer_set.num_rotamers(); ++n ) {
		conformation::ResidueOP rot( rotamer_set.nonconst_rotamer(n) );
		LKB_ResidueInfoOP rotinfo( new LKB_ResidueInfo( *rot ) );
		rot->nonconst_data_ptr()->set( conformation::residue_datacache::LK_BALL_INFO, rotinfo->clone() ); // DataCache::set() does not clone
		info->append( rotinfo );
	}

	rotamer_set.data().set( conformation::RotamerSetCacheableDataType::LK_BALL_ROTAMER_SET_INFO, info );

}


/////////////////////////////////////////////////////////////////////////////
// scoring
/////////////////////////////////////////////////////////////////////////////

void
LK_BallEnergy::setup_d2_bounds()
{
	Real const h2o_radius( 1.4 );
	chemical::AtomTypeSet const & atom_set( *(etable_->atom_set().lock() ) );
	d2_low_.resize( atom_set.n_atomtypes() );
	for ( Size i=1; i<= atom_set.n_atomtypes(); ++i ) {
		chemical::AtomType const & atype( atom_set[ i ] );
		Real const d2_high( numeric::square( h2o_radius + atype.lj_radius() ) ); // was 3.0 * 3.0
		d2_low_[ i ] = std::max( 0.0, d2_high - ramp_width_A2_ );                // was 2.1 * 2.1
		TR.Trace << "d2_low_high: " << atype.name() << ' ' <<
			std::sqrt( d2_low_[i] ) << ' ' << std::sqrt( d2_high ) << std::endl;
	}

	/// which atomtypes are "charged"? Narg, Nlys, OOC (for splitting into a separate score)

	atom_type_is_charged_.resize( atom_set.n_atomtypes() );
	for ( Size i=1; i<= atom_set.n_atomtypes(); ++i ) {
		std::string const & name( atom_set[ i ].name() );
		debug_assert( Size( atom_set.atom_type_index( name ) ) == i ); // sanity
		if ( name == "Narg" || name == "Nlys" || name == "OOC" ) atom_type_is_charged_[ i ] = true;
		else atom_type_is_charged_[ i ] = false;
		TR.Trace << "atom_type_is_charged_: " << name << ' ' << atom_type_is_charged_[i] << std::endl;
	}

	lk_ball_prefactor_.clear();
	lk_ball_prefactor_.resize( atom_set.n_atomtypes(), 1.0 );

	//  if ( options::option[ options::OptionKeys::dna::specificity::lk_ball_prefactor ].user() ) {
	//   utility::vector1< std::string > const lkbp_string
	//    ( options::option[ options::OptionKeys::dna::specificity::lk_ball_prefactor ] );
	//   if ( lkbp_string.size()%2 != 0 ) utility_exit_with_message("lk_ball_prefactor should have even length");
	//   for ( Size ii=0; ii< lkbp_string.size()/2; ++ii ) {
	//    std::string const atom_type_name( lkbp_string[ 2*ii+1 ] );
	//    Real const prefactor( float_of( lkbp_string[ 2*ii+2 ] ) );
	//    lk_ball_prefactor_[ atom_set.atom_type_index( atom_type_name ) ] = prefactor;
	//    TR.Trace << "lk_ball_prefactor: " << atom_type_name << ' ' << prefactor << std::endl;
	//   }
	//  }
}


/// @details  Stolen from LK_SigmoidalFunc in lk_hack
/// d2_delta = d2 - d2_low
///
Real
LK_BallEnergy::eval_lk_fraction( Real const d2_delta ) const
{
	debug_assert( d2_delta >= -0.001 && d2_delta <= ramp_width_A2_ + 0.001 );
	static Real const inv_range( 1.0 / ramp_width_A2_ );
	Real const xprime( inv_range * d2_delta );
	return ( 1 - xprime*xprime ) * ( 1 - xprime*xprime );
}

Real
LK_BallEnergy::eval_d_lk_fraction_dr_over_r( Real const d2_delta ) const
{
	debug_assert( d2_delta >= -0.001 && d2_delta <= ramp_width_A2_ + 0.001 );
	static Real const inv_range( 1.0 / ramp_width_A2_ );
	Real const xprime( inv_range * d2_delta );
	return -8.0 * inv_range * ( 1 - xprime * xprime ) * xprime;

	//Real const xprime_squared( numeric::square( inv_range * ( d2 - d2_low_ ) ) );
	//return -8.0 * ( 1 - xprime_squared ) * xprime_squared;

}

/// @note  closest_water may be set to 0 upon return if none of the waters are within ramp_width_A2_
Real
LK_BallEnergy::get_lk_fractional_contribution(
	Vector const & atom2_xyz,
	Size const atom2_type,
	Vectors const & atom1_waters,
	utility::vector1< Real > & d_weighted_d2_d_di,  // per water contribution
	Real & weighted_d2_water_delta
) const
{
	//Real max_frac( 0.0 ), frac, d2;
	Real const d2_low( d2_low_[ atom2_type ] );

	// softmax of closest water
	d_weighted_d2_d_di.clear();
	d_weighted_d2_d_di.resize( atom1_waters.size(), 0.0 );

	weighted_d2_water_delta = 0.0;
	Real d2_delta;
	for ( Size idx = 1; idx <= atom1_waters.size(); ++idx) {
		d2_delta = atom2_xyz.distance_squared( atom1_waters[idx] ) - d2_low;
		d_weighted_d2_d_di[idx] = exp( -d2_delta/multi_water_fade_ );
		weighted_d2_water_delta += d_weighted_d2_d_di[idx];
		//TR << "d2_delta = " << d2_delta << std::endl;
	}

	for ( Size idx = 1; idx <= atom1_waters.size(); ++idx) {
		d_weighted_d2_d_di[idx] /= weighted_d2_water_delta;
	}

	weighted_d2_water_delta = -multi_water_fade_ * log( weighted_d2_water_delta );
	//TR << "weighted_d2_water_delta = " << weighted_d2_water_delta << std::endl;

	Real frac( 0.0 );
	if ( weighted_d2_water_delta < ramp_width_A2_ ) {
		frac = ( weighted_d2_water_delta < 0.0 ? Real( 1.0 ) : eval_lk_fraction( weighted_d2_water_delta ) );
	}
	//TR << "frac = " << frac << std::endl;

	return frac;
}

//// helper (fpd: out of date!)
Real
LK_BallEnergy::get_lk_fractional_contribution_for_single_water(
	Vector const & atom2_xyz,
	Size const atom2_type,
	Vector const & atom1_water
) const
{
	//Real max_frac( 0.0 ), frac, d2;
	Real const d2_low( d2_low_[ atom2_type ] );

	// find the closest water:
	Real const d2_delta = atom2_xyz.distance_squared( atom1_water ) - d2_low;
	if ( d2_delta > ramp_width_A2_ ) return 0.0;
	else if ( d2_delta < 0.0 ) return 1.0;
	else return eval_lk_fraction( d2_delta );
}


Real
LK_BallEnergy::get_lk_fractional_contribution(
	Vector const & atom2_xyz,
	Size const atom2_type,
	Vectors const & atom1_waters
) const
{
	utility::vector1< Real > dweighted_water_ddi;
	Real weighted_water_d2_delta(0.0);
	return get_lk_fractional_contribution( atom2_xyz, atom2_type, atom1_waters, dweighted_water_ddi, weighted_water_d2_delta );
}


/// This guy is used during scoring if we are not minimizing, or inside linmem_ig packing and things like that...
void
LK_BallEnergy::residue_pair_energy(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	pose::Pose const & pose,
	ScoreFunction const &,
	EnergyMap & emap
) const
{
	/// there might be data stashed in these residues if we came through certain packing routes
	using conformation::residue_datacache::LK_BALL_INFO;
	if ( rsd1.data_ptr() != 0 && rsd1.data_ptr()->has( LK_BALL_INFO ) ) {
		debug_assert( dynamic_cast< LKB_ResidueInfo const * > ( rsd1.data_ptr()->get_raw_const_ptr( LK_BALL_INFO )));
		debug_assert( rsd2.data_ptr() != 0 &&
			rsd2.data_ptr()->get_const_ptr( LK_BALL_INFO ) != 0 &&
			dynamic_cast< LKB_ResidueInfo const * > ( rsd2.data_ptr()->get_raw_const_ptr( LK_BALL_INFO )));

		residue_pair_energy( rsd1,
			*( dynamic_cast< LKB_ResidueInfo const * >( rsd1.data_ptr()->get_raw_const_ptr( LK_BALL_INFO ))),
			rsd2,
			*( dynamic_cast< LKB_ResidueInfo const * >( rsd2.data_ptr()->get_raw_const_ptr( LK_BALL_INFO ))),
			emap );
	} else {
		debug_assert( rsd2.data_ptr() == 0 || !rsd2.data_ptr()->has( LK_BALL_INFO ) );
		LKB_ResidueInfo const & info1( retrieve_lkb_residue_info( pose, rsd1.seqpos() ) );
		LKB_ResidueInfo const & info2( retrieve_lkb_residue_info( pose, rsd2.seqpos() ) );
		residue_pair_energy( rsd1, info1, rsd2, info2, emap );
	}
}

void
LK_BallEnergy::sidechain_sidechain_energy(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	pose::Pose const &, // pose,
	ScoreFunction const &, // sfxn,
	EnergyMap & emap
) const
{
	using conformation::residue_datacache::LK_BALL_INFO;
	/// if we got here we should have come through packing...
	debug_assert( rsd1.data_ptr() != 0 &&
		rsd1.data_ptr()->get_const_ptr( LK_BALL_INFO ) != 0 &&
		dynamic_cast< LKB_ResidueInfo const * > ( rsd1.data_ptr()->get_raw_const_ptr( LK_BALL_INFO )));
	debug_assert( rsd2.data_ptr() != 0 &&
		rsd2.data_ptr()->get_const_ptr( LK_BALL_INFO ) != 0 &&
		dynamic_cast< LKB_ResidueInfo const * > ( rsd2.data_ptr()->get_raw_const_ptr( LK_BALL_INFO )));

	residue_pair_energy( rsd1,
		*( dynamic_cast< LKB_ResidueInfo const * >( rsd1.data_ptr()->get_raw_const_ptr( LK_BALL_INFO ))),
		rsd2,
		*( dynamic_cast< LKB_ResidueInfo const * >( rsd2.data_ptr()->get_raw_const_ptr( LK_BALL_INFO ))),
		emap );
}

// this guy is used during minimization
bool
LK_BallEnergy::use_extended_residue_pair_energy_interface() const
{
	return true;
}
void
LK_BallEnergy::residue_pair_energy_ext(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	ResPairMinimizationData const & pairdata,
	pose::Pose const & pose,
	ScoreFunction const &,
	EnergyMap & emap
) const
{
	if ( pose.energies().use_nblist_auto_update() ) return;

	LKB_ResPairMinData const & lkb_pairdata( retrieve_lkb_pairdata( pairdata ) );

	residue_pair_energy( rsd1, lkb_pairdata.res1_data(), rsd2, lkb_pairdata.res2_data(), emap );
}


/// helper function for outsiders
Real
LK_BallEnergy::calculate_lk_desolvation_of_single_atom_by_residue(
	Size const atom1,
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2
)
{
	using namespace etable::count_pair;
	CountPairFunctionOP cpfxn =
		CountPairFactory::create_count_pair_function( rsd1, rsd2, CP_CROSSOVER_4 );

	// setup residue information
	Vector const & atom1_xyz( rsd1.xyz( atom1 ) );
	Size const atom1_type_index( rsd1.atom( atom1 ).type() );

	Real total_desolvation( 0.0 );
	for ( Size atom2=1; atom2<= rsd2.nheavyatoms(); ++atom2 ) {
		Vector const & atom2_xyz( rsd2.xyz( atom2 ) );

		Real cp_weight = 1.0;
		Size pathdist;
		if ( cpfxn->count( atom1, atom2, cp_weight, pathdist ) ) {
			Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

			if ( ( d2 >= safe_max_dis2_) || ( d2 == Real(0.0) ) ) continue;

			Real lk_desolvation_of_atom1_by_atom2;
			if ( slim_etable_ ) {
				Real lk_desolvation_of_atom2_by_atom1;
				// not sure the order is correct here:
				etable_->analytic_lk_energy( rsd1.atom( atom1 ), rsd2.atom( atom2 ), lk_desolvation_of_atom1_by_atom2,
					lk_desolvation_of_atom2_by_atom1 );
				lk_desolvation_of_atom1_by_atom2 *= cp_weight;
			} else {
				// setup for solvation Etable lookups
				Size const atom2_type_index( rsd2.atom( atom2 ).type() );
				Real const d2_bin = d2 * etable_bins_per_A2_;
				int disbin = static_cast< int >( d2_bin ) + 1;
				Real frac = d2_bin - ( disbin - 1 );
				int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

				lk_desolvation_of_atom1_by_atom2 = cp_weight * ( ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ] );
			}
			total_desolvation += lk_desolvation_of_atom1_by_atom2;
		}
	}
	return total_desolvation;
}

/// get the lk-ball desolvation of atom1 by atom2, and the unoriented lk desolvation of atom1 by atom2

void
LK_BallEnergy::calculate_lk_ball_atom_energies(
	Size const atom1,
	conformation::Residue const & rsd1,
	Vectors const & atom1_waters,
	Size const atom2,
	conformation::Residue const & rsd2,
	Real & lk_desolvation_of_atom1_by_atom2,
	Real & lk_ball_desolvation_of_atom1_by_atom2 // includes lk-fraction
) const
{
	using namespace etable::count_pair;

	// initialize values
	lk_desolvation_of_atom1_by_atom2 = 0.0;
	lk_ball_desolvation_of_atom1_by_atom2 = 0.0;

	CountPairFunctionOP cpfxn =
		CountPairFactory::create_count_pair_function( rsd1, rsd2, CP_CROSSOVER_4 );

	Real cp_weight = 1.0; Size pathdist;
	if ( !cpfxn->count( atom1, atom2, cp_weight, pathdist ) ) return; /// NO COUNTPAIR

	Vector const & atom1_xyz( rsd1.xyz( atom1 ) );
	Vector const & atom2_xyz( rsd2.xyz( atom2 ) );

	Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

	if ( ( d2 >= safe_max_dis2_) || ( d2 == Real(0.0) ) ) return; // TOO FARAWAY (OR SAME ATOM?)

	Size const atom2_type_index( rsd2.atom( atom2 ).type() );
	if ( slim_etable_ ) {
		Real lk_desolvation_of_atom2_by_atom1;
		// note sure the order is correct here:
		etable_->analytic_lk_energy( rsd1.atom( atom1 ), rsd2.atom( atom2 ), lk_desolvation_of_atom1_by_atom2,
			lk_desolvation_of_atom2_by_atom1 );
		lk_desolvation_of_atom1_by_atom2 *= cp_weight;
	} else {
		// setup for solvation Etable lookups
		Size const atom1_type_index( rsd1.atom( atom1 ).type() );

		Real const d2_bin = d2 * etable_bins_per_A2_;
		int disbin = static_cast< int >( d2_bin ) + 1;
		Real frac = d2_bin - ( disbin - 1 );
		int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

		lk_desolvation_of_atom1_by_atom2 = ( cp_weight * ( ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ] ) );
	}
	lk_ball_desolvation_of_atom1_by_atom2 = lk_desolvation_of_atom1_by_atom2 *
		get_lk_fractional_contribution( atom2_xyz, atom2_type_index, atom1_waters );
}


void
LK_BallEnergy::calculate_lk_ball_atom_energies_cp(
	Size const atom1,
	conformation::Residue const & rsd1,
	Vectors const & atom1_waters,
	Size const atom2,
	conformation::Residue const & rsd2,
	etable::count_pair::CPCrossoverBehavior const & cp_crossover,
	Real & lk_desolvation_of_atom1_by_atom2,
	Real & lk_ball_desolvation_of_atom1_by_atom2 // includes lk-fraction
) const
{
	using namespace etable::count_pair;

	// initialize values
	lk_desolvation_of_atom1_by_atom2 = 0.0;
	lk_ball_desolvation_of_atom1_by_atom2 = 0.0;

	CountPairFunctionOP cpfxn
		( rsd1.seqpos() == rsd2.seqpos() ?
		CountPairFactory::create_intrares_count_pair_function( rsd1, cp_crossover ) :
		CountPairFactory::create_count_pair_function( rsd1, rsd2, cp_crossover ) );

	if ( rsd1.seqpos() == rsd2.seqpos() && atom1 == atom2 ) return;

	Real cp_weight = 1.0; Size pathdist;
	if ( !cpfxn->count( atom1, atom2, cp_weight, pathdist ) ) return; /// NO COUNTPAIR

	Vector const & atom1_xyz( rsd1.xyz( atom1 ) );
	Vector const & atom2_xyz( rsd2.xyz( atom2 ) );

	Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

	if ( ( d2 >= safe_max_dis2_) || ( d2 == Real(0.0) ) ) return; // TOO FARAWAY (OR SAME ATOM?)

	Size const atom2_type_index( rsd2.atom( atom2 ).type() );
	if ( slim_etable_ ) {
		Real lk_desolvation_of_atom2_by_atom1;
		// note sure the order is correct here:
		etable_->analytic_lk_energy( rsd1.atom( atom1 ), rsd2.atom( atom2 ), lk_desolvation_of_atom1_by_atom2,
			lk_desolvation_of_atom2_by_atom1 );
		lk_desolvation_of_atom1_by_atom2 *= cp_weight;
	} else {
		// setup for solvation Etable lookups
		Size const atom1_type_index( rsd1.atom( atom1 ).type() );

		Real const d2_bin = d2 * etable_bins_per_A2_;
		int disbin = static_cast< int >( d2_bin ) + 1;
		Real frac = d2_bin - ( disbin - 1 );
		int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

		lk_desolvation_of_atom1_by_atom2 = ( cp_weight * ( ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ] ) );
	}

	lk_ball_desolvation_of_atom1_by_atom2 = lk_desolvation_of_atom1_by_atom2 *
		get_lk_fractional_contribution( atom2_xyz, atom2_type_index, atom1_waters );
}


Real
LK_BallEnergy::calculate_lk_desolvation_of_single_atom_by_residue_no_count_pair(
	Size const atom1,
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2
)
{
	//using namespace etable::count_pair;

	// setup residue information
	Vector const & atom1_xyz( rsd1.xyz( atom1 ) );
	Size const atom1_type_index( rsd1.atom( atom1 ).type() );

	Real total_desolvation( 0.0 );
	for ( Size atom2=1; atom2<= rsd2.nheavyatoms(); ++atom2 ) {
		Vector const & atom2_xyz( rsd2.xyz( atom2 ) );

		Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

		if ( ( d2 >= safe_max_dis2_) || ( d2 < 1e-3 ) ) continue; // exclude self...

		Real lk_desolvation_of_atom1_by_atom2;
		if ( slim_etable_ ) {
			Real lk_desolvation_of_atom2_by_atom1;
			// note sure the order is correct here:
			etable_->analytic_lk_energy( rsd1.atom( atom1 ), rsd2.atom( atom2 ), lk_desolvation_of_atom1_by_atom2,
				lk_desolvation_of_atom2_by_atom1 );
		} else {
			// setup for solvation Etable lookups
			Size const atom2_type_index( rsd2.atom( atom2 ).type() );
			Real const d2_bin = d2 * etable_bins_per_A2_;
			int disbin = static_cast< int >( d2_bin ) + 1;
			Real frac = d2_bin - ( disbin - 1 );
			int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

			lk_desolvation_of_atom1_by_atom2 = ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ];
		}
		total_desolvation += lk_desolvation_of_atom1_by_atom2;

	}
	return total_desolvation;
}

void
LK_BallEnergy::eval_desolvation_derivs_no_count_pair(
	Real const d2,
	Size const atom1,
	conformation::Residue const & rsd1,
	Size const atom2,
	conformation::Residue const & rsd2,
	Real & atom1_lk_desolvation_by_atom2_deriv,
	Real & atom2_lk_desolvation_by_atom1_deriv
)
{
	if ( ( d2 >= safe_max_dis2_) || ( d2 == Real(0.0) ) ) {
		atom1_lk_desolvation_by_atom2_deriv = 0.0;
		atom2_lk_desolvation_by_atom1_deriv = 0.0;
		return;
	}

	if ( slim_etable_ ) {
		Real inv_dis;
		etable_->analytic_lk_derivatives( rsd1.atom( atom1 ), rsd2.atom( atom2 ),
			atom1_lk_desolvation_by_atom2_deriv, atom2_lk_desolvation_by_atom1_deriv, inv_dis );
	} else {
		Real const d2_bin( d2 * etable_bins_per_A2_ );
		Size const disbin( static_cast< int >( d2_bin ) + 1 );
		Real const frac( d2_bin - ( disbin - 1 ) );

		/// this index into solv1 or dsolv1 should represent desolvation of atom1 by atom2
		int const linear_index12( dsolv1_.index( disbin, rsd2.atom( atom2 ).type(), rsd1.atom( atom1 ).type() ) );
		/// this index into solv1 or dsolv1 should represent desolvation of atom2 by atom1
		int const linear_index21( dsolv1_.index( disbin, rsd1.atom( atom1 ).type(), rsd2.atom( atom2 ).type() ) );

		atom1_lk_desolvation_by_atom2_deriv = dsolv1_[ linear_index12 ] * ( 1-frac ) + dsolv1_[ linear_index12 + 1 ] * frac;
		atom2_lk_desolvation_by_atom1_deriv = dsolv1_[ linear_index21 ] * ( 1-frac ) + dsolv1_[ linear_index21 + 1 ] * frac;
	}
}

/// HACKY HELPER FXN TAKEN DIRECTLY FROM BaseEtableEnergy.tmpl.hh
/// IF YOU CHANGE THE BEHAVIOR THERE YOU SHOULD CHANGE HERE AS WELL
///
scoring::etable::count_pair::CPCrossoverBehavior
determine_crossover_behavior(
	conformation::Residue const & res1,
	conformation::Residue const & res2,
	bool const use_intra_dna_cp_crossover_4
	//pose::Pose const &
	//ScoreFunction const & sfxn
)
{
	using namespace scoring::etable::count_pair;
	// maybe should ask "are these two residues polymers and do they share a polymeric bond"
	//debug_assert( !sfxn.has_zero_weight( mm_twist ) );
	if ( res1.polymeric_sequence_distance(res2) == 1 ) {
		if ( ( !( res1.is_protein() && res2.is_protein() ) ) &&
				( !( use_intra_dna_cp_crossover_4 && res1.is_DNA() && res2.is_DNA() ) ) &&
				( !( res1.is_RNA() && res2.is_RNA() ) ) ) {
			return CP_CROSSOVER_3;
		} else {
			return CP_CROSSOVER_4; // peptide bond w/ or w/o rama, but definately w/o mm_twist
		}
	} else if ( res1.seqpos() == res2.seqpos() ) {
		// logic for controlling intra-residue count pair behavior goes here; for now, default to crossover 3
		return CP_CROSSOVER_3;
	} else {
		return CP_CROSSOVER_3; // e.g. disulfides where seqsep != 1
	}
}

void
LK_BallEnergy::accumulate_single_atom_contributions(
	Size const,
	Size const,
	Vectors const & atom1_waters,
	utility::vector1< Real > const & atom1_wts,
	conformation::Residue const &,
	Size const atom2_type_index,
	Vector const & atom2_xyz,
	Real const lk_desolvation_of_atom1_by_atom2,
	EnergyMap & emap
) const
{
	/// for first checkin to trunk, take old approach of only counting things for atoms with waters attached
	/// more logic here in the blab branch, hence existence as separate fxn
	if ( !atom1_waters.empty() ) {
		emap[ lk_ball_iso ] += lk_desolvation_of_atom1_by_atom2;
		Real const lk_desolvation_of_atom1_by_atom2_lkb
			( lk_desolvation_of_atom1_by_atom2 *
			get_lk_fractional_contribution( atom2_xyz, atom2_type_index, atom1_waters ) );
		emap[ lk_ball ] += lk_desolvation_of_atom1_by_atom2_lkb;
		emap[ lk_ball_wtd ] += ( atom1_wts[1] * lk_desolvation_of_atom1_by_atom2 +
			atom1_wts[2] * lk_desolvation_of_atom1_by_atom2_lkb );
	}
}


void
LK_BallEnergy::residue_pair_energy(
	conformation::Residue const & rsd1,
	LKB_ResidueInfo const & rsd1_info,
	conformation::Residue const & rsd2,
	LKB_ResidueInfo const & rsd2_info,
	EnergyMap & emap
) const
{
	//PROF_START( basic::LK_BALL_RESIDUE_PAIR_ENERGY );

	using namespace etable::count_pair;
	//bool const verbose( false );
	utility::vector1< Vectors > const & rsd1_waters( rsd1_info.waters() );
	utility::vector1< Vectors > const & rsd2_waters( rsd2_info.waters() );

	utility::vector1< utility::vector1< Real > > const & rsd1_atom_wts( rsd1_info.atom_weights() );
	utility::vector1< utility::vector1< Real > > const & rsd2_atom_wts( rsd2_info.atom_weights() );


	static Size counter(0);
	++counter;
#ifndef NDEBUG // every time
	counter=0;
#endif
	if ( counter%10000 == 0 ) { // total hack
		LKB_ResidueInfo info1b( rsd1 );
		utility::vector1< Vectors > const & rsd1_waters_new( info1b.waters() );

		runtime_assert( rsd1_waters.size() == rsd1_waters_new.size() );

		for ( Size i=1; i<= rsd1.nheavyatoms(); ++i ) {
			if ( !rsd1_waters[i].empty() ) {
				runtime_assert( rsd1_waters[i].size() == rsd1_waters_new[i].size() );
				for ( Size j=1; j<= rsd1_waters[i].size(); ++j ) {
					Real const dis2( rsd1_waters[i][j].distance_squared( rsd1_waters_new[i][j] ) );
					if ( dis2>1e-3 ) {
						std::cout << "LKB-bigdis2: " << rsd1.name() << ' ' << rsd2.name() << ' ' << std::sqrt( dis2 ) << std::endl;
						std::cerr << "LKB-bigdis2: " << rsd1.name() << ' ' << rsd2.name() << ' ' << std::sqrt( dis2 ) << std::endl;
						debug_assert(dis2<1e-3);
						utility_exit();
					}
				}
			}
		}
	}


	// the old way:
	//
	//CountPairFunctionOP cpfxn =
	// CountPairFactory::create_count_pair_function( rsd1, rsd2, CP_CROSSOVER_4 );

	CPCrossoverBehavior crossover = determine_crossover_behavior( rsd1, rsd2, use_intra_dna_cp_crossover_4_ ); //, pose ); //, sfxn );
	CountPairFunctionOP cpfxn = CountPairFactory::create_count_pair_function( rsd1, rsd2, crossover );

	// setup residue information
	for ( Size atom1=1; atom1<= rsd1.nheavyatoms(); ++atom1 ) {
		Vectors const & atom1_waters( rsd1_waters[ atom1 ] );
		Vector const & atom1_xyz( rsd1.xyz( atom1 ) );
		Size const atom1_type_index( rsd1.atom( atom1 ).type() );
		//chemical::AtomType const & atom1_type( rsd1.atom_type( atom1 ) );
		utility::vector1< Real > const & atom1_weights( rsd1_atom_wts[atom1] );

		for ( Size atom2=1; atom2<= rsd2.nheavyatoms(); ++atom2 ) {
			Vectors const & atom2_waters( rsd2_waters[ atom2 ] );
			Vector const & atom2_xyz( rsd2.xyz( atom2 ) );
			//chemical::AtomType const & atom2_type( rsd2.atom_type( atom2 ) );

			if ( atom1_waters.empty() && atom2_waters.empty() ) continue;

			utility::vector1< Real > const & atom2_weights( rsd2_atom_wts[atom2] );

			Real cp_weight = 1.0; Size pathdist;
			if ( cpfxn->count( atom1, atom2, cp_weight, pathdist ) ) {
				Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

				if ( ( d2 >= safe_max_dis2_) || ( d2 == Real(0.0) ) ) continue;

				Real lk_desolvation_of_atom1_by_atom2, lk_desolvation_of_atom2_by_atom1;
				Size const atom2_type_index( rsd2.atom( atom2 ).type() );
				if ( slim_etable_ ) {
					etable_->analytic_lk_energy( rsd1.atom( atom1 ), rsd2.atom( atom2 ), lk_desolvation_of_atom1_by_atom2,
						lk_desolvation_of_atom2_by_atom1 );
					lk_desolvation_of_atom1_by_atom2 *= cp_weight;
					lk_desolvation_of_atom2_by_atom1 *= cp_weight;

				} else {
					// setup for solvation Etable lookups
					Real const d2_bin = d2 * etable_bins_per_A2_;
					int disbin = static_cast< int >( d2_bin ) + 1;
					Real frac = d2_bin - ( disbin - 1 );
					int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

					lk_desolvation_of_atom1_by_atom2 = cp_weight * ( ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ] );
					lk_desolvation_of_atom2_by_atom1 = cp_weight * ( ( 1. - frac ) * solv2_[ l1 ] + frac * solv2_[ l1+1 ] );
				}
				//TR << "pair " << rsd1.seqpos() << "." << atom1 << " : " << rsd2.seqpos() << "." << atom2 << std::endl;
				accumulate_single_atom_contributions( atom1, atom1_type_index, atom1_waters, atom1_weights,
					rsd1, atom2_type_index, atom2_xyz,
					lk_desolvation_of_atom1_by_atom2, emap );

				accumulate_single_atom_contributions( atom2, atom2_type_index, atom2_waters, atom2_weights,
					rsd2, atom1_type_index, atom1_xyz,
					lk_desolvation_of_atom2_by_atom1, emap );

			} // count pair
		} // atom2
	} // atom1


	//PROF_STOP( basic::LK_BALL_RESIDUE_PAIR_ENERGY );
}


///I AM HAVING A SLEEPOVER


/////////////////////////////////////////////////////////////////////////////
// derivatives
/////////////////////////////////////////////////////////////////////////////


void
LK_BallEnergy::setup_for_derivatives(
	pose::Pose & pose,
	ScoreFunction const &
) const
{
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
	pose.update_residue_neighbors();
	compute_and_store_pose_waters( pose );
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
}


/**

Note that we calculate the lk_ball_iso derivative as well as the lk_ball derivative...

Derivatives are only included for heavyatoms.

For a non-polar heavyatom, the derivs are for all polar atoms that it desolvates

For a polar heavyatom, derivs are for all polar atoms that it desolvates as well as all atoms it's being
desolvated by.

Given an atom desolvating a polar atom:
* the lk_ball_iso deriv is the standard lk deriv, but make sure we use the correct array! (see LK_hack code)

* the lk_ball score = wt * lk_polar, so the derivs have two components. One looks like wt * lk_polar deriv
contribution. The other looks like the lk_polar term * the derivative of the wt. The derivative of the wt
is found by getting the closest water, taking the derivative of the wt term wrt distance and using f1/f2
contributions for the desolvating atom xyz and the water xyz.

**/

/// @details  Compute the f1 and f2 vectors for the derivative of the lk_fraction term for atom2 desolvating atom1
///
/// @note f1 and f2 are zeroed and filled within this function, rather than being accumulated into
/// @note We pretend that atom1 is the "moving" atom
/// Should eliminate code duplication with next routine... currently this is for outside use.

Real
LK_BallEnergy::eval_lk_ball_fraction_deriv(
	Vector const & atom2_xyz,
	Size const atom2_type,
	Vectors const & atom1_waters,
	bool const evaluate_deriv,
	Vector & f1,
	Vector & f2
) const
{
	///
	Real const d2_low( d2_low_[ atom2_type ] );

	if ( evaluate_deriv ) {
		f1.clear(); f2.clear();
	}

	// water1-atom2 interaction
	// note that there's no derivative unless we're in the ramping zone:
	// if ( closest_water_d2_delta < ramp_width_A2_ && closest_water_d2_delta > 0.0 ) {
	//  Vector const & atom1_water_xyz( atom1_waters[ closest_water ] );
	//  // what is the derivative of the lk_fraction term wrt r?
	//  Real const dE_dr_over_r( eval_d_lk_fraction_dr_over_r( closest_water_d2_delta ) );
	//  f1 = dE_dr_over_r * ( atom1_water_xyz.cross( atom2_xyz ) );
	//  f2 = dE_dr_over_r * ( atom1_water_xyz - atom2_xyz );
	// }

	//fpd use softmax of closest water
	//   dlk/dx = sum(dlk/dd * dd/dx)
	Real weighted_d2_water_delta = 0.0;
	Real d2_delta;
	for ( Vectors::const_iterator water= atom1_waters.begin(), water_end= atom1_waters.end();
			water != water_end; ++water ) {
		d2_delta = atom2_xyz.distance_squared( *water ) - d2_low;

		if ( d2_delta > ramp_width_A2_ || d2_delta < 0 ) continue;

		Real scale = exp( -d2_delta/multi_water_fade_ );
		weighted_d2_water_delta += scale;

		Vector const & atom1_water_xyz( *water );

		// compute dd/dx here
		if ( evaluate_deriv ) {
			f1 += scale * ( atom1_water_xyz.cross( atom2_xyz ) );
			f2 += scale * ( atom1_water_xyz - atom2_xyz );
		}
	}

	// compute dlk/dd here
	Real const dE_dr_over_r( eval_d_lk_fraction_dr_over_r( weighted_d2_water_delta ) );
	if ( evaluate_deriv && weighted_d2_water_delta>0 && weighted_d2_water_delta<ramp_width_A2_ ) {
		f1 = f1 * (dE_dr_over_r / weighted_d2_water_delta);
		f2 = f2 * (dE_dr_over_r / weighted_d2_water_delta);
	}

	Real frac = ( weighted_d2_water_delta <= 0.0 ? Real( 1.0 ) : eval_lk_fraction( weighted_d2_water_delta ) );

	return frac;
}


/// @note  atom2 is desolvating atom1. atom1_waters is non-empty
/// @note  Pretend that atom1 is the atom whose derivs are being calculated. weight_factor may include -1 term
/// to switch the order...
///
void
LK_BallEnergy::sum_deriv_contributions_for_heavyatom_pair_one_way(
	Size const heavyatom1,
	conformation::Residue const & rsd1,
	LKB_ResidueInfo const & rsd1_info,
	Size const heavyatom2,
	conformation::Residue const & rsd2,
	EnergyMap const & weights,
	Real const weight_factor,
	Real const d2,
	utility::vector1< DerivVectorPair > & r1_at_derivs,
	utility::vector1< DerivVectorPair > & r2_at_derivs
) const
{
	Vectors const & atom1_waters = rsd1_info.waters()[heavyatom1];
	utility::vector1< Real > const & atom1_wts = rsd1_info.atom_weights()[heavyatom1];

	if ( atom1_waters.empty() ) return;

	if ( atom1_wts.size() != 2 ) {
		std::cerr << "LK_BallEnergy: bad atom1_wts " << rsd1.name() << ' ' << rsd1.atom_name( heavyatom1 ) << ' ' <<
			atom1_wts.size() << std::endl;
		utility_exit_with_message("bad atom1_wts: "+rsd1.name()+" "+rsd1.atom_name(heavyatom1));
	}

	Real lk_deriv, lk_score;

	if ( slim_etable_ ) {
		Real other_lk_score, other_lk_deriv, inv_dis;
		etable_->analytic_lk_energy( rsd1.atom( heavyatom1 ), rsd2.atom( heavyatom2 ), lk_score, other_lk_score );
		etable_->analytic_lk_derivatives( rsd1.atom( heavyatom1 ), rsd2.atom( heavyatom2 ), lk_deriv, other_lk_deriv, inv_dis );
	} else {
		Real const d2_bin( d2 * etable_bins_per_A2_ );
		Size const disbin( static_cast< int >( d2_bin ) + 1 );
		Real const frac( d2_bin - ( disbin - 1 ) );

		/// this index into solv1 or dsolv1 should represent desolvation of heavyatom1 by atom2
		Size const heavyatom1_type_index( rsd1.atom( heavyatom1 ).type() );
		int const linear_index( dsolv1_.index( disbin, rsd2.atom( heavyatom2 ).type(), heavyatom1_type_index ) );

		lk_deriv = dsolv1_[ linear_index ] * ( 1-frac ) + dsolv1_[ linear_index+1 ] * frac;
		lk_score =  solv1_[ linear_index ] * ( 1-frac ) +  solv1_[ linear_index+1 ] * frac;
	}

	// get weights
	Real const // include contributions from 3 score types:  lk_ball_iso, lk_ball, and lk_ball_wtd
		lk_ball_iso_weight  ( weights[ lk_ball_iso ] + atom1_wts[1] * weights[ lk_ball_wtd ] ), // HACKING
		lk_ball_aniso_weight( weights[ lk_ball     ] + atom1_wts[2] * weights[ lk_ball_wtd ] );

	Vector const & heavyatom1_xyz( rsd1.xyz( heavyatom1 ) );
	Vector const & atom2_xyz( rsd2.xyz( heavyatom2 ) );
	Vector f1( heavyatom1_xyz.cross( atom2_xyz ) ), f2( heavyatom1_xyz - atom2_xyz );
	Real const inv_dis( 1.0 / std::sqrt( d2 ) );

	// now the lk_ball deriv
	//
	// lk_ball = fraction * lk
	// so d lk_ball = fraction * ( d lk ) + ( d fraction ) * lk
	// fpd with softmax:
	//              = fraction * (d lk / d dsum * d dsum/d di ) + ( d fraction / d dsum * d dsum/d di ) * lk
	Real weighted_water_d2(0);
	utility::vector1< core::Real > dwwd2_ddi;
	Real const lk_fraction(
		get_lk_fractional_contribution( atom2_xyz, rsd2.atom( heavyatom2 ).type(), atom1_waters, dwwd2_ddi, weighted_water_d2 ) );

	// (1) the derivs for the parts that don't involve waters:
	{
		Real const dE_dr_over_r( weight_factor * lk_ball_iso_weight * lk_deriv * inv_dis );
		r1_at_derivs[heavyatom1].f1() += dE_dr_over_r * f1;
		r1_at_derivs[heavyatom1].f2() += dE_dr_over_r * f2;
		r2_at_derivs[heavyatom2].f1() -= dE_dr_over_r * f1;
		r2_at_derivs[heavyatom2].f2() -= dE_dr_over_r * f2;
	}

	// (2) atom1-atom2 interaction
	{
		Real const dE_dr_over_r( weight_factor * lk_ball_aniso_weight * lk_fraction * lk_deriv * inv_dis );
		r1_at_derivs[heavyatom1].f1() += dE_dr_over_r * f1;
		r1_at_derivs[heavyatom1].f2() += dE_dr_over_r * f2;
		r2_at_derivs[heavyatom2].f1() -= dE_dr_over_r * f1;
		r2_at_derivs[heavyatom2].f2() -= dE_dr_over_r * f2;
	}

	// (3) waters-atom2 interaction
	// note that there's no derivative unless we're in the ramping zone:
	// we only do this if this term hasn't already been captured by one of our dependent hydrogens
	// we assume that for heavyatoms with dependent polar hydrogens, every water belongs to one of the hydrogens
	if ( weighted_water_d2 < ramp_width_A2_ && weighted_water_d2 > 0.0 ) {
		for (Size i=1; i<=atom1_waters.size(); ++i) {
			Vector const & atom1_water_xyz( atom1_waters[ i ] );

			// update f1 and f2 to reflect water-atom2 as the interaction
			f1 = atom1_water_xyz.cross( atom2_xyz );
			f2 = atom1_water_xyz - atom2_xyz;

			// what is the derivative of the lk_fraction term wrt r?
			Real const dE_dr_over_r
				( weight_factor * lk_ball_aniso_weight * lk_score * dwwd2_ddi[i] * eval_d_lk_fraction_dr_over_r( weighted_water_d2 ) );

			// Derivatives for the desolvating atom
			r2_at_derivs[heavyatom2].f1() -= dE_dr_over_r * f1;
			r2_at_derivs[heavyatom2].f2() -= dE_dr_over_r * f2;

			// derivatives for the desolvated atoms
			// dR/datom1 = dR/dwater * dwater/datom1
			//core::Real dwater = f2.length();
			//Real denom = dwater*dwater*dwater;
			{
				Size atom1 = rsd1_info.get_water_builder( rsd1 , heavyatom1 )[i].atom1();
				Vector const & r1_atom1_xyz( rsd1.xyz( atom1 ) );

				numeric::xyzMatrix< Real >const & dwater_datom1 = rsd1_info.atom1_derivs()[heavyatom1][i];
				Vector dwater_datom1x ( dwater_datom1(1,1), dwater_datom1(2,1), dwater_datom1(3,1) );
				Vector dwater_datom1y ( dwater_datom1(1,2), dwater_datom1(2,2), dwater_datom1(3,2) );
				Vector dwater_datom1z ( dwater_datom1(1,3), dwater_datom1(2,3), dwater_datom1(3,3) );
				Vector dRdatom( f2.dot( dwater_datom1x ), f2.dot( dwater_datom1y ), f2.dot( dwater_datom1z ) );

				Vector f2t = dRdatom;
				Vector f1t = r1_atom1_xyz.cross( r1_atom1_xyz - dRdatom );

				r1_at_derivs[atom1].f1() += dE_dr_over_r * f1t;
				r1_at_derivs[atom1].f2() += dE_dr_over_r * f2t;
			}
			{
				Size atom2 = rsd1_info.get_water_builder( rsd1 , heavyatom1 )[i].atom2();
				Vector const & r1_atom2_xyz( rsd1.xyz( atom2 ) );

				numeric::xyzMatrix< Real >const & dwater_datom2 = rsd1_info.atom2_derivs()[heavyatom1][i];
				Vector dwater_datom2x ( dwater_datom2(1,1), dwater_datom2(2,1), dwater_datom2(3,1) );
				Vector dwater_datom2y ( dwater_datom2(1,2), dwater_datom2(2,2), dwater_datom2(3,2) );
				Vector dwater_datom2z ( dwater_datom2(1,3), dwater_datom2(2,3), dwater_datom2(3,3) );
				Vector dRdatom( f2.dot( dwater_datom2x ), f2.dot( dwater_datom2y ), f2.dot( dwater_datom2z ) );

				Vector f2t = dRdatom;
				Vector f1t = r1_atom2_xyz.cross( r1_atom2_xyz - dRdatom );

				r1_at_derivs[atom2].f1() += dE_dr_over_r * f1t;
				r1_at_derivs[atom2].f2() += dE_dr_over_r * f2t;
			}
			{
				Size atom3 = rsd1_info.get_water_builder( rsd1 , heavyatom1 )[i].atom3();
				Vector const & r1_atom3_xyz( rsd1.xyz( atom3 ) );

				numeric::xyzMatrix< Real >const & dwater_datom3 = rsd1_info.atom3_derivs()[heavyatom1][i];
				Vector dwater_datom3x ( dwater_datom3(1,1), dwater_datom3(2,1), dwater_datom3(3,1) );
				Vector dwater_datom3y ( dwater_datom3(1,2), dwater_datom3(2,2), dwater_datom3(3,2) );
				Vector dwater_datom3z ( dwater_datom3(1,3), dwater_datom3(2,3), dwater_datom3(3,3) );
				Vector dRdatom( f2.dot( dwater_datom3x ), f2.dot( dwater_datom3y ), f2.dot( dwater_datom3z ) );

				Vector f2t = dRdatom;
				Vector f1t = r1_atom3_xyz.cross( r1_atom3_xyz - dRdatom );

				r1_at_derivs[atom3].f1() += dE_dr_over_r * f1t;
				r1_at_derivs[atom3].f2() += dE_dr_over_r * f2t;
			}
		}
	}
}

/// @note  Assumes that atom1 is the "moving" atom, ie the atom for which eval_atom_derivative was called
/// @note  Calculates the water positions for atom2 if d2 < safe_max_dis2
void
LK_BallEnergy::sum_deriv_contributions_for_heavyatom_pair(
	Real const d2,
	Size const heavyatom1,
	conformation::Residue const & rsd1,
	LKB_ResidueInfo const & rsd1_info,
	Size const heavyatom2,
	conformation::Residue const & rsd2,
	LKB_ResidueInfo const & rsd2_info,
	pose::Pose const &,
	EnergyMap const & weights,
	Real const cp_weight,
	utility::vector1< DerivVectorPair > & r1_at_derivs,
	utility::vector1< DerivVectorPair > & r2_at_derivs
) const
{

	sum_deriv_contributions_for_heavyatom_pair_one_way( heavyatom1, rsd1, rsd1_info, heavyatom2, rsd2, weights,
		cp_weight, d2, r1_at_derivs, r2_at_derivs );

	sum_deriv_contributions_for_heavyatom_pair_one_way( heavyatom2, rsd2, rsd2_info, heavyatom1, rsd1, weights,
		cp_weight, d2, r2_at_derivs, r1_at_derivs );

}

void
LK_BallEnergy::indicate_required_context_graphs(
	utility::vector1< bool > & /* context_graphs_required */ ) const
{}


void
LK_BallEnergy::evaluate_rotamer_pair_energies(
	conformation::RotamerSetBase const & set1,
	conformation::RotamerSetBase const & set2,
	pose::Pose const &, // pose,
	ScoreFunction const &, // sfxn,
	EnergyMap const & weights,
	ObjexxFCL::FArray2D< core::PackerEnergy > & energy_table
) const
{
	using namespace conformation;
	using namespace numeric;

	//TR.Trace << "rotamer_pair" << std::endl;

	//PROF_START( basic::LK_BALL_ROTAMER_PAIR_ENERGIES );

	LKB_RotamerSetInfo const & info1( retrieve_lkb_rotamer_set_info( set1 ) );
	LKB_RotamerSetInfo const & info2( retrieve_lkb_rotamer_set_info( set2 ) );


	for ( Size ii = 1; ii <= set1.get_n_residue_types(); ++ii ) {
		Size const ii_offset = set1.get_residue_type_begin( ii );
		for ( Size jj = 1; jj <= set2.get_n_residue_types(); ++jj ) {
			Size const jj_offset = set2.get_residue_type_begin( jj );
			if ( !info1[ ii_offset ].has_waters() && !info2[ jj_offset ].has_waters() ) continue;

			for ( Size kk = 1, kke = set1.get_n_rotamers_for_residue_type( ii ); kk <= kke; ++kk ) {
				Size const kk_rot_id = ii_offset + kk - 1;
				for ( Size ll = 1, lle = set2.get_n_rotamers_for_residue_type( jj ); ll <= lle; ++ll ) {
					Size const ll_rot_id = jj_offset + ll - 1;
					EnergyMap emap;
					residue_pair_energy( *set1.rotamer( kk_rot_id ), info1[ kk_rot_id ],
						*set2.rotamer( ll_rot_id ), info2[ ll_rot_id ], emap );

					energy_table( ll_rot_id, kk_rot_id ) += static_cast< core::PackerEnergy >( weights.dot( emap ) );
				}
			}
		}
	}
	//PROF_STOP( basic::LK_BALL_ROTAMER_PAIR_ENERGIES );
}

void
LK_BallEnergy::evaluate_rotamer_background_energies(
	conformation::RotamerSetBase const & set,
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	ScoreFunction const &, // sfxn,
	EnergyMap const & weights,
	utility::vector1< core::PackerEnergy > & energy_vector
) const
{

	//PROF_START( basic::LK_BALL_ROTAMER_BACKGROUND_ENERGIES );
	//TR.Trace << "rotamer_background!" << std::endl;

	using conformation::Residue;
	LKB_ResidueInfo const & rsd_info( retrieve_lkb_residue_info( pose, rsd.seqpos() ) );
	LKB_RotamerSetInfo const & set_info( retrieve_lkb_rotamer_set_info( set ) );

	for ( Size ii = 1; ii <= set.get_n_residue_types(); ++ii ) {
		Size const ii_offset = set.get_residue_type_begin( ii );

		if ( !set_info[ ii_offset ].has_waters() && !rsd_info.has_waters() ) continue;

		for ( Size kk = 1, kke = set.get_n_rotamers_for_residue_type( ii ); kk <= kke; ++kk ) {
			Size const kk_rot_id = ii_offset + kk - 1;

			EnergyMap emap;
			residue_pair_energy( *set.rotamer( kk_rot_id ), set_info[ kk_rot_id ], rsd, rsd_info, emap );

			/**{ // check that this agrees with the old way!
			// explicitly build the residue_info's now
			LKB_ResidueInfo const info1_redo( *set.rotamer( kk_rot_id ) );
			LKB_ResidueInfo const info2_redo( rsd );
			Real lk_ball_energy_redo( 0.0 ), lk_ball_iso_energy_redo( 0.0 );
			residue_pair_energy( *set.rotamer( kk_rot_id ), info1_redo.waters(),
			rsd, info2_redo.waters(),
			lk_ball_energy_redo, lk_ball_iso_energy_redo );
			debug_assert( std::abs( lk_ball_energy - lk_ball_energy_redo ) +
			std::abs( lk_ball_iso_energy - lk_ball_iso_energy_redo ) < 1e-3 );

			}**/

			energy_vector[ kk_rot_id ] += static_cast< core::PackerEnergy >( weights.dot( emap ) );
		} // kk - rotamers for residue types
	} // ii - residue types for rotamer set

	//PROF_STOP( basic::LK_BALL_ROTAMER_BACKGROUND_ENERGIES );
}


void
LK_BallEnergy::eval_residue_pair_derivatives(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	ResSingleMinimizationData const & res1data,
	ResSingleMinimizationData const & res2data,
	ResPairMinimizationData const & min_data,
	pose::Pose const & pose, // provides context
	EnergyMap const & weights,
	utility::vector1< DerivVectorPair > & r1_at_derivs,
	utility::vector1< DerivVectorPair > & r2_at_derivs
) const
{
	if ( pose.energies().use_nblist_auto_update() ) return;

	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
	debug_assert( r1_at_derivs.size() >= rsd1.natoms() );
	debug_assert( r2_at_derivs.size() >= rsd2.natoms() );

	// retrieve some info
	// LKB_ResidueInfo const & rsd1_info( retrieve_lkb_residue_info( pose, rsd1.seqpos() ) );
	// LKB_ResidueInfo const & rsd2_info( retrieve_lkb_residue_info( pose, rsd2.seqpos() ) );
	LKB_ResidueInfo const & rsd1_info( retrieve_lkb_resdata( res1data ) );
	LKB_ResidueInfo const & rsd2_info( retrieve_lkb_resdata( res2data ) );

	//debug_assert( dynamic_cast< ResiduePairNeighborList const * > (&*min_data.get_data( etab_pair_nblist )));
	//ResiduePairNeighborList const & nblist
	// ( static_cast< ResiduePairNeighborList const & > (min_data.get_data_ref( etab_pair_nblist )) );

	ResiduePairNeighborList const & nblist =
		static_cast< ResiduePairNeighborList const & > (min_data.get_data_ref( etab_pair_nblist ));

	utility::vector1< SmallAtNb > const & neighbs( nblist.atom_neighbors() );
	for ( Size ii = 1, iiend = neighbs.size(); ii <= iiend; ++ii ) {
		Size const heavyatom1( neighbs[ ii ].atomno1() ), heavyatom2( neighbs[ ii ].atomno2() );
		if ( rsd1.atom_is_hydrogen( heavyatom1 ) || rsd2.atom_is_hydrogen( heavyatom2 ) ) continue;
		Real const cp_weight( neighbs[ ii ].weight() );

		Real const d2( rsd1.xyz( heavyatom1 ).distance_squared( rsd2.xyz( heavyatom2 ) ) );

		if ( ( d2 >= safe_max_dis2_ ) || ( d2 == Real(0.0) ) ) continue; // no contribution

		// fpd ... new version works at the heavyatom level
		sum_deriv_contributions_for_heavyatom_pair(
			d2, heavyatom1, rsd1, rsd1_info, heavyatom2, rsd2, rsd2_info, pose, weights, cp_weight, r1_at_derivs, r2_at_derivs );
	}
	//std::cout << "LK_BallEnergy.cc: " << __LINE__ << std::endl;
}


//
//fpd this function is only called during nblist auto update
void
LK_BallEnergy::finalize_total_energy(
	pose::Pose & pose,
	ScoreFunction const &,
	EnergyMap & totals
) const
{
	if ( pose.energies().use_nblist() && pose.energies().use_nblist_auto_update() ) {
		EnergyMap tbenergy_map;
		// add in contributions from the nblist atom-pairs
		NeighborList const & nblist ( pose.energies().nblist( EnergiesCacheableDataType::ETABLE_NBLIST ) );
		nblist.check_domain_map( pose.energies().domain_map() );

		/// Trick to avoid calls to Conformation::residue()
		utility::vector1< conformation::Residue const * > resvect;
		resvect.reserve( pose.total_residue() );
		for ( Size ii = 1; ii <= pose.total_residue(); ++ii ) {
			resvect.push_back( & pose.residue( ii ) );
		}

		for ( Size i=1, i_end = pose.total_residue(); i<= i_end; ++i ) {
			conformation::Residue const & rsd1( *resvect[i] );
			LKB_ResidueInfo const & rsd1_info( retrieve_lkb_residue_info( pose, rsd1.seqpos() ) );
			utility::vector1< Vectors > const & rsd1_waters( rsd1_info.waters() );
			utility::vector1< utility::vector1< Real > > const & rsd1_atom_wts( rsd1_info.atom_weights() );

			for ( Size ii=1, ii_end=rsd1.nheavyatoms(); ii<= ii_end; ++ii ) {
				Vectors const & atom1_waters( rsd1_waters[ ii ] );
				Vector const & atom1_xyz( rsd1.xyz( ii ) );
				Size const atom1_type_index( rsd1.atom( ii ).type() );
				utility::vector1< Real > const & atom1_weights( rsd1_atom_wts[ii] );

				AtomNeighbors const & nbrs( nblist.upper_atom_neighbors(i,ii) );
				for ( AtomNeighbors::const_iterator nbr=nbrs.begin(),
						nbr_end=nbrs.end(); nbr!= nbr_end; ++nbr ) {
					Size const  j( nbr->rsd() );
					Size const jj( nbr->atomno() );

					conformation::Residue const & rsd2( *resvect[j] );

					if ( rsd2.atom_is_hydrogen(jj) ) continue;
					if ( i == j ) continue;

					LKB_ResidueInfo const & rsd2_info( retrieve_lkb_residue_info( pose, rsd2.seqpos() ) );
					utility::vector1< Vectors > const & rsd2_waters( rsd2_info.waters() );
					utility::vector1< utility::vector1< Real > > const & rsd2_atom_wts( rsd2_info.atom_weights() );
					utility::vector1< Real > const & atom2_weights( rsd2_atom_wts[jj] );

					Vectors const & atom2_waters( rsd2_waters[ jj ] );
					Vector const & atom2_xyz( rsd2.xyz( jj ) );

					if ( atom1_waters.empty() && atom2_waters.empty() ) continue;

					Real const cp_weight( nbr->weight_func()*nbr->weight() );  // fpd is this correct (same CP as etable)

					Real const d2( atom1_xyz.distance_squared( atom2_xyz ) );

					Real lk_desolvation_of_atom1_by_atom2, lk_desolvation_of_atom2_by_atom1;
					Size const atom2_type_index( rsd2.atom( jj ).type() );
					if ( slim_etable_ ) {
						etable_->analytic_lk_energy( rsd1.atom( ii ), rsd2.atom( jj ), lk_desolvation_of_atom1_by_atom2,
							lk_desolvation_of_atom2_by_atom1 );
						lk_desolvation_of_atom1_by_atom2 *= cp_weight;
						lk_desolvation_of_atom2_by_atom1 *= cp_weight;

					} else {
						// setup for solvation Etable lookups
						Real const d2_bin = d2 * etable_bins_per_A2_;
						int disbin = static_cast< int >( d2_bin ) + 1;
						Real frac = d2_bin - ( disbin - 1 );
						int const l1 = solv1_.index( disbin, atom2_type_index, atom1_type_index );

						lk_desolvation_of_atom1_by_atom2 = cp_weight * ( ( 1. - frac ) * solv1_[ l1 ] + frac * solv1_[ l1+1 ] );
						lk_desolvation_of_atom2_by_atom1 = cp_weight * ( ( 1. - frac ) * solv2_[ l1 ] + frac * solv2_[ l1+1 ] );
					}

					accumulate_single_atom_contributions( ii, atom1_type_index, atom1_waters, atom1_weights,
						rsd1, atom2_type_index, atom2_xyz,
						lk_desolvation_of_atom1_by_atom2, tbenergy_map );

					accumulate_single_atom_contributions( jj, atom2_type_index, atom2_waters, atom2_weights,
						rsd2, atom1_type_index, atom1_xyz,
						lk_desolvation_of_atom2_by_atom1, tbenergy_map );
				}
			}
		}
		totals += tbenergy_map;
	}

}


//
//fpd  this function is only called during during minimization when nblist auto update is on
//fpd   there is some redundancy in this function as some terms are computed multiple times
void
LK_BallEnergy::eval_atom_derivative(
	id::AtomID const & id,
	pose::Pose const & pose,
	kinematics::DomainMap const &, // domain_map,
	ScoreFunction const & , //sfxn, // needed for non-nblist minimization
	EnergyMap const & weights,
	Vector & F1,
	Vector & F2
) const
{
	Size const idresid = id.rsd();

	conformation::Residue const & rsd1( pose.residue(idresid) );
	Size const atom1( id.atomno() );

	if ( pose.energies().use_nblist() ) {
		LKB_ResidueInfo const & rsd1_info( retrieve_lkb_residue_info( pose, rsd1.seqpos() ) );
		scoring::AtomNeighbors const & nbrs ( pose.energies().nblist( EnergiesCacheableDataType::ETABLE_NBLIST ).atom_neighbors( id ) );
		// part 1: all polar atoms desolvated by this atom (heavyatoms only)
		if (!rsd1.atom_is_hydrogen( atom1 )) {
			for ( scoring::AtomNeighbors::const_iterator it2=nbrs.begin(), it2e=nbrs.end(); it2 != it2e; ++it2 ) {
				scoring::AtomNeighbor const & nbr( *it2 );
				Size const nbrresid = nbr.rsd();
				Real const cp_weight( nbr.weight() );  // do not use nbr->weight_func() here
				if (nbrresid == idresid) continue; // no intra

				conformation::Residue const & rsd2( pose.residue(nbrresid) );
				LKB_ResidueInfo const & rsd2_info( retrieve_lkb_residue_info( pose, nbrresid ) );

				Size const heavyatom2( nbr.atomno() );

				if ( rsd2.atom_is_hydrogen( heavyatom2 ) ) continue;
				Real const d2( rsd1.xyz( atom1 ).distance_squared( rsd2.xyz( heavyatom2 ) ) );

				utility::vector1< DerivVectorPair > r1_at_derivs(rsd1.natoms()), r2_at_derivs(rsd2.natoms());
				sum_deriv_contributions_for_heavyatom_pair_one_way(
					heavyatom2, rsd2, rsd2_info, atom1, rsd1, weights, cp_weight, d2, r2_at_derivs, r1_at_derivs );

				F1 += r1_at_derivs[atom1].f1();
				F2 += r1_at_derivs[atom1].f2();
			}
		}

		// part 2: all polar groups on rsd1 this atom is a part of
		for (Size i=1; i<=rsd1.nheavyatoms(); ++i) {
			Size nwaters = rsd1_info.waters()[i].size();
			if (nwaters == 0) continue;

			WaterBuilders const &	builder = rsd1_info.get_water_builder( rsd1 , i );

			bool need_to_calculate = false;
			for (Size j=1; j<=nwaters && !need_to_calculate; ++j) {
				if (builder[j].atom1() == atom1 || builder[j].atom2() == atom1 || builder[j].atom3() == atom1) {
					need_to_calculate = true;
				}
			}

			if (!need_to_calculate) continue;

			scoring::AtomNeighbors const & nbrs2 (
				pose.energies().nblist( EnergiesCacheableDataType::ETABLE_NBLIST ).atom_neighbors( id::AtomID( i ,idresid) ) );
			for ( scoring::AtomNeighbors::const_iterator it2=nbrs2.begin(), it2e=nbrs2.end(); it2 != it2e; ++it2 ) {
				scoring::AtomNeighbor const & nbr( *it2 );
				Size const nbrresid = nbr.rsd();
				Real const cp_weight( nbr.weight() );  // do not use nbr->weight_func() here

				if (nbrresid == idresid) continue; // no intra

				conformation::Residue const & rsd2( pose.residue(nbrresid) );

				Size const heavyatom2( nbr.atomno() );
				if ( rsd2.atom_is_hydrogen( heavyatom2 ) ) continue;
				Real const d2( rsd1.xyz( i ).distance_squared( rsd2.xyz( heavyatom2 ) ) );

				utility::vector1< DerivVectorPair > r1_at_derivs(rsd1.natoms()), r2_at_derivs(rsd2.natoms());
				sum_deriv_contributions_for_heavyatom_pair_one_way(
					i, rsd1, rsd1_info, heavyatom2, rsd2, weights, cp_weight, d2, r1_at_derivs, r2_at_derivs );

				F1 += r1_at_derivs[atom1].f1();
				F2 += r1_at_derivs[atom1].f2();
			}
		}
	} else {
		utility_exit_with_message("non-nblist minimize!");
	}
}


core::Size
LK_BallEnergy::version() const
{
	return 1; // Initial versioning
}


}
}
}

