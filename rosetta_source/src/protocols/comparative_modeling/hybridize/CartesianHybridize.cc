// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file
/// @brief
/// @author Yifan Song
/// @author Frank DiMaio

#include <protocols/comparative_modeling/hybridize/CartesianHybridize.hh>

#include <protocols/comparative_modeling/hybridize/TemplateHistory.hh>
#include <core/pose/datacache/CacheableDataType.hh>
#include <basic/datacache/BasicDataCache.hh>

//#include <protocols/viewer/viewers.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <protocols/moves/Mover.hh>
#include <protocols/simple_moves/PackRotamersMover.hh>
#include <protocols/moves/MoverContainer.hh>
#include <protocols/simple_moves/SwitchResidueTypeSetMover.hh>
#include <protocols/jd2/JobDistributor.hh>
#include <protocols/relax/FastRelax.hh>
#include <protocols/relax/util.hh>

#include <protocols/simple_moves/FragmentMover.hh>

#include <protocols/idealize/IdealizeMover.hh>
#include <protocols/moves/MonteCarlo.hh>
#include <protocols/simple_moves/ConstraintSetMover.hh>
#include <core/scoring/dssp/Dssp.hh>
#include <protocols/comparative_modeling/coord_util.hh>

#include <protocols/loops/loops_main.hh>
#include <protocols/loops/Loops.hh>

#include <core/scoring/rms_util.hh>
#include <core/sequence/util.hh>
#include <core/sequence/Sequence.hh>
#include <core/id/SequenceMapping.hh>
#include <core/sequence/SequenceAlignment.hh>

#include <core/pose/Pose.hh>
#include <core/pose/PDBInfo.hh>
#include <core/pose/Remarks.hh>
#include <core/import_pose/import_pose.hh>
#include <core/chemical/ChemicalManager.hh>

#include <core/pose/util.hh>

#include <core/kinematics/FoldTree.hh>
#include <core/kinematics/MoveMap.hh>

#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/symmetry/SymmetricScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>

#include <core/optimization/AtomTreeMinimizer.hh>
#include <core/optimization/MinimizerOptions.hh>
#include <core/kinematics/MoveMap.hh>
#include <core/optimization/CartesianMinimizer.hh>
#include <core/optimization/MinimizerOptions.hh>

#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/scoring/methods/EnergyMethodOptions.hh>

#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/pack_rotamers.hh>
#include <core/pack/task/operation/TaskOperations.hh>
#include <core/conformation/util.hh>

#include <core/fragment/ConstantLengthFragSet.hh>
#include <core/fragment/FragmentIO.hh>
#include <core/fragment/FragSet.hh>
#include <core/fragment/Frame.hh>
#include <core/fragment/FrameIterator.hh>
#include <core/fragment/FragData.hh>

// symmetry
#include <core/pose/symmetry/util.hh>
#include <core/optimization/symmetry/SymAtomTreeMinimizer.hh>
#include <protocols/simple_moves/symmetry/SymPackRotamersMover.hh>
#include <core/conformation/symmetry/SymmetricConformation.hh>
#include <core/conformation/symmetry/SymmetryInfo.hh>


#include <utility/excn/Exceptions.hh>
#include <utility/file/file_sys_util.hh>

#include <numeric/random/random.hh>
#include <numeric/xyzVector.hh>
#include <numeric/xyz.functions.hh>
#include <numeric/model_quality/rms.hh>

#include <basic/options/option.hh>
#include <basic/options/option_macros.hh>
#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/options/keys/cm.OptionKeys.gen.hh>

#include <basic/Tracer.hh>
#include <boost/unordered/unordered_map.hpp>

namespace protocols {
namespace comparative_modeling {
namespace hybridize {

const core::Size DEFAULT_NCYCLES=400;

static basic::Tracer TR("protocols.comparative_modeling.hybridize.CartesianHybridize");

CartesianHybridize::CartesianHybridize( ) : ncycles_(DEFAULT_NCYCLES) {
	init();
}

CartesianHybridize::CartesianHybridize(
		utility::vector1 < core::pose::PoseOP > const & templates_in,
		utility::vector1 < core::Real > const & template_wts_in,
		utility::vector1 < protocols::loops::Loops > const & template_chunks_in, 
		utility::vector1 < protocols::loops::Loops > const & template_contigs_in,
		core::fragment::FragSetOP fragments9_in ) : ncycles_(DEFAULT_NCYCLES) {
	init();

	templates_ = templates_in;
	template_wts_ = template_wts_in;
	template_contigs_ = template_contigs_in;
	fragments9_ = fragments9_in;

	// make sure all data is there
	runtime_assert( templates_.size() == template_wts_.size() );
	runtime_assert( templates_.size() == template_contigs_.size() );

	// normalize weights
	core::Real weight_sum = 0.0;
	for (int i=1; i<=templates_.size(); ++i) weight_sum += template_wts_[i];
	for (int i=1; i<=templates_.size(); ++i) template_wts_[i] /= weight_sum;

	// map resids to frames
	for (core::fragment::FrameIterator i = fragments9_->begin(); i != fragments9_->end(); ++i) {
		core::Size position = (*i)->start();
		library_[position] = **i;
	}

	// use chunks to subdivide contigs
	core::Size ntempls = templates_.size();
	for( int tmpl = 1; tmpl <= ntempls; ++tmpl) {
		core::Size ncontigs = template_contigs_[tmpl].size();  // contigs to start
		for (int i=1; i<=ncontigs; ++i) {
			core::Size cstart = template_contigs_[tmpl][i].start(), cstop = template_contigs_[tmpl][i].stop();
			bool spilt_chunk=false;

			// assumes sorted
			for (int j=2; j<=template_chunks_in[tmpl].size(); ++j) {
				core::Size j0start = template_chunks_in[tmpl][j-1].start(), j0stop = template_chunks_in[tmpl][j-1].stop();
				core::Size j1start = template_chunks_in[tmpl][j].start(), j1stop = template_chunks_in[tmpl][j].stop();
	
				bool j0incontig = ((j0start>=cstart) && (j0stop<=cstop));
				bool j1incontig = ((j1start>=cstart) && (j1stop<=cstop));
	
				if (j0incontig && j1incontig) {
					spilt_chunk=true;
					core::Size cutpoint = (j0stop+j1start)/2;
					template_contigs_[tmpl].add_loop( cstart, cutpoint-1 );
					TR << "Make subfrag " << cstart << " , " << cutpoint-1 << std::endl;
					cstart=cutpoint;
				} else if(spilt_chunk && j0incontig && !j1incontig) {
					spilt_chunk=false;
					template_contigs_[tmpl].add_loop( cstart, cstop );
					TR << "Make subfrag " << cstart << " , " << cstop << std::endl;
				}
			}
		}
	}
	TR.Debug << "template_contigs:" << std::endl;
	for (int i=1; i<= template_contigs_.size(); ++i) {
		TR.Debug << "templ. " << i << std::endl << template_contigs_[i] << std::endl;
	}
}

void
CartesianHybridize::init() {
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	increase_cycles_ = option[cm::hybridize::stage2_increase_cycles]();
	no_global_frame_ = option[cm::hybridize::no_global_frame]();
	linmin_only_ = option[cm::hybridize::linmin_only]();

	// default scorefunction
	set_scorefunction ( core::scoring::ScoreFunctionFactory::create_score_function( "score4_smooth_cart" ) );
}

void
CartesianHybridize::set_scorefunction(core::scoring::ScoreFunctionOP scorefxn_in) {
	lowres_scorefxn_ = scorefxn_in->clone();

	min_scorefxn_ = scorefxn_in->clone();

	bonds_scorefxn_ = new core::scoring::symmetry::SymmetricScoreFunction();
	bonds_scorefxn_->set_weight( core::scoring::vdw, lowres_scorefxn_->get_weight( core::scoring::vdw ) );
	bonds_scorefxn_->set_weight( core::scoring::cart_bonded, lowres_scorefxn_->get_weight( core::scoring::cart_bonded ) );

	nocst_scorefxn_ = lowres_scorefxn_->clone();
	nocst_scorefxn_->set_weight( core::scoring::atom_pair_constraint, 0.0 );
}

void
CartesianHybridize::apply_frag( core::pose::Pose &pose, core::pose::Pose &templ, protocols::loops::Loop &frag, bool superpose) {
	// superimpose frag
	numeric::xyzMatrix< core::Real > R;
	numeric::xyzVector< core::Real > preT(0,0,0), postT(0,0,0);
	R.xx() = R.yy() = R.zz() = 1;
	R.xy() = R.yx() = R.zx() = R.zy() = R.yz() = R.xz() = 0;

	if (superpose) {
		core::Size len = frag.size();
		core::Size aln_len = std::min( (core::Size)9999, len );   //fpd  can change 9999 to some max alignment sublength
		core::Size aln_start = numeric::random::random_range(frag.start(), len-aln_len+frag.start() );

		// don't try to align really short frags
		if (len > 2) {
			ObjexxFCL::FArray2D< core::Real > final_coords( 3, 4*aln_len );
			ObjexxFCL::FArray2D< core::Real > init_coords( 3, 4*aln_len );
	
			for (int ii=0; ii<(int)aln_len; ++ii) {
				int i=aln_start+ii;
				numeric::xyzVector< core::Real > x_1 = templ.residue(i).atom(" C  ").xyz();
				numeric::xyzVector< core::Real > x_2 = templ.residue(i).atom(" O  ").xyz();
				numeric::xyzVector< core::Real > x_3 = templ.residue(i).atom(" CA ").xyz();
				numeric::xyzVector< core::Real > x_4 = templ.residue(i).atom(" N  ").xyz();
				preT += x_1+x_2+x_3+x_4;
	
				numeric::xyzVector< core::Real > y_1 = pose.residue(templ.pdb_info()->number(i)).atom(" C  ").xyz();
				numeric::xyzVector< core::Real > y_2 = pose.residue(templ.pdb_info()->number(i)).atom(" O  ").xyz();
				numeric::xyzVector< core::Real > y_3 = pose.residue(templ.pdb_info()->number(i)).atom(" CA ").xyz();
				numeric::xyzVector< core::Real > y_4 = pose.residue(templ.pdb_info()->number(i)).atom(" N  ").xyz();
				postT += y_1+y_2+y_3+y_4;
	
				for (int j=0; j<3; ++j) { 
					init_coords(j+1,4*ii+1) = x_1[j];
					init_coords(j+1,4*ii+2) = x_2[j];
					init_coords(j+1,4*ii+3) = x_3[j];
					init_coords(j+1,4*ii+4) = x_4[j];
					final_coords(j+1,4*ii+1) = y_1[j];
					final_coords(j+1,4*ii+2) = y_2[j];
					final_coords(j+1,4*ii+3) = y_3[j];
					final_coords(j+1,4*ii+4) = y_4[j];
				}
			}
			preT /= 4*len;
			postT /= 4*len;
			for (int i=1; i<=(int)4*len; ++i) {
				for ( int j=0; j<3; ++j ) {
					init_coords(j+1,i) -= preT[j];
					final_coords(j+1,i) -= postT[j];
				}
			}

			// get optimal superposition
			// rotate >init< to >final<
			ObjexxFCL::FArray1D< numeric::Real > ww( 4*len, 1.0 );
			ObjexxFCL::FArray2D< numeric::Real > uu( 3, 3, 0.0 );
			numeric::Real ctx;

			numeric::model_quality::findUU( init_coords, final_coords, ww, 4*len, uu, ctx );
			R.xx( uu(1,1) ); R.xy( uu(2,1) ); R.xz( uu(3,1) );
			R.yx( uu(1,2) ); R.yy( uu(2,2) ); R.yz( uu(3,2) );
			R.zx( uu(1,3) ); R.zy( uu(2,3) ); R.zz( uu(3,3) );
		}
	}

	// xyz copy fragment to pose
	for (int i=frag.start(); i<=frag.stop(); ++i) {
		for (int j=1; j<=templ.residue(i).natoms(); ++j) {
			core::id::AtomID src(j,i), tgt(j, templ.pdb_info()->number(i));
			pose.set_xyz( tgt, postT + (R*(templ.xyz( src )-preT)) );
		}
	}
}


void
CartesianHybridize::apply_frame( core::pose::Pose & pose, core::fragment::Frame &frame ) {
	core::Size start = frame.start(),len = frame.length();

	// we might want to tune this
	// it might make sense to change this based on gap width
	// for really large gaps make it one sided to emulate fold-tree fragment insertion
	int aln_len = 3;
	
	core::Size nres = pose.total_residue();

	//symmetry
	core::conformation::symmetry::SymmetryInfoCOP symm_info;
	if ( core::pose::symmetry::is_symmetric(pose) ) {
		core::conformation::symmetry::SymmetricConformation & SymmConf (
			dynamic_cast<core::conformation::symmetry::SymmetricConformation &> ( pose.conformation()) );
		symm_info = SymmConf.Symmetry_Info();
		nres = symm_info->num_independent_residues();
	}

	if (pose.residue(nres).aa() == core::chemical::aa_vrt) nres--;
	bool nterm = (start == 1);
	bool cterm = (start == nres-8);

	// insert frag
	core::pose::Pose pose_copy = pose;

	ObjexxFCL::FArray1D< numeric::Real > ww( 2*4*aln_len, 1.0 );
	ObjexxFCL::FArray2D< numeric::Real > uu( 3, 3, 0.0 );
	numeric::xyzVector< core::Real > com1(0,0,0), com2(0,0,0);

	for (int i=0; i<(int)len; ++i) {
		core::conformation::idealize_position(start+i, pose_copy.conformation());
	}
	for (int tries = 0; tries<80; ++tries) {
		ww = 1.0;
		uu = 0.0;
		com1 = numeric::xyzVector< core::Real >(0,0,0);
		com2 = numeric::xyzVector< core::Real >(0,0,0);

		// grab coords
		ObjexxFCL::FArray2D< core::Real > init_coords( 3, 2*4*aln_len );
		for (int ii=-aln_len; ii<aln_len; ++ii) {
			int i = (ii>=0) ? (nterm?len-ii-1:ii) : (cterm?-ii-1:len+ii);
			numeric::xyzVector< core::Real > x_1 = pose.residue(start+i).atom(" C  ").xyz();
			numeric::xyzVector< core::Real > x_2 = pose.residue(start+i).atom(" O  ").xyz();
			numeric::xyzVector< core::Real > x_3 = pose.residue(start+i).atom(" CA ").xyz();
			numeric::xyzVector< core::Real > x_4 = pose.residue(start+i).atom(" N  ").xyz();
			com1 += x_1+x_2+x_3+x_4;
			for (int j=0; j<3; ++j) { 
				init_coords(j+1,4*(ii+aln_len)+1) = x_1[j];
				init_coords(j+1,4*(ii+aln_len)+2) = x_2[j];
				init_coords(j+1,4*(ii+aln_len)+3) = x_3[j];
				init_coords(j+1,4*(ii+aln_len)+4) = x_4[j];
			}
		}
		com1 /= 2.0*4.0*aln_len;
		for (int ii=0; ii<2*4*aln_len; ++ii) {
			for ( int j=0; j<3; ++j ) init_coords(j+1,ii+1) -= com1[j];
		}

		core::Size toget = numeric::random::random_range( 1, frame.nr_frags() );
		frame.apply( toget, pose_copy );

		// grab new coords
		ObjexxFCL::FArray2D< core::Real > final_coords( 3, 2*4*aln_len );
		for (int ii=-aln_len; ii<aln_len; ++ii) {
			int i = (ii>=0) ? (nterm?len-ii-1:ii) : (cterm?-ii-1:len+ii);
			numeric::xyzVector< core::Real > x_1 = pose_copy.residue(start+i).atom(" C  ").xyz();
			numeric::xyzVector< core::Real > x_2 = pose_copy.residue(start+i).atom(" O  ").xyz();
			numeric::xyzVector< core::Real > x_3 = pose_copy.residue(start+i).atom(" CA ").xyz();
			numeric::xyzVector< core::Real > x_4 = pose_copy.residue(start+i).atom(" N  ").xyz();
			com2 += x_1+x_2+x_3+x_4;
			for (int j=0; j<3; ++j) {
			final_coords(j+1,4*(ii+aln_len)+1) = x_1[j];
				final_coords(j+1,4*(ii+aln_len)+2) = x_2[j];
				final_coords(j+1,4*(ii+aln_len)+3) = x_3[j];
				final_coords(j+1,4*(ii+aln_len)+4) = x_4[j];
			}
		}
		com2 /= 2.0*4.0*aln_len;
		for (int ii=0; ii<2*4*aln_len; ++ii) {
			for ( int j=0; j<3; ++j ) final_coords(j+1,ii+1) -= com2[j];
		}

		numeric::Real ctx;
		float rms;

		numeric::model_quality::findUU( final_coords, init_coords, ww, 2*4*aln_len, uu, ctx );
		numeric::model_quality::calc_rms_fast( rms, final_coords, init_coords, ww, 2*4*aln_len, ctx );

		//fpd  another place where we might want to tune parameters
		if (rms < 0.5) break;
		if (tries >= 20 && rms < 1) break;
		if (tries >= 40 && rms < 2) break;
		if (tries >= 60 && rms < 4) break;
	}
	numeric::xyzMatrix< core::Real > R;
	R.xx( uu(1,1) ); R.xy( uu(2,1) ); R.xz( uu(3,1) );
	R.yx( uu(1,2) ); R.yy( uu(2,2) ); R.yz( uu(3,2) );
	R.zx( uu(1,3) ); R.zy( uu(2,3) ); R.zz( uu(3,3) );

	// apply rotation to ALL atoms
	// x_i' <- = R*x_i + com1;
	for ( Size i = 0; i < len; ++i ) {
		for ( Size j = 1; j <= pose.residue_type(start+i).natoms(); ++j ) {
			core::id::AtomID id( j, start+i );
			pose.set_xyz( id, R * ( pose_copy.xyz(id) - com2) + com1 );
		}
	}
}


void
CartesianHybridize::apply( Pose & pose ) {
	using namespace basic::options;
	using namespace basic::options::OptionKeys;
	using namespace core::pose::datacache;

	//protocols::viewer::add_conformation_viewer(  pose.conformation(), "hybridize" );

	protocols::moves::MoverOP tocen =
		new protocols::simple_moves::SwitchResidueTypeSetMover( core::chemical::CENTROID );
	tocen->apply( pose );

	// minimizer
	core::optimization::MinimizerOptions options( "linmin", 0.01, true, false, false );
	core::optimization::MinimizerOptions options_minilbfgs( "lbfgs_armijo_nonmonotone", 0.01, true, false, false );
	options_minilbfgs.max_iter(5);
	core::optimization::MinimizerOptions options_lbfgs( "lbfgs_armijo_nonmonotone", 0.01, true, false, false );
	options_lbfgs.max_iter(500);
	core::optimization::CartesianMinimizer minimizer;
	core::kinematics::MoveMap mm;
	mm.set_bb  ( true ); mm.set_chi ( true ); mm.set_jump( true );

	core::Real max_cart = lowres_scorefxn_->get_weight( core::scoring::cart_bonded );
	core::Real max_cst  = lowres_scorefxn_->get_weight( core::scoring::atom_pair_constraint );
	core::Real max_vdw  = lowres_scorefxn_->get_weight( core::scoring::vdw );

	// for i = 1 to n cycles
	core::Size NMACROCYCLES = 4;
	TR << "RUNNING FOR " << NMACROCYCLES << " MACROCYCLES" << std::endl;

	Pose pose_in = pose;
	core::Size nres = pose.total_residue();
	if (pose.residue(nres).aa() == core::chemical::aa_vrt) nres--;

	// 10% of the time, skip moves in the global frame 
	bool no_ns_moves = no_global_frame_; // (numeric::random::uniform() <= 0.1);

sampler:
	for (int m=1; m<=NMACROCYCLES; m+=1) {
		core::Real bonded_weight = max_cart;
		if (m==1) bonded_weight = 0.0*max_cart;
		if (m==2) bonded_weight = 0.01*max_cart;
		if (m==3) bonded_weight = 0.1*max_cart;

		core::Real cst_weight = max_cst;
		if (m==1)  cst_weight = 2*max_cst;
		if (m==2)  cst_weight = 2*max_cst;
		if (m==3)  cst_weight = 2*max_cst;

		core::Real vdw_weight = max_vdw;
		if (m==1) vdw_weight = 0.1*max_vdw;
		if (m==2) vdw_weight = 0.1*max_vdw;
		if (m==3) vdw_weight = 0.1*max_vdw;

		TR << "CYCLE " << m << std::endl;
		TR << "  setting bonded weight = " << bonded_weight << std::endl;
		TR << "  setting cst    weight = " << cst_weight << std::endl;
		TR << "  setting vdw    weight = " << vdw_weight << std::endl;

		lowres_scorefxn_->set_weight( core::scoring::cart_bonded, bonded_weight );
		lowres_scorefxn_->set_weight( core::scoring::atom_pair_constraint, cst_weight );
		lowres_scorefxn_->set_weight( core::scoring::vdw, vdw_weight );

		(*min_scorefxn_)(pose);
		(*lowres_scorefxn_)(pose);
		protocols::moves::MonteCarloOP mc = new protocols::moves::MonteCarlo( pose, *lowres_scorefxn_, 2.0 );

		core::Size neffcycles = (core::Size)(ncycles_*increase_cycles_);
		if (m==4) neffcycles /=2;
		for (int n=1; n<=neffcycles; ++n) {
			// possible actions:
			//  1 - insert homologue frag, global frame
			//  2 - insert homologue frag, local frame
			//  3 - insert sequence frag
			core::Real action_picker = numeric::random::uniform();
			core::Size action = 0;
			if (m==1) {
				action = no_ns_moves?2:1;
				if (action_picker < 0.2) action = 3;
			} else if (m==2) {
				action = 2;
				if (action_picker < 0.2) action = 3;
			} else if (m==3) {
				action = 2;
				if (action_picker < 0.2) action = 3;
			} else if (m==4) {
				action = 3;
			}

			std::string action_string;
			if (action == 1) action_string = "fragNS";
			if (action == 2) action_string = "frag";
			if (action == 3) action_string = "picker";

			if (action == 1 || action == 2) {
				// uniform sampling of templates ... change this for weighted sampling
				// core::Size templ_id = numeric::random::random_range( 1, templates_.size() );
				// core::Size nfrags = template_chunks_[templ_id].num_loop() + template_contigs_[templ_id].num_loop();
				// core::Size frag_id = numeric::random::random_range( 1, nfrags );
				// protocols::loops::LoopOP frag =  new protocols::loops::Loop (
				// 	(frag_id <= template_chunks_[templ_id].num_loop()) ? 
				// 			template_chunks_[templ_id][frag_id]
				// 		: template_contigs_[templ_id][frag_id - template_chunks_[templ_id].num_loop()]
				// 	);

				core::Size templ_id = numeric::random::random_range( 1, templates_.size() );
				core::Size nfrags = template_contigs_[templ_id].num_loop();
				core::Size frag_id = numeric::random::random_range( 1, nfrags );
				protocols::loops::LoopOP frag =  new protocols::loops::Loop ( template_contigs_[templ_id][frag_id] );

				if (frag->size() > 14)
					action_string = action_string+"_15+";
				else if (frag->size() <= 4)
					action_string = action_string+"_0-4";
				else
					action_string = action_string+"_5-14";

				apply_frag( pose, *templates_[templ_id], *frag, (action==2) );

				if (action == 1) {
					//fpd assume this was initialized elsewhere
					runtime_assert( pose.data().has( CacheableDataType::TEMPLATE_HYBRIDIZATION_HISTORY ) );
					TemplateHistory &history = 
						*( static_cast< TemplateHistory* >( pose.data().get_ptr( CacheableDataType::TEMPLATE_HYBRIDIZATION_HISTORY )() ));
					history.set( frag->start(), frag->stop(), templ_id );
				}
			}

			if (action == 3) {
				// pick an insert position based on gap
				utility::vector1<core::Real> residuals( nres , 0.0 );
				utility::vector1<core::Real> max_residuals(3,0);
				utility::vector1<int> max_poses(4,-1);
				for (int i=1; i<nres; ++i) {
					if (pose.fold_tree().is_cutpoint(i+1)) {
						residuals[i] = -1;
					} else {
						numeric::xyzVector< core::Real > c0 , n1;
						c0 = pose.residue(i).atom(" C  ").xyz();
						n1 = pose.residue(i+1).atom(" N  ").xyz();
						core::Real d2 = c0.distance( n1 );
						residuals[i] = (d2-1.328685)*(d2-1.328685);
						if ( residuals[i] > max_residuals[1]) {
							max_residuals[3] = max_residuals[2]; max_residuals[2] = max_residuals[1]; max_residuals[1] = residuals[i];
							max_poses[3] = max_poses[2]; max_poses[2] = max_poses[1]; max_poses[1] = i;
						} else if ( residuals[i] > max_residuals[2]) {
							max_residuals[3] = max_residuals[2]; max_residuals[2] = residuals[i];
							max_poses[3] = max_poses[2]; max_poses[2] = i;
						} else if ( residuals[i] > max_residuals[3]) {
							max_residuals[3] = residuals[i];
							max_poses[3] = i;
						}
					}
				}

				// 25% chance of random position
				max_poses[ 4 ] = numeric::random::random_range(1,nres);
				int select_position = numeric::random::random_range(1,4);
				if (select_position == 4)
					action_string = action_string+"_rand";
				core::Size max_pos = max_poses[ select_position ];

				// select random pos in [i-8,i]
				core::Size insert_pos = max_pos - numeric::random::random_range(3,5);
				insert_pos = std::min( insert_pos, nres-8);
				insert_pos = std::max( (int)insert_pos, 1);

				if (library_.find(insert_pos) != library_.end())
					apply_frame (pose, library_[insert_pos]);
			}

			// MC
			try {
				(*min_scorefxn_)(pose);
				if ( m<4 || linmin_only_ )
					minimizer.run( pose, mm, *min_scorefxn_, options );
				else
					minimizer.run( pose, mm, *min_scorefxn_, options_minilbfgs );

				mc->boltzmann( pose , action_string );
			} catch( utility::excn::EXCN_Base& excn ) {
				//fpd hbond fail? start over
				pose = pose_in;
				goto sampler;
			}

			if (n%100 == 0) {
				TR << "Step " << n << std::endl;
				mc->show_scores();
				mc->show_counters();
			}

		}
		mc->recover_low(pose);
	}

	//for (int i=1; i<=templates_.size(); ++i) {
	//	std::ostringstream oss;
	//	oss << "templ"<<i<<".pdb";
	//	templates_[i]->dump_pdb( oss.str() );
	//}

	// final minimization	
	try {
		(*min_scorefxn_)(pose); minimizer.run( pose, mm, *min_scorefxn_, options_lbfgs );
		(*nocst_scorefxn_)(pose); minimizer.run( pose, mm, *nocst_scorefxn_, options_lbfgs );
		(*bonds_scorefxn_)(pose); minimizer.run( pose, mm, *bonds_scorefxn_, options_lbfgs );
		(*min_scorefxn_)(pose); minimizer.run( pose, mm, *min_scorefxn_, options_lbfgs );
	} catch( utility::excn::EXCN_Base& excn ) {
		//fpd hbond fail? start over
		pose = pose_in;
		goto sampler;
	}

	lowres_scorefxn_->set_weight( core::scoring::cart_bonded, max_cart );
	lowres_scorefxn_->set_weight( core::scoring::atom_pair_constraint, max_cst );
	lowres_scorefxn_->set_weight( core::scoring::vdw, max_vdw );
	(*lowres_scorefxn_)(pose);
}

}
}
}
