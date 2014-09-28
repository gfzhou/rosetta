// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/antibody_design/AntibodyDesignModeler.cc
/// @brief Handles modeling of the antibody.  Before and after design
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

//Antibody Headers
#include <protocols/antibody/design/AntibodyDesignModeler.hh>
#include <protocols/antibody/design/util.hh>
#include <protocols/antibody/AntibodyEnum.hh>
#include <protocols/antibody/clusters/CDRClusterEnum.hh>
#include <protocols/antibody/AntibodyInfo.hh>
#include <protocols/antibody/util.hh>

// Core Headers
#include <core/pose/PDBInfo.hh>
#include <core/pose/Pose.hh>
#include <core/pose/util.hh>
#include <core/scoring/ScoreType.hh>
#include <core/kinematics/MoveMap.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/scoring/constraints/util.hh>
#include <core/scoring/constraints/ConstraintSet.hh>
#include <core/optimization/MinimizerOptions.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/PackerTask.hh>
#include <core/pack/task/operation/TaskOperations.hh>
#include <core/kinematics/MoveMap.hh>

//Protocol Headers
#include <protocols/simple_moves/SwitchResidueTypeSetMover.hh>
#include <protocols/simple_moves/PackRotamersMover.hh>
#include <protocols/relax/FastRelax.hh>
#include <protocols/relax/CentroidRelax.hh>
#include <protocols/relax/RelaxProtocolBase.hh>
#include <protocols/simple_moves/MinMover.hh>
#include <protocols/docking/DockingLowRes.hh>
#include <protocols/docking/DockMCMProtocol.hh>
#include <protocols/docking/util.hh>
#include <protocols/toolbox/task_operations/RestrictToInterface.hh>
#include <protocols/toolbox/task_operations/PreventChainFromRepackingOperation.hh>
#include <protocols/loops/loops_main.hh>
#include <protocols/simple_moves/BackboneMover.hh>
#include <protocols/loops/util.hh>
#include <protocols/loops/loops_main.hh>

// Basic Headers
#include <basic/options/option.hh>
#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/antibody.OptionKeys.gen.hh>
//#include <basic/options/keys/constraints.OptionKeys.gen.hh>
#include <basic/Tracer.hh>
#include <utility/file/FileName.hh>
#include <sstream>

static thread_local basic::Tracer TR( "antibody.design.AntibodyDesignModeler" );
namespace protocols {
namespace antibody {
namespace design {


using namespace core::kinematics;
using namespace core::pose;
using namespace core::scoring;
using namespace core::pack::task;
using namespace core::pack::task::operation;
using namespace protocols::simple_moves;
using namespace protocols::toolbox::task_operations;
using namespace protocols::antibody::clusters;
using core::Size;
using std::string;


AntibodyDesignModeler::AntibodyDesignModeler(AntibodyInfoOP ab_info) :
	utility::pointer::ReferenceCount(),
	ab_info_(ab_info)
{


	set_defaults();
	read_command_line_options();
	setup_scorefxns();
	setup_task_operations();

}

void
AntibodyDesignModeler::set_defaults(){

	model_cdrs_.clear();
	model_cdrs_.resize(CDRNameEnum_total, true);

	overhangs_.clear();
	overhangs_.resize(CDRNameEnum_total, 0);
}

AntibodyDesignModeler::~AntibodyDesignModeler(){}

void
AntibodyDesignModeler::read_command_line_options(){
	interface_detection_dis(basic::options::option [basic::options::OptionKeys::antibody::design::interface_dis]());
	neighbor_detection_dis(basic::options::option [basic::options::OptionKeys::antibody::design::neighbor_dis]());
	//set_constrain_dock_to_ab_chain(basic::options::option [basic::options::OptionKeys::antibody::design::ab_dock_chains]());
}

void
AntibodyDesignModeler::setup_scorefxns(){

	scorefxn_ = get_score_function();
	scorefxn_->apply_patch_from_file("antibody_design");
	docking_scorefxn_high_ = ScoreFunctionFactory::create_score_function( "docking", "docking_min" );
	docking_scorefxn_high_->apply_patch_from_file("antibody_design");

	docking_scorefxn_low_ = ScoreFunctionFactory::create_score_function("interchain_cen");
	docking_scorefxn_low_->apply_patch_from_file("antibody_design");
}

void
AntibodyDesignModeler::setup_task_operations(){

	cmd_line_operation_ = operation::InitializeFromCommandlineOP( new core::pack::task::operation::InitializeFromCommandline() );
	restrict_design_operation_ = operation::RestrictToRepackingOP( new core::pack::task::operation::RestrictToRepacking() );
	loops_operation_ = protocols::toolbox::task_operations::RestrictToLoopsAndNeighborsOP( new RestrictToLoopsAndNeighbors() );

	tf_ = TaskFactoryOP( new core::pack::task::TaskFactory() );
	interface_tf_ = TaskFactoryOP( new core::pack::task::TaskFactory() );
	interface_tf_->push_back(cmd_line_operation_);
	interface_tf_->push_back( restrict_design_operation_);
	interface_tf_->push_back(TaskOperationCOP( new RestrictToInterface(1, interface_dis_/*Distance*/) ));
}

void
AntibodyDesignModeler::set_cdr_range(CDRNameEnum const cdr_start, CDRNameEnum const cdr_end, bool setting) {
	for (core::SSize i=cdr_start; i<=cdr_end; ++i){
		CDRNameEnum cdr = static_cast<CDRNameEnum>(i);
		set_cdr(cdr, setting);
	}
}

void
AntibodyDesignModeler::set_cdrs(utility::vector1<bool> cdrs){
	model_cdrs_ = cdrs;
}

void
AntibodyDesignModeler::set_cdr_only(CDRNameEnum cdr, bool setting){
	if (setting==true){
		set_cdr_range(CDRNameEnum_start, CDRNameEnum_total, false);
		set_cdr(cdr, true);
	}
	else{
		set_cdr_range(CDRNameEnum_start, CDRNameEnum_total, true);
		set_cdr(cdr, false);
	}
}

void
AntibodyDesignModeler::set_cdr(CDRNameEnum const cdr, bool setting){
	model_cdrs_[cdr]=setting;
}

void
AntibodyDesignModeler::cdr_overhang(const CDRNameEnum cdr, const core::Size overhang) {
	overhangs_[cdr] = overhang;
}

void
AntibodyDesignModeler::ab_dock_chains(std::string ab_dock_chains) {
	ab_dock_chains_ = ab_dock_chains;

}

void
AntibodyDesignModeler::set_scorefunction(ScoreFunctionCOP scorefxn){
	scorefxn_ = scorefxn->clone();
}

void
AntibodyDesignModeler::set_dock_low_res_scorefunction(ScoreFunctionCOP scorefxn){
	docking_scorefxn_low_ = scorefxn->clone();
}

void
AntibodyDesignModeler::set_dock_high_res_scorefunction(ScoreFunctionCOP scorefxn){
	docking_scorefxn_high_ = scorefxn->clone();
}

void
AntibodyDesignModeler::interface_detection_dis(core::Real interface_distance){
	interface_dis_ = interface_distance;
}

void
AntibodyDesignModeler::neighbor_detection_dis(core::Real neighbor_distance){
	neighbor_dis_ = neighbor_distance;
}

protocols::loops::LoopsOP
AntibodyDesignModeler::get_cdr_loops(Pose const & pose) const {

	protocols::loops::LoopsOP cdr_loops( new protocols::loops::Loops );

	for (Size i = 1; i <= CDRNameEnum_total; ++i){
		if (model_cdrs_[i]){
			CDRNameEnum cdr = static_cast<CDRNameEnum>(i);
			Size start = ab_info_->get_CDR_start(cdr, pose);
			Size stop =  ab_info_->get_CDR_end(cdr, pose);
			Size cutpoint = (stop-start)/2+start;
			protocols::loops::Loop cdr_loop = protocols::loops::Loop(start, stop, cutpoint);
			cdr_loops->add_loop(cdr_loop);
		}
	}
	return cdr_loops;
}

protocols::loops::LoopsOP
AntibodyDesignModeler::get_cdr_loops_with_overhang(Pose const & pose) const {

	protocols::loops::LoopsOP cdr_loops( new protocols::loops::Loops );

	for (Size i = 1; i <= CDRNameEnum_total; ++i){
		if (model_cdrs_[i]){
			CDRNameEnum cdr = static_cast<CDRNameEnum>(i);
			protocols::loops::Loop cdr_loop = get_cdr_loop_with_overhang(pose, cdr);
			cdr_loops->add_loop(cdr_loop);
		}
	}
	return cdr_loops;
}

protocols::loops::Loop
AntibodyDesignModeler::get_cdr_loop_with_overhang(Pose const & pose, CDRNameEnum cdr) const {

	Size start = ab_info_->get_CDR_start(cdr, pose) - overhangs_[cdr];
	Size stop =  ab_info_->get_CDR_end(cdr, pose) + overhangs_[cdr];
	Size cutpoint = (stop-start)/2+start;
	protocols::loops::Loop cdr_loop = protocols::loops::Loop(start, stop, cutpoint);
	return cdr_loop;

}

void
AntibodyDesignModeler::apply_LH_A_foldtree(core::pose::Pose & pose) const {
	vector1< char > antigen_chains = ab_info_->get_antigen_chains();

	std::string antigen(antigen_chains.begin(), antigen_chains.end());
	std::string dock_chains = "LH_"+antigen;

	vector1< int > movable_jumps(1, 1);
	protocols::docking::setup_foldtree(pose, dock_chains, movable_jumps);
}

MoveMapOP
AntibodyDesignModeler::get_cdrs_movemap_with_overhang(Pose const & pose, bool min_bb, bool min_sc, bool include_neighbor_sc, bool include_neighbor_bb) const {

	MoveMapOP mm( new MoveMap() );
	for (core::Size i = 1; i <= core::Size(CDRNameEnum_total); ++i){
		if (model_cdrs_[i]){
			CDRNameEnum cdr = static_cast<CDRNameEnum>(i);
			protocols::loops::Loop cdr_loop = get_cdr_loop_with_overhang(pose, cdr);

			vector1<bool> all_included_res(pose.total_residue(), false);
			select_loop_residues( pose, cdr_loop, (include_neighbor_sc || include_neighbor_bb) ,all_included_res , neighbor_dis_);
			for (core::Size x = 1; x <= pose.total_residue(); ++x){
				if (x >= cdr_loop.start() && x <= cdr_loop.stop()){
					if (min_bb) mm->set_bb(x, true);
					if (min_sc) mm->set_chi(x, true);

				}
				//Not in the loop, then it's a neighbor
				else if  (all_included_res[x]) {
					if (include_neighbor_sc) mm->set_chi(x, true);
					if (include_neighbor_bb) mm->set_bb(x, true);
				}
			}
		}
	}

	return mm;
}

MoveMapOP
AntibodyDesignModeler::get_movemap_from_task(core::pose::Pose const & pose, core::pack::task::PackerTaskCOP task) const {
	MoveMapOP mm( new MoveMap() );
	for (core::Size i = 1; i<=pose.total_residue(); i++){
		if (task->pack_residue(i)){
			mm->set_chi(i, true);
		}
	}
	return mm;
}


////////////////////////////////////////////////////////// Modeling Functions /////////////////////////////////////////////

void
AntibodyDesignModeler::extend_CDR(Pose& pose, CDRNameEnum cdr) const {

	core::Size start = ab_info_->get_CDR_start(cdr, pose);
	core::Size end = ab_info_->get_CDR_end(cdr, pose);

	protocols::loops::Loop cdr_loop  = protocols::loops::Loop(start+2, end-2, start+3);
	protocols::loops::Loops cdr_loops = protocols::loops::Loops();
	cdr_loops.add_loop(cdr_loop);

	core::kinematics::FoldTree original_ft = pose.fold_tree();
	core::kinematics::FoldTree loop_ft = core::kinematics::FoldTree();

	protocols::loops::fold_tree_from_loops(pose, cdr_loops, loop_ft);
	pose.fold_tree(loop_ft);
	protocols::loops::set_extended_torsions_and_idealize_loops(pose, cdr_loops);

	pose.fold_tree(original_ft);

}

void
AntibodyDesignModeler::repack_cdrs(Pose& pose, bool include_neighbor_sc ) {
	using namespace protocols::toolbox::task_operations;

	protocols::loops::LoopsOP cdr_loops = get_cdr_loops_with_overhang(pose);

	TR << "repacking cdrs start: "<<(*scorefxn_)(pose) << std::endl;
	loops_operation_->set_design_loop(false);
	loops_operation_->set_include_neighbors(include_neighbor_sc);
	loops_operation_->set_cutoff_distance(neighbor_dis_);
	loops_operation_->set_loops(cdr_loops);

	tf_->clear();
	tf_->push_back(cmd_line_operation_ );
	tf_->push_back( restrict_design_operation_ );
	tf_->push_back(loops_operation_);

	core::pack::task::PackerTaskOP task = tf_->create_task_and_apply_taskoperations(pose);

	PackRotamersMover packer = PackRotamersMover(scorefxn_, task);
	packer.apply(pose);
	TR << "repacking cdrs end: "<<(*scorefxn_)(pose) << std::endl;

}

void
AntibodyDesignModeler::minimize_cdrs(Pose & pose,
		bool min_sc,
		bool include_neighbor_sc ,
		bool min_interface,
		bool cartmin,
		bool use_talaris_cartmin) const {

	ScoreFunctionOP local_scorefxn;
	if (cartmin && use_talaris_cartmin ){
		local_scorefxn = core::scoring::ScoreFunctionFactory::create_score_function("talaris2013_cart");
	}else if (cartmin){
		local_scorefxn = scorefxn_->clone();
		local_scorefxn->set_weight(cart_bonded, .5);
		local_scorefxn->set_weight(pro_close, 0);
	} else {
		local_scorefxn =  scorefxn_->clone();
	}

	local_scorefxn->set_weight(chainbreak, 100.00);
	TR << "CDR min start: "<<(*local_scorefxn)(pose) << std::endl;

	core::kinematics::FoldTree original_ft = pose.fold_tree();
	if (min_interface){
		TR << "Including RB interface min" <<std::endl;
		vector1< int > movable_jumps(1, 1);
		protocols::docking::setup_foldtree(pose, get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_), movable_jumps);
	}
	else if (! cartmin) {
		//Make sure Minimization of loops does not effect the rest of the chain
		protocols::loops::LoopsOP loops = get_cdr_loops_with_overhang(pose);
		core::kinematics::FoldTree ft = core::kinematics::FoldTree();
		protocols::loops::fold_tree_from_loops(pose, *loops, ft);
		pose.fold_tree(ft);
		protocols::loops::add_cutpoint_variants(pose);
	}

	MoveMapOP mm = get_cdrs_movemap_with_overhang(pose, true /*BB*/, min_sc/*SC*/, include_neighbor_sc, false /*neighbor_bb*/);

	//Tolerance of .001 is slower by ~25% or less.  However, it can also decrease energies significantly compared to .01.
	//(In a test set of one protein minimized by bb - write a script to test this )
	protocols::simple_moves::MinMover min_mover(mm, local_scorefxn, "dfpmin_armijo_nonmonotone", 0.001, false /*use_nblist*/ );

	if (cartmin){
		min_mover.min_type("lbfgs_armijo_nonmonotone");
		min_mover.cartesian(true);
		min_mover.min_options()->max_iter(200); //Great suggestion from Patrick Conway
		min_mover.apply(pose);
	}
	else{
		min_mover.apply(pose);
		protocols::loops::remove_cutpoint_variants(pose);
		pose.fold_tree(original_ft);
	}

	//local_scorefxn->show(TR, pose);
	TR << "CDR min end: "<<(*local_scorefxn)(pose) << std::endl;
}

void
AntibodyDesignModeler::relax_cdrs(Pose & pose,  bool include_neighbor_sc /*true*/,  bool starting_coordinate_constraints /*false*/, bool min_interface /* false */, bool dualspace /*false*/) const {

	ScoreFunctionOP local_scorefxn = scorefxn_->clone();


	local_scorefxn->set_weight(chainbreak, 100.00);

	core::kinematics::FoldTree original_ft = pose.fold_tree();
	protocols::loops::LoopsOP cdr_loops = get_cdr_loops_with_overhang(pose);

	if (min_interface){
		TR << "Including RB interface min" <<std::endl;
		vector1< int > movable_jumps(1, 1);
		protocols::docking::setup_foldtree(pose, protocols::antibody::design::get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_), movable_jumps);
	}
	//else if (! dualspace){
	else {
		//Make sure Minimization of loops does not effect the rest of the chain
		//Dualspace as well - since half of it is dihedral, we still need cutpoints.
		//No reason chainbreak term won't work with cart.

		core::kinematics::FoldTree ft = core::kinematics::FoldTree();
		protocols::loops::fold_tree_from_loops(pose, *cdr_loops, ft);
		pose.fold_tree(ft);
		protocols::loops::add_cutpoint_variants(pose);
	}

	loops_operation_->set_design_loop(false);
	loops_operation_->set_include_neighbors(include_neighbor_sc);
	loops_operation_->set_loops(cdr_loops);
	loops_operation_->set_cutoff_distance(neighbor_dis_);

	tf_->clear();
	tf_->push_back(cmd_line_operation_ );
	tf_->push_back( restrict_design_operation_ );
	tf_->push_back(loops_operation_);

	MoveMapOP mm = get_cdrs_movemap_with_overhang(pose, true /*BB*/, true/*SC*/, include_neighbor_sc,  false /*neighbor_bb*/);


	protocols::relax::FastRelaxOP rel( new protocols::relax::FastRelax() );//Stack construction failed to compile.

	if (dualspace) {
		local_scorefxn->set_weight(cart_bonded, .5);
		local_scorefxn->set_weight(pro_close, 0);
		rel->dualspace(true);
		rel->max_iter(200);
		//rel->minimize_bond_angles(true); Too Long - should be option
	}
	TR << "CDR relax start: "<<(*local_scorefxn)(pose) << std::endl;
	rel->set_scorefxn(local_scorefxn);
	rel->set_movemap(mm);
	rel->set_task_factory(tf_);
	if (starting_coordinate_constraints){
		rel->constrain_relax_to_start_coords(true);
	}
	rel->apply(pose);

	local_scorefxn->show_pretty(TR.Debug);

	TR << "CDR relax end: "<<(*local_scorefxn)(pose) << std::endl;
	if (starting_coordinate_constraints){
		core::scoring::constraints::remove_constraints_of_type(pose, "CoordinateConstraint");
	}
	protocols::loops::remove_cutpoint_variants(pose);
	pose.fold_tree(original_ft);
}

void
AntibodyDesignModeler::relax_interface(Pose & pose, bool min_interface_sc /* true */) const {

	//TR << "Relaxing Interface: " << dock_chains << std::endl;
	ScoreFunctionOP local_scorefxn = scorefxn_->clone();
	vector1< int > movable_jumps(1, 1);
	core::kinematics::FoldTree original_ft = pose.fold_tree();
	protocols::docking::setup_foldtree(pose, get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_), movable_jumps);

	MoveMapOP mm;
	mm->set_jump(1, true); //There is more than one jump!
	core::pack::task::TaskFactoryOP tf = interface_tf_->clone();
	core::pack::task::PackerTaskOP task = interface_tf_->create_task_and_apply_taskoperations(pose);
	protocols::relax::FastRelaxOP rel( new protocols::relax::FastRelax(local_scorefxn) );

	//Add residues to movemap, so that they minimize during minimization, while repacking will happen via tf. Maybe should not have any chi packing.
	if (min_interface_sc){
		mm = get_movemap_from_task(pose, task);
	}

	rel->set_movemap(mm);
	rel->set_task_factory(tf);
	rel->apply(pose);
	pose.fold_tree(original_ft);
}

void
AntibodyDesignModeler::minimize_interface(Pose& pose, bool min_interface_sc /* true */) const {

	//TR << "Minimizing Interface: " << dock_chains << std::endl;
	vector1< int > movable_jumps(1, 1);
	core::kinematics::FoldTree original_ft = pose.fold_tree();
	protocols::docking::setup_foldtree(pose, get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_), movable_jumps);


	core::pack::task::PackerTaskOP task = interface_tf_->create_task_and_apply_taskoperations(pose);
	MoveMapOP mm;
	if (min_interface_sc){
		mm = get_movemap_from_task(pose, task);
	}

	mm->set_jump(1, true); //There is more than one jump!

	ScoreFunctionOP local_scorefxn = scorefxn_->clone();
	protocols::simple_moves::MinMover min_mover(mm, local_scorefxn, "dfpmin_armijo_nonmonotone", 0.01, false /*use_nblist*/ );
	min_mover.apply(pose);


	pose.fold_tree(original_ft);
}


void
AntibodyDesignModeler::repack_antigen_ab_interface(Pose& pose) const {
	vector1< char > antigen_chains = ab_info_->get_antigen_chains();
	if (antigen_chains.size() == 0){
		TR <<" Antigen not present to repack interface" << std::endl;
		return;
	}

	core::kinematics::FoldTree original_ft = pose.fold_tree();
	apply_LH_A_foldtree(pose);
	TR << "repack antigen-antibody interface start: "<<(*scorefxn_)(pose) << std::endl;
	core::pack::task::PackerTaskOP task = interface_tf_->create_task_and_apply_taskoperations(pose);

	PackRotamersMover packer = PackRotamersMover(scorefxn_, task);
	packer.apply(pose);
	TR << "repack antigen-antibody interface end: "<<(*scorefxn_)(pose) << std::endl;
	pose.fold_tree(original_ft);
}

void
AntibodyDesignModeler::repack_antigen_interface(Pose & pose) const {
	vector1< char > antigen_chains = ab_info_->get_antigen_chains();
	if (antigen_chains.size() == 0){
		TR <<" Antigen not present to repack interface" << std::endl;
		return;
	}

	core::kinematics::FoldTree original_ft = pose.fold_tree();
	apply_LH_A_foldtree(pose);
	TR << "repack antigen interface start: "<<(*scorefxn_)(pose) << std::endl;
	core::pack::task::TaskFactoryOP tf = interface_tf_->clone();
	Size L_chain  = core::pose::get_chain_id_from_chain('L', pose);
	Size H_chain = core::pose::get_chain_id_from_chain('H', pose);
	tf->push_back(TaskOperationCOP( new PreventChainFromRepackingOperation(L_chain) ));
	tf->push_back(TaskOperationCOP( new PreventChainFromRepackingOperation(H_chain) ));

	core::pack::task::PackerTaskOP task = tf->create_task_and_apply_taskoperations(pose);

	PackRotamersMover packer = PackRotamersMover(scorefxn_, task);
	packer.apply(pose);
	TR << "repack antigen interface end: "<<(*scorefxn_)(pose) << std::endl;
	pose.fold_tree(original_ft);
}

void
AntibodyDesignModeler::repack_antibody_interface(Pose & pose) const {
	vector1< char > antigen_chains = ab_info_->get_antigen_chains();
	if (antigen_chains.size() == 0){
		TR <<" Antigen not present to detect antibody interface" << std::endl;
		return;
	}

	TR << "Repacking antibody part of interface" << std::endl;
	core::kinematics::FoldTree original_ft = pose.fold_tree();
	apply_LH_A_foldtree(pose);
	TR << "start: "<<(*scorefxn_)(pose) << std::endl;
	core::pack::task::TaskFactoryOP tf = interface_tf_->clone();

	for (Size i = 1; i <= antigen_chains.size(); ++i){
		Size chain = core::pose::get_chain_id_from_chain(antigen_chains[i], pose);
		tf->push_back(TaskOperationCOP( new PreventChainFromRepackingOperation(chain) ));
	}
	core::pack::task::PackerTaskOP task = tf->create_task_and_apply_taskoperations(pose);

	PackRotamersMover packer = PackRotamersMover(scorefxn_, task);
	packer.apply(pose);
	TR << "end: "<<(*scorefxn_)(pose) << std::endl;
	pose.fold_tree(original_ft);
}


void
AntibodyDesignModeler::dock_low_res(Pose& pose, bool pack_interface ) const {
	std::string dock_chains = get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_);
	TR << "Docking " <<dock_chains << " low res " << std::endl;
	vector1< int > movable_jumps(1, 1);
	core::kinematics::FoldTree original_ft = pose.fold_tree();
	//TR<< pose << std::endl;
	//TR << original_ft << std::endl;

	//vector1<char> chains;
	//for (core::Size i = 1; i <= pose.total_residue(); ++i){
	//	chains.push_back(core::pose::get_chain_from_chain_id(pose.chain(i), pose));
	//}

	//std::string chains_s(chains.begin(), chains.end());
	//TR << "Chains: " << chains_s << std::endl;
	protocols::docking::setup_foldtree(pose, dock_chains, movable_jumps);

	TR << "dock low-res start: "<<(*scorefxn_)(pose) << std::endl;
	protocols::docking::DockingLowResOP docker( new protocols::docking::DockingLowRes(docking_scorefxn_low_) );
	protocols::simple_moves::SwitchResidueTypeSetMover cen_switch = protocols::simple_moves::SwitchResidueTypeSetMover("centroid");
	protocols::simple_moves::ReturnSidechainMover return_sc = protocols::simple_moves::ReturnSidechainMover(pose);

	cen_switch.apply(pose);
	docker->apply(pose);
	return_sc.apply(pose);

	pose.fold_tree(original_ft);

	if (pack_interface){
		repack_antigen_ab_interface(pose);
	}
	TR << "dock low-res end: "<<(*scorefxn_)(pose) << std::endl;
}

void
AntibodyDesignModeler::dock_high_res(Pose& pose, core::Size first_cycle, core::Size second_cycle) const {

	std::string dock_chains = get_dock_chains_from_ab_dock_chains(ab_info_, ab_dock_chains_);
	TR << "Docking " <<dock_chains << " high res " << std::endl;
	vector1< int > movable_jumps(1, 1);
	core::kinematics::FoldTree original_ft = pose.fold_tree();
	protocols::docking::setup_foldtree(pose, dock_chains, movable_jumps);
	TR << "dock high-res start: "<<(*scorefxn_)(pose) << std::endl;
	protocols::docking::DockMCMProtocolOP docker( new protocols::docking::DockMCMProtocol(1, docking_scorefxn_high_, scorefxn_) );

	docker->set_first_cycle(first_cycle);
	docker->set_second_cycle(second_cycle);

	docker->apply(pose);
	TR << "dock high-res end: "<<(*scorefxn_)(pose) << std::endl;
	pose.fold_tree(original_ft);
}

}//design
}//antibody
}//protocols
