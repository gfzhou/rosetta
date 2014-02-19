// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/antibody/AntibodyInfo.cc
/// @brief Class for getting antibody-specific objects and information
/// @author Jianqing Xu (xubest@gmail.com)
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

// Project Headers

#include <protocols/antibody/AntibodyInfo.hh>
#include <protocols/antibody/AntibodyEnum.hh>
#include <protocols/antibody/AntibodyEnumManager.hh>
#include <protocols/antibody/clusters/CDRClusterEnum.hh>
#include <protocols/antibody/clusters/CDRClusterEnumManager.hh>

// Core Headers
#include <core/scoring/constraints/ConstraintIO.hh>
#include <core/scoring/constraints/DihedralConstraint.hh>
#include <core/scoring/ScoreType.hh>    // scoring stuff can probably be removed / moved elsewhere
#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/scoring/rms_util.hh>
#include <core/pose/PDBInfo.hh>
#include <core/pose/util.hh>
#include <core/pose/util.tmpl.hh>

// Protocol Headers
#include <protocols/antibody/util.hh>
#include <protocols/rigid/RB_geometry.hh>
#include <protocols/loops/Loop.hh>
#include <protocols/loops/Loops.hh>
#include <protocols/loops/loops_main.hh>  //really?
#include <protocols/toolbox/task_operations/RestrictToInterface.hh>
#include <protocols/simple_moves/ConstraintSetMover.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/FArray1D.hh>
#include <ObjexxFCL/format.hh>

// Utility headers
#include <utility/exit.hh>
#include <utility/io/izstream.hh>
#include <utility/excn/Exceptions.hh>
#include <utility/file/FileName.hh>
#include <utility/file/file_sys_util.hh>
#include <utility/vector1.hh>
#include <utility/vector1.functions.hh>
#include <utility/string_util.hh>

//Options
#include <basic/options/option.hh>
#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/options/keys/antibody.OptionKeys.gen.hh>

// Basic headers
#include <numeric/NumericTraits.hh>
#include <basic/Tracer.hh>
#include <basic/database/open.hh>
#include <math.h>


// Boost headers
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////////////////

static basic::Tracer TR("antibody.AntibodyInfo");

namespace protocols {
namespace antibody {

using namespace basic::options;
using namespace basic::options::OptionKeys;



AntibodyInfo::AntibodyInfo( pose::Pose const & pose,
		AntibodyNumberingSchemeEnum const & numbering_scheme,
		CDRDefinitionEnum const & cdr_definition,
		bool const & cdr_pdb_numbered) //:
		//framework_info_(NULL)
{
	enum_manager_ = new AntibodyEnumManager();
	set_default();

	numbering_info_.numbering_scheme = numbering_scheme;
	numbering_info_.cdr_definition = cdr_definition;
	cdr_pdb_numbered_ = cdr_pdb_numbered;

	identify_antibody(pose);
	init(pose);
}

AntibodyInfo::AntibodyInfo(const pose::Pose& pose, const bool& cdr_pdb_numbered) {
	enum_manager_ = new AntibodyEnumManager();
	set_default();
	cdr_pdb_numbered_ = cdr_pdb_numbered;

	identify_antibody(pose);
	init(pose);
}

AntibodyInfo::~AntibodyInfo() {}

void AntibodyInfo::set_default() {
	is_camelid_ = false;
	InputPose_has_antigen_ = false;
	predicted_H3_base_type_ = Kinked;
	loopsop_having_allcdrs_=NULL;
	
	
	
	std::string numbering_scheme = option [OptionKeys::antibody::numbering_scheme]();
	if (numbering_scheme == "Kabat_Scheme"){
		TR <<"Kabat Numbering scheme is not fully supported due to H1 numbering.  Use with caution. http://www.bioinf.org.uk/abs/" <<std::endl;
	}
	std::string cdr_definition = option [OptionKeys::antibody::cdr_definition]();
	if (! (enum_manager_->numbering_scheme_is_present(numbering_scheme) || enum_manager_->cdr_definition_is_present(cdr_definition))){
		utility_exit_with_message("-numbering_scheme or -cdr_definition not recognized"
		"Recognized Numbering Schemes: Chothia_Scheme, Enhanced_Chothia_Scheme, AHO_Scheme, Kabat_Scheme, IMGT_Scheme"
		"Recognized CDR Definitions: Chothia, Aroop, North, Martin, Kabat");
	}
	numbering_info_.numbering_scheme = enum_manager_->numbering_scheme_string_to_enum(numbering_scheme);
	numbering_info_.cdr_definition = enum_manager_->cdr_definition_string_to_enum(cdr_definition);
}

void AntibodyInfo::init(pose::Pose const & pose) {

	if(is_camelid_) total_cdr_loops_ = camelid_last_loop;
	else            total_cdr_loops_ = num_cdr_loops;

	
	numbering_parser_ = new AntibodyNumberingParser(enum_manager_);
	
	
	cdr_cluster_manager_ = new CDRClusterEnumManager();
	cdr_cluster_set_ = new CDRClusterSet(this);
	
	setup_numbering_info_for_scheme(numbering_info_.numbering_scheme, numbering_info_.cdr_definition);

	setup_CDRsInfo(pose) ;

	setup_FrameWorkInfo(pose) ;

	setup_VL_VH_packing_angle( pose );

	predict_H3_base_type( pose );
	setup_CDR_clusters( pose );

}

void AntibodyInfo::setup_numbering_info_for_scheme(AntibodyNumberingSchemeEnum const & numbering_scheme, CDRDefinitionEnum const & cdr_definition) {

	//////////////////////////////////////////////////////////////////////
	///Oct 2013-
	///Refactored by Jared Adolf-Bryfogle 
	
	numbering_info_ = numbering_parser_->get_antibody_numbering(numbering_scheme, cdr_definition);
	
	packing_angle_numbering_.resize(PackingAngleEnum_total);
	for (core::Size i=1; i <= PackingAngleEnum_total; ++i) {
		packing_angle_numbering_[i].resize(2);
	}
	
	packing_angle_numbering_[VL_sheet_1][1] = 35;
	packing_angle_numbering_[VL_sheet_1][2] = 38;
	packing_angle_numbering_[VL_sheet_2][1] = 85;
	packing_angle_numbering_[VL_sheet_2][2] = 88;

	packing_angle_numbering_[VH_sheet_1][1] = 36;
	packing_angle_numbering_[VH_sheet_1][2] = 39;
	packing_angle_numbering_[VH_sheet_2][1] = 89;
	packing_angle_numbering_[VH_sheet_2][2] = 92;
	
}

bool AntibodyInfo::cdr_definition_transform_present(const CDRDefinitionEnum cdr_definition) const{
	std::map< CDRDefinitionEnum, vector1< vector1< PDBLandmarkOP > > >::const_iterator iter( numbering_info_.cdr_definition_transform.find( cdr_definition ) );
	return iter != numbering_info_.cdr_definition_transform.end();
}

vector1< vector1< PDBLandmarkOP > >
AntibodyInfo::get_cdr_definition_transform(const CDRDefinitionEnum cdr_definition) const {
	

	if (! cdr_definition_transform_present(cdr_definition)){
		utility_exit_with_message("Numbering scheme transform: "+enum_manager_->cdr_definition_enum_to_string(cdr_definition)+\
			" not present for current numbering scheme: "+enum_manager_->cdr_definition_enum_to_string(numbering_info_.cdr_definition));
	}
	else{
		std::map< CDRDefinitionEnum,vector1< vector1< PDBLandmarkOP > > >::const_iterator iter( numbering_info_.cdr_definition_transform.find( cdr_definition ) );
		return iter->second;
	}
}


bool
AntibodyInfo::numbering_scheme_transform_present(const AntibodyNumberingSchemeEnum numbering_scheme) const {

	std::map< AntibodyNumberingSchemeEnum,vector1< PDBLandmarkOP > > ::const_iterator iter( numbering_info_.numbering_scheme_transform.find( numbering_scheme ) );
	return iter != numbering_info_.numbering_scheme_transform.end();
}

vector1< PDBLandmarkOP >
AntibodyInfo::get_numbering_scheme_landmarks(const AntibodyNumberingSchemeEnum numbering_scheme) const {
	if (numbering_scheme_transform_present(numbering_scheme)){
		std::map< AntibodyNumberingSchemeEnum,vector1< PDBLandmarkOP > > ::const_iterator iter( numbering_info_.numbering_scheme_transform.find( numbering_scheme ) );
		return iter->second;
	}
	else {
		utility_exit_with_message("Undefined Antibody numbering scheme in scheme definitions: "+enum_manager_->numbering_scheme_enum_to_string(numbering_scheme));
	}
	
}

core::Size
AntibodyInfo::get_landmark_resnum(core::pose::Pose const & pose, const AntibodyNumberingSchemeEnum scheme, const char chain, const core::Size pdb_resnum,  const char insertion_code) const {
	
	//No conversion is nessassary.
	if (scheme == numbering_info_.numbering_scheme) {
		return pose.pdb_info()->pdb2pose(chain, pdb_resnum, insertion_code);
		TR << "scheme equals numbering scheme" << std::endl;
	}
	
	
	///Check to make sure both schemes are present.
	if (! numbering_scheme_transform_present(scheme)){
		utility_exit_with_message("Numbering scheme landmark info requested, "+enum_manager_->numbering_scheme_enum_to_string(scheme)+ " not present in scheme transform");
	}
	
	if (! numbering_scheme_transform_present(numbering_info_.numbering_scheme)){
		utility_exit_with_message("Numbering scheme landmark for numbering scheme of current PDB,"+enum_manager_->numbering_scheme_enum_to_string(numbering_info_.numbering_scheme)+" not present");
	}
	
	
	vector1< PDBLandmarkOP > landmarks = get_numbering_scheme_landmarks(scheme);
	PDBLandmarkOP query_landmark = new PDBLandmark(chain, pdb_resnum, insertion_code);
	
	//TR<< "Need "<< enum_manager_->numbering_scheme_enum_to_string(scheme)<< " in " << enum_manager_->numbering_scheme_enum_to_string(numbering_info_.numbering_scheme) << std::endl;
	for (core::Size i = 1; i <= landmarks.size(); ++i){
		PDBLandmark landmark = *landmarks[i];
		if (landmark == *query_landmark){
			PDBLandmarkOP new_landmark = get_numbering_scheme_landmarks(numbering_info_.numbering_scheme)[i];
			//TR << "Matched "<< chain << " "<<pdb_resnum << " " << insertion_code << std::endl;
			//TR << "To " << new_landmark->chain() << " " << new_landmark->resnum() << " " << new_landmark->insertion_code()<< std::endl; 
			return pose.pdb_info()->pdb2pose(new_landmark->chain(), new_landmark->resnum(), new_landmark->insertion_code());

		}
		else {
			continue;
		}
	}
	
	///We have reached the end, and the pdb landmark is not found.
	
	utility_exit_with_message("Requested landmark resnum "+utility::to_string(chain)+" "+utility::to_string(pdb_resnum)+" "+utility::to_string(insertion_code)+" not found. ");
}


std::string
AntibodyInfo::get_current_AntibodyNumberingScheme()  const {
	return enum_manager_->numbering_scheme_enum_to_string(numbering_info_.numbering_scheme);
}

std::string
AntibodyInfo::get_current_CDRDefinition()   const {
	return enum_manager_->cdr_definition_enum_to_string(numbering_info_.cdr_definition);
}

void AntibodyInfo::identify_antibody(pose::Pose const & pose){
	
	//Jadofbr (4/2013)  Allow any order in the PDB file by adding this for loop..
	//Todo: Add command-line argument for other types (SCFv, Diabody, etc.)
	bool H_found = false; vector1<char> H_sequence;
	bool L_found = false; vector1<char> L_sequence;

	if (core::pose::has_chain('L', pose)){
		L_found=true;
		L_chain_ = core::pose::get_chain_id_from_chain('L', pose);
		
		for (core::Size x = 1; x<=pose.total_residue(); ++x){
			if (pose.residue(x).chain()==L_chain_){
				L_sequence.push_back(pose.residue(x).name1());
				sequence_map_[x]= pose.residue(x).name1();
			}
		}
	}
	
	if (core::pose::has_chain('H', pose)){
		H_found=true;
		H_chain_ = core::pose::get_chain_id_from_chain('H', pose);
		
		for (core::Size x = 1; x<=pose.total_residue(); ++x){
			if (pose.residue(x).chain()==H_chain_){
				H_sequence.push_back(pose.residue(x).name1());
				sequence_map_[x]= pose.residue(x).name1();
			}
		}
	}
	
	switch (pose.conformation().num_chains() ) {
	case 0:
		throw excn::EXCN_Msg_Exception("the number of chains in the input pose is '0' !!");
		break;
	case 1:
		//if pose has only "1" chain, it is a nanobody
		if ( H_found ) {
			is_camelid_ = true;
			InputPose_has_antigen_=false;
		} else {
			throw excn::EXCN_Msg_Exception("  A): the input pose has only 1 chain, if it is a nanobody, the chain ID is supposed to be 'H' !!");
		}
		break;
	case 2:
		// if pose has "2" chains, it can be 2 possibilities
		// possiblity 1): L and H, regular antibody
		if (  L_found  && H_found ) {
			is_camelid_ = false;
			InputPose_has_antigen_ = false;
		}
		// possiblity 2): H nanobody and antigen
		else if ( H_found) {
			is_camelid_ = true;
			InputPose_has_antigen_ = true;
		} else {
			throw excn::EXCN_Msg_Exception("  B): the input pose has two chains, 1). if it is nanobody, the chain should be 'H'. 2). Light chain SCFv not implemented ");
		}
		break;
	default:
		// if pose has >=3 chains, it can be 2 possibilities
		// possiblity 1): L and H, and antigen
		if(   L_found  &&  H_found ) {
			is_camelid_ = false;
			InputPose_has_antigen_ = true;
		}
		// possiblity 2):  H nanobody and antigen
		else if ( H_found ) {
			is_camelid_ = true;
			InputPose_has_antigen_ = true;
		} else {
			throw excn::EXCN_Msg_Exception("  C). the input pose has more than two chains, but either 1) Light chain SCFv not implemented 2) Antibody is not renumbered ");
		}
		break;
	}

	/// record the antibody sequence
	if (is_camelid_) {
		ab_sequence_ = H_sequence;
	} else {
		//ab_sequence_.reserve(L_sequence.size() + H_sequence.size());
		ab_sequence_.insert(ab_sequence_.end(), L_sequence.begin(), L_sequence.end());
		ab_sequence_.insert(ab_sequence_.end(), H_sequence.begin(), H_sequence.end());

	}
	
	//Get antigen chains if present.
	if (InputPose_has_antigen_){
		for (core::Size i=1; i <= pose.conformation().num_chains(); ++i){
			char chain = core::pose::get_chain_from_chain_id(i, pose);
			if (chain != 'L' && chain != 'H'){
				Chain_IDs_for_antigen_.push_back(chain);
			}
		}
	}
}



//Jadolfbr 3/2012  Refactored to be numbering scheme agnostic.
void AntibodyInfo::setup_CDRsInfo( pose::Pose const & pose ) {


	for (Size i=1; i<=3; ++i) {
		Chain_IDs_for_CDRs_.push_back('H');    // HEAVY chain first
	}
	for (Size i=1; i<=3; ++i) {
		Chain_IDs_for_CDRs_.push_back('L');    // light
	}

	int loop_start_in_pose, loop_stop_in_pose, cut_position ;
	loopsop_having_allcdrs_ = new loops::Loops();

	for (Size i=start_cdr_loop; i <= Size(total_cdr_loops_); ++i ) {
		loop_start_in_pose = pose.pdb_info()->pdb2pose( Chain_IDs_for_CDRs_[i], numbering_info_.cdr_numbering[i][cdr_start]->resnum());
		loop_stop_in_pose= pose.pdb_info()->pdb2pose( Chain_IDs_for_CDRs_[i], numbering_info_.cdr_numbering[i][cdr_end]->resnum());
		if(i != h3 ) {
			cut_position = (loop_stop_in_pose - loop_start_in_pose +1) /2 + loop_start_in_pose;
		} else {
			cut_position = (loop_start_in_pose +1 ) ;
			// JQX:
			// why this is different compared to other cuts of other loops?
			// Aroop seems did this in his old R3 code, CHECK LATER !!!
			// JAB:
			// why is the cutpoint so close to the start of H3?  Has this been shown to be better than the middle somewhere?
		}

		loops::Loop  one_loop(loop_start_in_pose, loop_stop_in_pose, cut_position);
		loops::LoopsOP one_loops = new loops::Loops();
		one_loops->add_loop(one_loop);

		// make a "LoopsOP" object, in which each "Loop" was saved
		loopsop_having_allcdrs_->add_loop(one_loop);

		// make a "vector1" of "LoopsOP" object, each "LoopsOP" has one "Loop" object
		vector1_loopsop_having_cdr_.push_back(one_loops);
	}

	/// FIXME:  ***********************
	loopsop_having_allcdrs_->sequential_order(); /// TODO: kind of dangerous here

	TR<<"Successfully finished the CDR definition"<<std::endl;

}

vector1< vector1<FrameWork> >
AntibodyInfo::get_AntibodyFrameworkInfo() const {
	if (framework_info_.empty()) {
		utility_exit_with_message("Numbering scheme setup failed for Framework.");
	} else {
		return framework_info_;
	}
}

H3BaseTypeEnum
AntibodyInfo::get_Predicted_H3BaseType() const {
	return predicted_H3_base_type_;
}

void AntibodyInfo::setup_FrameWorkInfo( pose::Pose const & pose ) {

	//JAB using landmarks to refactor.  Still don't quite know exactly why these residues.
	//

	FrameWork frmwk;
	vector1<FrameWork> Lfr, Hfr;

	core::Size H_begin_pos_num = 0;
	core::Size H_end_pos_num = 0;
	core::Size L_begin_pos_num = 0;
	core::Size L_end_pos_num = 0;


	if(is_camelid() == true ) {
		H_begin_pos_num=pose.conformation().chain_begin(H_chain_);
		H_end_pos_num=pose.conformation().chain_end(H_chain_);
	} else {
		L_begin_pos_num=pose.conformation().chain_begin(L_chain_);
		L_end_pos_num=pose.conformation().chain_end(L_chain_);
		H_begin_pos_num=pose.conformation().chain_begin(H_chain_);
		H_end_pos_num=pose.conformation().chain_end(H_chain_);;


		if (  L_begin_pos_num   >=    get_landmark_resnum(pose, Chothia_Scheme, 'L', 23)   )  {
			throw excn::EXCN_Msg_Exception( "L chain 1st residue starting after L 23, framework definition failed!!! " );
		}
		if (  L_end_pos_num     <=    get_landmark_resnum(pose, Chothia_Scheme, 'L', 97)   ) {
			throw excn::EXCN_Msg_Exception( "L chain last residue ending before L 97, framework definition failed!!! " );
		}
	}


	if (    H_begin_pos_num    >=     get_landmark_resnum(pose, Chothia_Scheme, 'H', 26 )      ) {
		throw excn::EXCN_Msg_Exception( "H chain 1st residue starting after H 26, framework definition failed!!! " );
	}

	if (    H_end_pos_num      <=     get_landmark_resnum(pose, Chothia_Scheme, 'H', 103)      ) {
		throw excn::EXCN_Msg_Exception( "H chain last residue ending before H 103, framework definition failed!!! " );
	}


	if(! is_camelid_) {
		frmwk.chain_name='L';
		if(   L_begin_pos_num <= get_landmark_resnum(pose, Chothia_Scheme, 'L',5)      ) { // <= 5
			frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',5);
			frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',6);
			Lfr.push_back(frmwk);
		} else if (     L_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'L',6)        ) { // 5 <= x <= 6
			frmwk.start=L_begin_pos_num;
			frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',6);
			Lfr.push_back(frmwk);
		}
		if(   L_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'L',10)      ) { // <= 10
			frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',10);
			frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',23);
			Lfr.push_back(frmwk);
		} else if (   L_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'L',23)           ) { //  10 <= x <=23
			frmwk.start=L_begin_pos_num;
			frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',23);
			Lfr.push_back(frmwk);
		}
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',35);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',38);
		Lfr.push_back(frmwk);
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',45);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',49);
		Lfr.push_back(frmwk);
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',57);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',66);
		Lfr.push_back(frmwk);
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',71);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',88);
		Lfr.push_back(frmwk);
		if (   L_end_pos_num   >=    get_landmark_resnum(pose, Chothia_Scheme, 'L',105)      ) {
			frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',98);
			frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'L',105);
			Lfr.push_back(frmwk);
		} else {
			frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'L',98);
			frmwk.stop=L_end_pos_num;
			Lfr.push_back(frmwk);
		}
	}

	frmwk.chain_name='H';
	if(   H_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'H',5)      ) { // <= 5
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',5);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',6);
		Hfr.push_back(frmwk);
	} else if (   H_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'H',6)      ) { // 5 <= x <= 6
		frmwk.start=H_begin_pos_num;
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',6);
		Hfr.push_back(frmwk);
	}
	if(   H_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'H',10)      ) { // <= 10
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',10);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',25);
		Hfr.push_back(frmwk);
	} else if(   H_begin_pos_num   <=    get_landmark_resnum(pose, Chothia_Scheme, 'H',25)      ) { //  10 <= x <=25
		frmwk.start=H_begin_pos_num;
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',25);
		Hfr.push_back(frmwk);
	}
	frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',36);
	frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',39);
	Hfr.push_back(frmwk);
	frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',46);
	frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',49);
	Hfr.push_back(frmwk);
	frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',66);
	frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',94);
	Hfr.push_back(frmwk);
	if(   H_end_pos_num >=  get_landmark_resnum(pose, Chothia_Scheme, 'H',110)         ) {
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',103);
		frmwk.stop=get_landmark_resnum(pose, Chothia_Scheme, 'H',110);
		Hfr.push_back(frmwk);
	} else {
		frmwk.start=get_landmark_resnum(pose, Chothia_Scheme, 'H',103);
		frmwk.stop=H_end_pos_num;
		Hfr.push_back(frmwk);
	}
	
	
	
	if (Lfr.size()>0)  {
		framework_info_.push_back(Lfr);
	}
	if (Hfr.size()>0)  {
		framework_info_.push_back(Hfr);
	}

	//Removed.  Check is now in get_AntibodyFrameworkInfo.
	//else {utility_exit_with_message("The heavy chain has no framework? This cannot be correct");}


}


void AntibodyInfo::setup_VL_VH_packing_angle( pose::Pose const & pose ) {

	vector1<char> Chain_IDs_for_packing_angle;
	Chain_IDs_for_packing_angle.resize(PackingAngleEnum_total);
	Chain_IDs_for_packing_angle[VL_sheet_1] = 'L';
	Chain_IDs_for_packing_angle[VL_sheet_2] = 'L';
	Chain_IDs_for_packing_angle[VH_sheet_1] = 'H';
	Chain_IDs_for_packing_angle[VH_sheet_2] = 'H';

	Size packing_angle_start_in_pose, packing_angle_stop_in_pose;

	for (Size i=1; i<=PackingAngleEnum_total; ++i) {
		packing_angle_start_in_pose = get_landmark_resnum(pose, Chothia_Scheme, Chain_IDs_for_packing_angle[i], packing_angle_numbering_[i][1]);
		packing_angle_stop_in_pose = get_landmark_resnum(pose, Chothia_Scheme, Chain_IDs_for_packing_angle[i], packing_angle_numbering_[i][2]);
		for (Size j=packing_angle_start_in_pose; j<=packing_angle_stop_in_pose; j++) {
			packing_angle_residues_.push_back( j );
		}
	}
}




////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///	predicit H3 cterminus base type (Kinked or Extended) based on sequence   ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////
void AntibodyInfo::predict_H3_base_type( pose::Pose const & pose ) {
	if( is_camelid_ ) {
		detect_and_set_camelid_CDR_H3_stem_type( pose );
	} else {
		//detect_and_set_regular_CDR_H3_stem_type( pose );
		detect_and_set_regular_CDR_H3_stem_type_new_rule( pose );
	}
} // detect_CDR_H3_stem_type


void AntibodyInfo::detect_and_set_camelid_CDR_H3_stem_type(pose::Pose const & pose ) {
	TR << "AC Detecting Camelid CDR H3 Stem Type" << std::endl;

	bool kinked_H3 (false);
	bool extended_H3 (false);
	
	
	// extract single letter aa codes for the chopped loop residues
	vector1< char > cdr_h3_sequence;
	for( Size ii = get_CDR_loop(h3, pose, Aroop).start() - 2; ii <= get_CDR_loop(h3, pose, Aroop).stop(); ++ii )
		cdr_h3_sequence.push_back( pose.sequence()[ii-1] );

	// Rule for extended
	if( ( ( get_CDR_loop(h3, pose, Aroop).stop() - get_CDR_loop(h3, pose, Aroop).start() ) ) >= 12 ) {
		if( ( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'Y' ) ||
		        ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'W' ) ||
		        ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'F' ) ) &&
		        ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] != 'H' ) &&
		        ( cdr_h3_sequence[ cdr_h3_sequence.size() - 1 ] != 'G' ) )
			extended_H3 = true;
	}

	if( !extended_H3 ) {
		kinked_H3 = true;
		if(           ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'R' ) ||
		              ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'Y' ) ||
		              ((     ( cdr_h3_sequence[ cdr_h3_sequence.size() - 1 ] != 'Y' ) || ( cdr_h3_sequence[ cdr_h3_sequence.size() - 1 ] != 'W' )    ) &&
		               (     ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] != 'Y' ) || ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] != 'W' )    ) &&
		               ( 	  ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] != 'Y' ) || ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] != 'W' )    ))
		  )
			kinked_H3 = false;
	}

	if (kinked_H3) predicted_H3_base_type_ = Kinked;
	if (extended_H3) predicted_H3_base_type_ = Extended;
	if (!kinked_H3 && !extended_H3) predicted_H3_base_type_ = Neutral;

	TR << "AC Finished Detecting Camelid CDR H3 Stem Type: " << enum_manager_->h3_base_type_enum_to_string(predicted_H3_base_type_) << std::endl;
	
	
}


void AntibodyInfo::detect_and_set_regular_CDR_H3_stem_type( pose::Pose const & pose ) {
	TR << "AC Detecting Regular CDR H3 Stem Type" << std::endl;
	bool extended_H3 (false) ;
	bool kinked_H3 (false);
	bool is_H3( false );

	
	// extract single letter aa codes for the chopped loop residues
	vector1< char > cdr_h3_sequence;
	for( Size ii = get_CDR_loop(h3, pose, Aroop).start() - 2; ii <= get_CDR_loop(h3, pose, Aroop).stop()+1 ; ++ii )
		cdr_h3_sequence.push_back( pose.sequence()[ii-1] );

	// Rule 1a for standard kink
	if( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] != 'D') {
		kinked_H3 = true;
		is_H3 = true;
	}

	// Rule 1b for standard extended form
	if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D')
	        && ( (cdr_h3_sequence[2] != 'K') &&
	             (cdr_h3_sequence[2] != 'R') ) && (is_H3 != true)) {
		extended_H3 = true;
		is_H3 = true;
	}

	if( !is_H3 ) {
		// Rule 1b extension for special kinked form
		bool is_basic( false ); // Special basic residue exception flag
		for(Size ii = 3; ii <= Size(cdr_h3_sequence.size() - 4); ++ii) {
			if( cdr_h3_sequence[ii] == 'R' || cdr_h3_sequence[ii] == 'K') {
				is_basic = true;
				break;
			}
		}

		if( !is_basic ) {
			Size L49_pose_number = get_landmark_resnum(pose, Chothia_Scheme, 'L', 49);
			char aa_code_L49 = pose.residue( L49_pose_number ).name1();
			if( aa_code_L49 == 'R' || aa_code_L49 == 'K')
				is_basic = true;
		}
		if( is_basic ) {
			kinked_H3 = true;
			is_H3 = true;
		}
	}

	// Rule 1c for kinked form with salt bridge
	if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D') &&
	        ( (cdr_h3_sequence[2] == 'K') ||
	          (cdr_h3_sequence[2] == 'R') ) &&
	        ( (cdr_h3_sequence[1] != 'K') &&
	          (cdr_h3_sequence[1] != 'R') ) && (is_H3 != true) ) {
		kinked_H3 = true;
		is_H3 = true;
		if( !is_H3 ) {
			bool is_basic( false ); // Special basic residue exception flag
			Size L46_pose_number = get_landmark_resnum(pose, Chothia_Scheme, 'L', 46);
			char aa_code_L46 = pose.residue( L46_pose_number ).name1();
			if( aa_code_L46 == 'R' || aa_code_L46 == 'K')
				is_basic = true;
			if( is_basic ) {
				extended_H3 = true;
				is_H3 = true;
			}
		}
	}

	// Rule 1d for extened form with salt bridge
	if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D') &&
	        ( ( cdr_h3_sequence[ 2 ] == 'K') ||
	          (cdr_h3_sequence[2] == 'R')) &&
	        ( (cdr_h3_sequence[1] == 'K') ||
	          (cdr_h3_sequence[1] == 'R') ) && (is_H3 != true) ) {
		extended_H3 = true;
		is_H3 = true;
	}

	if (kinked_H3) predicted_H3_base_type_ = Kinked;
	if (extended_H3) predicted_H3_base_type_ = Extended;
	if (!kinked_H3 && !extended_H3) predicted_H3_base_type_ = Neutral;
	TR << "AC Finished Detecting Regular CDR H3 Stem Type: " << enum_manager_->h3_base_type_enum_to_string(predicted_H3_base_type_) << std::endl;
	
} // detect_regular_CDR_H3_stem_type()


void AntibodyInfo::detect_and_set_regular_CDR_H3_stem_type_new_rule( pose::Pose const & pose ) {
	TR << "AC Detecting Regular CDR H3 Stem Type" << std::endl;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////HACK/////////////////////////////////////////////////////


	bool extended_H3 (false) ;
	bool kinked_H3 (false);

	// extract single letter aa codes for the chopped loop residues
	vector1< char > cdr_h3_sequence;
	std::string seq = "";
	core::Size start = get_CDR_loop(h3, pose, Aroop).start() - 2;
	core::Size end = get_CDR_loop(h3, pose, Aroop).stop() + 1;
	
	//TR << "Start: "<<start<<"End:"<<end<<std::endl;
	for( Size ii = get_CDR_loop(h3, pose, Aroop).start() - 2; ii <= get_CDR_loop(h3, pose, Aroop).stop() + 1; ++ii ){
		//TR<< utility::to_string(pose.sequence()[ii-1])<< std::endl;
		seq = seq+utility::to_string(pose.sequence()[ii-1]);
		cdr_h3_sequence.push_back( pose.sequence()[ii-1] );
	}
	//for (Size i=1; i<=cdr_h3_sequence.size();++i){    TR<<cdr_h3_sequence[i];} TR<<std::endl;

	/// @author: Daisuke Kuroda (dkuroda1981@gmail.com) 06/18/2012
	///
	/// @last_modified 10/24/2013 (JAB)
	///
	/// @reference Kuroda et al. Proteins. 2008 Nov 15;73(3):608-20.
	///			   Koliansnikov et al. J Bioinform Comput Biol. 2006 Apr;4(2):415-24.

	
	//JAB note.  Changed to use CDR start/end as landmarks for Aroop/Chothia numbering L46/L49/L36.
	// This can be refactored as new landmarks and added to CDRLandmarkEnum and the antibody numbering scheme files in the database if desired.

	// This is only for rule 1b
	core::Size cdr_length = cdr_h3_sequence.size();
	bool is_basic( false ); // Special basic residue exception flag
	if( !is_basic ) {
		Size L49_pose_number = get_landmark_resnum(pose, Chothia_Scheme, 'L', 49);
		char aa_code_L49 = pose.residue( L49_pose_number ).name1();
		if( aa_code_L49 == 'R' || aa_code_L49 == 'K')
			is_basic = true;
	}

	/// START H3-RULE 2007
	if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D') &&
	        ( ( cdr_h3_sequence[ 2 ] == 'K') || (cdr_h3_sequence[2] == 'R') ) &&
	        ( ( cdr_h3_sequence[ 1 ] == 'K') || (cdr_h3_sequence[1] == 'R') ) ) {
		// Rule 1d for extended form with salt bridge
		extended_H3 = true;
	} else if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D') &&
	           ( ( cdr_h3_sequence[ 2 ] == 'K') || ( cdr_h3_sequence[ 2 ] == 'R') ) &&
	           ( ( cdr_h3_sequence[ 1 ] != 'K') && ( cdr_h3_sequence[ 1 ] != 'R') ) ) {
		// Rule 1c for kinked form with salt bridge with/without Notable signal (L46)
		// Special basic residue exception flag
		Size L46_pose_number = get_landmark_resnum(pose, Chothia_Scheme, 'L', 46);
		char aa_code_L46 = pose.residue( L46_pose_number ).name1();

		// Special Tyr residue exception flag
		Size L36_pose_number = get_landmark_resnum(pose, Chothia_Scheme, 'L', 36);
		char aa_code_L36 = pose.residue( L36_pose_number ).name1();

		if( ( aa_code_L46 == 'R' || aa_code_L46 == 'K') && aa_code_L36 != 'Y' ) {
			extended_H3 = true;
		} else {
			kinked_H3   = true;
		}
	} else if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D' ) &&
	           ( cdr_h3_sequence[ 2 ] != 'K' ) && ( cdr_h3_sequence[ 2 ] != 'R' ) &&
	           ( is_basic == true ) ) {
		// Rule 1b for standard extended form with Notable signal (L49)
		kinked_H3   = true;
	} else if( ( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'F' ) &&
	             ( cdr_h3_sequence[ cdr_h3_sequence.size() - 4 ] == 'A' ) ) ||
	           ( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'F' ) &&
	             ( cdr_h3_sequence[ cdr_h3_sequence.size() - 4 ] == 'G' ) ) ||
	           ( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'M' ) &&
	             ( cdr_h3_sequence[ cdr_h3_sequence.size() - 4 ] == 'A' ) ) ||
	           ( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'M' ) &&
	             ( cdr_h3_sequence[ cdr_h3_sequence.size() - 4 ] == 'G' ) ) ) {
		// This is new feature
		kinked_H3   = true;
	} else if( ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'R' ) ||
	           ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'K' ) ||
	           ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'D' ) ||
	           ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'N' ) ) {
		// This is new feature
		extended_H3 = true;
	} else if( ( ( cdr_h3_sequence[ 3 ] == 'Y' ) &&
	             ( cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'F' ) ) ||
	           ( (cdr_h3_sequence[ 3 ] == 'Y' ) &&
	             (cdr_h3_sequence[ cdr_h3_sequence.size() - 3 ] == 'M') ) ) {
		// This is new feature
		extended_H3 = true;
	} else if( cdr_h3_sequence.size() - 3  == 7 ) {
		// This is new feature
		extended_H3 = true;
	} else if( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] == 'D' ) {
		// Rule 1b for standard extended form without Notable signal (L49)
		extended_H3 = true;
	} else if( cdr_h3_sequence[ cdr_h3_sequence.size() - 2 ] != 'D' ) {
		// Rule 1a for standard kink. i.e. No sequence feature...
		kinked_H3 = true;
	}
	// END H3-RULE 2007

	if (kinked_H3) predicted_H3_base_type_ = Kinked;
	if (extended_H3) predicted_H3_base_type_ = Extended;
	if (!kinked_H3 && !extended_H3) predicted_H3_base_type_ = Neutral;
	TR << "AC Finished Detecting Regular CDR H3 Stem Type: " << enum_manager_->h3_base_type_enum_to_string(predicted_H3_base_type_) << std::endl;

	TR << "AC Finished Detecting Regular CDR H3 Stem Type: "
	   << "Kink: " << kinked_H3 << " Extended: " << extended_H3 << std::endl;
	
} // detect_regular_CDR_H3_stem_type()


Size
AntibodyInfo::get_CDR_start(CDRNameEnum const & cdr_name, pose::Pose const & pose) const {
	
	PDBLandmark landmark = *(numbering_info_.cdr_numbering[cdr_name][cdr_start]);
	return pose.pdb_info()->pdb2pose(Chain_IDs_for_CDRs_[cdr_name], landmark.resnum(), landmark.insertion_code());

}

Size
AntibodyInfo::get_CDR_start(CDRNameEnum const & cdr_name, pose::Pose const &  pose, CDRDefinitionEnum const & transform) const {
	
	if (transform == numbering_info_.cdr_definition){
		return get_CDR_start(cdr_name, pose);
	}
	else {
		PDBLandmark landmark = *(get_cdr_definition_transform(transform)[cdr_name][cdr_start]);
		return pose.pdb_info()->pdb2pose(Chain_IDs_for_CDRs_[cdr_name], landmark.resnum(), landmark.insertion_code());
	}
}


Size
AntibodyInfo::get_CDR_end(CDRNameEnum const & cdr_name, pose::Pose const & pose) const {
	PDBLandmark landmark = *(numbering_info_.cdr_numbering[cdr_name][cdr_end]);
	return pose.pdb_info()->pdb2pose(Chain_IDs_for_CDRs_[cdr_name], landmark.resnum(), landmark.insertion_code());
}

Size
AntibodyInfo::get_CDR_end(CDRNameEnum const & cdr_name, pose::Pose const & pose, CDRDefinitionEnum const & transform) const {
	
	if (transform == numbering_info_.cdr_definition){
		return get_CDR_end(cdr_name, pose);
	}
	else {
		PDBLandmark landmark = *(get_cdr_definition_transform(transform)[cdr_name][cdr_end]);
		return pose.pdb_info()->pdb2pose(Chain_IDs_for_CDRs_[cdr_name], landmark.resnum(), landmark.insertion_code());
	}
}

	/// @brief return the loop of a certain loop type
loops::Loop
AntibodyInfo::get_CDR_loop( CDRNameEnum const & cdr_name ) const {
	
	loops::Loop loop = (*vector1_loopsop_having_cdr_[cdr_name])[1];
	return loop;
}
	


loops::Loop
AntibodyInfo::get_CDR_loop( CDRNameEnum const & cdr_name, pose::Pose const & pose) const {
	
	core::Size start = get_CDR_start(cdr_name, pose);
	core::Size stop =  get_CDR_end(cdr_name, pose);
	core::Size cutpoint = (stop-start+1)/2+start;
	protocols::loops::Loop cdr_loop = protocols::loops::Loop(start, stop, cutpoint);
	return cdr_loop;
	
}

loops::Loop
AntibodyInfo::get_CDR_loop(CDRNameEnum const & cdr_name, core::pose::Pose const & pose, CDRDefinitionEnum const & transform) const{
	
	if (transform == numbering_info_.cdr_definition){
		return get_CDR_loop(cdr_name, pose);
	}
	else {
		core::Size start = get_CDR_start(cdr_name, pose, transform);
		core::Size stop =  get_CDR_end(cdr_name, pose, transform);
		core::Size cutpoint = (stop-start+1)/2+start;
		protocols::loops::Loop cdr_loop = protocols::loops::Loop(start, stop, cutpoint);
		return cdr_loop;
	}
	
}

loops::LoopsOP
AntibodyInfo::get_CDR_loops(pose::Pose const & pose) const {
	
	protocols::loops::LoopsOP cdr_loops = new loops::Loops;
	for (Size i = 1; i <= Size(total_cdr_loops_); ++i){
		CDRNameEnum cdr = static_cast<CDRNameEnum>(i);
		protocols::loops::Loop cdr_loop =get_CDR_loop(cdr, pose);
		cdr_loops->add_loop(cdr_loop);
	}
	return cdr_loops;
}

/// @brief return the loop of a certain loop type
loops::LoopsOP
AntibodyInfo::get_CDR_in_loopsop( CDRNameEnum const & cdr_name ) const {
	
	return vector1_loopsop_having_cdr_[cdr_name];

}
	
/// @brief return a LoopsOP object, initialized upon class construction.
loops::LoopsOP
AntibodyInfo::get_AllCDRs_in_loopsop() const {

	return loopsop_having_allcdrs_;
}


std::vector<Vector>
AntibodyInfo::kink_anion_atoms(const core::pose::Pose & pose) const {
	Size resi = get_CDR_loop(h3, pose, Aroop).stop() - 1;
	core::conformation::Residue res = pose.residue(resi);
	//print "H3_N-1 (%i): %s" % (resi,res.name3())
	std::vector<Vector> atoms;
	switch (res.name1()) {
	case 'D' :
		atoms.push_back(res.xyz("OD1"));
		atoms.push_back(res.xyz("OD2"));
		break;
	case 'E' :
		atoms.push_back(res.xyz("OD1"));
		atoms.push_back(res.xyz("OD2"));
		break;
	}
	return atoms;
}

/// @brief return side chain cation atoms (typically Lys/His nitrogens) in the kink bulge HBond
std::vector<Vector>
AntibodyInfo::kink_cation_atoms(const core::pose::Pose & pose) const {
	Size resi = get_CDR_loop(h3, pose, Aroop).start() - 1;
	core::conformation::Residue res = pose.residue(resi);
	//print "H3_0   (%i): %s" % (resi,res.name3())
	std::vector<Vector> atoms;
	switch (res.name1()) {
	case 'R' :
		atoms.push_back(res.xyz("NH1"));
		atoms.push_back(res.xyz("NH2"));
		break;
	case 'K' :
		atoms.push_back(res.xyz("NZ"));
		break;
	}
	return atoms;
}



void
AntibodyInfo::setup_CDR_clusters(pose::Pose const & pose) {

	TR << "Setting up CDR Clusters" << std::endl;

	cdr_cluster_set_->clear();
	for (core::Size i=1; i <= core::Size(total_cdr_loops_); ++i) {
		CDRNameEnum cdr_name = static_cast<CDRNameEnum>(i);
		cdr_cluster_set_->identify_and_set_cdr_cluster(pose, cdr_name);
		CDRClusterOP cluster_id = get_CDR_cluster(cdr_name);
		//TR <<get_CDR_name(cdr_name)+" North cluster: "<< get_cluster_name(cluster_id->cluster()) +" normalized_distance(deg): "<< cluster_id->normalized_distance_in_degrees() << std::endl;
	}
}

CDRClusterOP
AntibodyInfo::get_CDR_cluster(CDRNameEnum const cdr_name) const  {
	
	if (cdr_cluster_set_->empty(cdr_name)){
		utility_exit_with_message("CDR cluster does not exist for CDR. ");
	}
	
	return cdr_cluster_set_->get_cluster_data(cdr_name);
}

bool
AntibodyInfo::has_cluster_for_cdr(const CDRNameEnum cdr_name) const {
	return !cdr_cluster_set_->empty(cdr_name);
}

std::string
AntibodyInfo::get_CDR_name(CDRNameEnum const & cdr_name) const {
	return enum_manager_->cdr_name_enum_to_string(cdr_name);
}

CDRNameEnum
AntibodyInfo::get_CDR_name_enum(std::string const & cdr_name) const {
	return enum_manager_->cdr_name_string_to_enum(cdr_name);
}

Size
AntibodyInfo::get_CDR_length(CDRNameEnum const & cdr_name) const {
	loops::Loop cdr_loop = get_CDR_loop(cdr_name);
	Size len = cdr_loop.stop()-cdr_loop.start()+1;
	return len;
}

Size
AntibodyInfo::get_CDR_length(CDRNameEnum const & cdr_name, core::pose::Pose const & pose, CDRDefinitionEnum const & transform) const {
	loops::Loop cdr_loop = get_CDR_loop(cdr_name, pose, transform);
	Size len = cdr_loop.stop()-cdr_loop.start()+1;
	return len;
}

std::string
AntibodyInfo::get_cluster_name(CDRClusterEnum const cluster) const {
	return cdr_cluster_manager_->cdr_cluster_enum_to_string(cluster);
}

CDRClusterEnum
AntibodyInfo::get_cluster_enum(const std::string cluster) const {
	return cdr_cluster_manager_->cdr_cluster_string_to_enum(cluster);
}

CDRNameEnum
AntibodyInfo::get_cdr_enum_for_cluster(CDRClusterEnum const cluster) const {
		std::string cluster_string = cdr_cluster_manager_->cdr_cluster_enum_to_string(cluster);
		utility::vector1< std::string > cluster_vec = utility::string_split(cluster_string, '-');
		return enum_manager_->cdr_name_string_to_enum(cluster_vec[1]);
	}
	
core::Size
AntibodyInfo::get_cluster_length(CDRClusterEnum const cluster) const {
		std::string cluster_string = cdr_cluster_manager_->cdr_cluster_enum_to_string(cluster);
		utility::vector1< std::string > cluster_vec = utility::string_split(cluster_string, '-');
		return utility::string2int(cluster_vec[2]);
	}


////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///       provide fold tree utilities for various purpose                    ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

kinematics::FoldTreeCOP AntibodyInfo::setup_simple_fold_tree(
    Size const & jumppoint1,
    Size const & cutpoint,
    Size const & jumppoint2,
    pose::Pose const & pose ) const {
	using namespace kinematics;

	FoldTreeOP f = new FoldTree();
	loops::Loops moveable_region;
	moveable_region.add_loop(loops::Loop(jumppoint1 + 1, jumppoint2 - 1, cutpoint));
	loops::fold_tree_from_loops(pose, moveable_region, *f);
	return f;

}


kinematics::FoldTreeCOP AntibodyInfo::get_FoldTree_AllCDRs (pose::Pose const & pose ) const {
	using namespace kinematics;

	kinematics::FoldTreeOP f = new FoldTree();
	loops::fold_tree_from_loops(pose, *(get_AllCDRs_in_loopsop()), *f);
	return f;

} // all_cdr_fold_tree()

///////////////////////////////////////////////////////////////////////////
/// @begin all_cdr_VL_VH_fold_tree
///
/// @brief change to all CDR and VL-VH dock fold tree
///
/// @author Aroop 07/13/2010
/// @author Brian D. Weitzner
///
/// @last_modified 06/25/2013
///////////////////////////////////////////////////////////////////////////
kinematics::FoldTreeCOP
AntibodyInfo::get_FoldTree_AllCDRs_LHDock( pose::Pose const & pose ) const {

	using namespace kinematics;

	// Start by creating a FoldTree for all of the CDR loops.
	// NOTE: Making deep copy becuase this method returns a FoldTreeCOP and we need to perform additional configuration.
	FoldTreeOP f = new FoldTree(*get_FoldTree_AllCDRs(pose));
	
	// NOTE: This class assumes the first chain is the light chain,
	// the second chain is the heavy chain and any other chains are
	// the antigen.
	Size end_of_light_chain = pose.conformation().chain_end(1);
	Size light_chain_COM = geometry::residue_center_of_mass(pose, 1, end_of_light_chain);
	Size heavy_chain_COM = geometry::residue_center_of_mass(pose, end_of_light_chain + 1, pose.total_residue());

	// Adjust the FoldTree that is produced by fold_tree_from_loops to have no edges that span the light and heavy chains..
	// Specifically, this will:
	//     1) Add an edge from the end of L3 to the end of the light chain, a
	//     2) Add an edge from the beginning of the heavy chain to the beginning of H1
	//     3) Delete the existing edge from the end of L3 to the beginning of H1
	Edge chain_spanning_edge = f->get_residue_edge(end_of_light_chain);
	f->add_edge(chain_spanning_edge.start(), end_of_light_chain, Edge::PEPTIDE);
	f->add_edge(end_of_light_chain + 1, chain_spanning_edge.stop(), Edge::PEPTIDE);
	f->delete_edge(chain_spanning_edge);

	// Make sure rb jumps do not reside in the loop region
	// NOTE: This check is insufficient.  Perhaps the jump adjustment should be done in a while loop.
	for(loops::Loops::const_iterator it= get_AllCDRs_in_loopsop()->begin(), it_end = get_AllCDRs_in_loopsop()->end();
		it != it_end; ++it) {
		if (light_chain_COM >= (it->start() - 1) && light_chain_COM <= (it->stop() + 1)) {
			light_chain_COM = it->stop() + 2;
		}
		else if (heavy_chain_COM >= (it->start() - 1) && heavy_chain_COM <= (it->stop() + 1)) {
			heavy_chain_COM = it->start() - 2;
		}
	}

	// Ok, so right now we have a FoldTree set up for all of the CDR loops and we want to modify it to also include a
	// jump from light_chain_COM to heavy_chain_COM.

	// Let's make sure the rigid body docking jump is always labelled "1"
	Edge old_first_jump = f->jump_edge(1);
	f->update_edge_label(old_first_jump.start(), old_first_jump.stop(), old_first_jump.label(), f->num_jump() + 1);

	// Make the jump and always label it "1"
	f->add_edge(light_chain_COM, heavy_chain_COM, 1);

	// Find the edges that will be affected.
	Edge light_chain_edge = f->get_residue_edge(light_chain_COM);
	Edge heavy_chain_edge = f->get_residue_edge(heavy_chain_COM);

	// Add new edges directed toward the jump points
	// Light chain
	f->add_edge(light_chain_edge.start(), light_chain_COM, Edge::PEPTIDE);
	f->add_edge(light_chain_edge.stop(), light_chain_COM, Edge::PEPTIDE);
	
	// Heavy chain
	f->add_edge(heavy_chain_edge.start(), heavy_chain_COM, Edge::PEPTIDE);
	f->add_edge(heavy_chain_edge.stop(), heavy_chain_COM, Edge::PEPTIDE);

	// Delete the old edges
	f->delete_edge(light_chain_edge);
	f->delete_edge(heavy_chain_edge);

	f->reorder(1);
	return f;

}


///////////////////////////////////////////////////////////////////////////
/// @begin LH_A_foldtree
///
/// @brief	Fold tree for snugdock, docks LH with the antigen chains. The function
///			assumes that the coordinates for antigen chains in the input PDB file
///			are right after the antibody heavy chain (which must be named H).The
///			expected order of chains is thus L, H followed by the antigen chains.
///
/// @authors Krishna Praneeth Kilambi 08/14/2012
///
/// @last_modified 08/14/2012
///////////////////////////////////////////////////////////////////////////
kinematics::FoldTree
AntibodyInfo::get_FoldTree_LH_A( pose::Pose const & pose ) const {
    
	using namespace core;
	using namespace kinematics;
    
	Size nres = pose.total_residue();
	pose::PDBInfoCOP pdb_info = pose.pdb_info();
	char second_chain = 'H';
	Size cutpoint = 0;
    
	kinematics::FoldTree LH_A_foldtree;
    
	for ( Size i = 1; i <= nres; ++i ) {
		if(pdb_info->chain(1) != 'L'){
			throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
		break;
		}
		if( (pdb_info->chain(i) == 'L') && (pdb_info->chain(i) != pdb_info->chain(i+1))) {
			if(pdb_info->chain(i+1) != second_chain){
				throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
				break;
			}
		}
		if( (pdb_info->chain(i) == second_chain) && (pdb_info->chain(i) != pdb_info->chain(i+1))) {
			cutpoint = i;
			break;
		}
	}
    
	Size jump_pos1 ( geometry::residue_center_of_mass( pose, 1, cutpoint ) );
	Size jump_pos2 ( geometry::residue_center_of_mass( pose, cutpoint+1, pose.total_residue() ) );
    
	//setup fold tree based on cutpoints and jump points
	LH_A_foldtree.clear();
	LH_A_foldtree.simple_tree( pose.total_residue() );
	LH_A_foldtree.new_jump( jump_pos1, jump_pos2, cutpoint);
    
	Size chain_begin(0), chain_end(0);
    
	//rebuild jumps between antibody light and heavy chains
	chain_end = cutpoint;
	chain_begin = pose.conformation().chain_begin( pose.chain(chain_end) );
	while (chain_begin != 1){
		chain_end = chain_begin-1;
		LH_A_foldtree.new_jump( chain_end, chain_begin, chain_end);
		chain_begin = pose.conformation().chain_begin( pose.chain(chain_end) );
	}
    
	//rebuild jumps between all the antigen chains
	chain_begin = cutpoint+1;
	chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	while (chain_end != pose.total_residue()){
		chain_begin = chain_end+1;
		LH_A_foldtree.new_jump( chain_end, chain_begin, chain_end);
		chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	}
    
	LH_A_foldtree.reorder( 1 );
	LH_A_foldtree.check_fold_tree();
    
	 return LH_A_foldtree;
}


///////////////////////////////////////////////////////////////////////////
/// @begin L_HA_foldtree
///
/// @brief	Fold tree for LH refinement in snugdock, docks L with H + antigen
///			chains. The function assumes that the coordinates for antigen chains
///			in the input PDB file are right after the antibody heavy chain
///			(which must be named H).The expected order of chains is thus
///			L, H followed by the antigen chains.
///
/// @authors Krishna Praneeth Kilambi 08/14/2012
///
/// @last_modified 08/14/2012
///////////////////////////////////////////////////////////////////////////
kinematics::FoldTree
AntibodyInfo::get_FoldTree_L_HA( pose::Pose const & pose ) const {

	using namespace core;
	using namespace kinematics;

	Size nres = pose.total_residue();
	pose::PDBInfoCOP pdb_info = pose.pdb_info();
	char second_chain = 'H';
	Size cutpoint = 0;

	kinematics::FoldTree L_HA_foldtree;

	for ( Size i = 1; i <= nres; ++i ) {
		if(pdb_info->chain(1) != 'L') {
			throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
			break;
		}
		if( (pdb_info->chain(i) == 'L') && (pdb_info->chain(i) != pdb_info->chain(i+1))) {
			if(pdb_info->chain(i+1) != second_chain) {
				throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
				break;
			}
		}
		if( (pdb_info->chain(i) == 'L') && (pdb_info->chain(i+1) == second_chain)) {
			cutpoint = i;
			break;
		}
	}

	Size jump_pos1 ( geometry::residue_center_of_mass( pose, 1, cutpoint ) );
	Size jump_pos2 ( geometry::residue_center_of_mass( pose, cutpoint+1, pose.conformation().chain_end( pose.chain(cutpoint+1) ) ) );

	//setup fold tree based on cutpoints and jump points
	L_HA_foldtree.clear();
	L_HA_foldtree.simple_tree( pose.total_residue() );
	L_HA_foldtree.new_jump( jump_pos1, jump_pos2, cutpoint);

	Size chain_begin(0), chain_end(0);

	//rebuild jumps between heavy chain and antigen chains
	chain_begin = cutpoint+1;
	chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	while (chain_end != pose.total_residue()) {
		chain_begin = chain_end+1;
		L_HA_foldtree.new_jump( chain_end, chain_begin, chain_end);
		chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	}

	L_HA_foldtree.reorder( 1 );
	L_HA_foldtree.check_fold_tree();

	return L_HA_foldtree;

}

///////////////////////////////////////////////////////////////////////////
/// @begin LA_H_foldtree
///
/// @brief	Fold tree for LH refinement in snugdock, docks L + antigen chains
///			with H. The function assumes that the coordinates for antigen chains
///			in the input PDB file are right after the antibody heavy chain
///			(which must be named H).The expected order of chains is thus
///			L, H followed by the antigen chains.
///
/// @authors Krishna Praneeth Kilambi 08/14/2012
///
/// @last_modified 08/14/2012
///////////////////////////////////////////////////////////////////////////
kinematics::FoldTree
AntibodyInfo::get_FoldTree_LA_H( pose::Pose const & pose ) const {

	using namespace core;
	using namespace kinematics;

	Size nres = pose.total_residue();
	pose::PDBInfoCOP pdb_info = pose.pdb_info();
	char second_chain = 'H';
	Size cutpoint = 0;
	bool lchain_jump = false;

	kinematics::FoldTree LA_H_foldtree ;

	for ( Size i = 1; i <= nres; ++i ) {
		if(pdb_info->chain(1) != 'L') {
			throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
			break;
		}
		if( (pdb_info->chain(i) == 'L') && (pdb_info->chain(i) != pdb_info->chain(i+1))) {
			if(pdb_info->chain(i+1) != second_chain) {
				throw excn::EXCN_Msg_Exception("Chains are not named correctly or are not in the expected order");
				break;
			}
		}
		if( (pdb_info->chain(i) == 'L') && (pdb_info->chain(i+1) == second_chain)) {
			cutpoint = i;
			break;
		}
	}

	Size jump_pos1 ( geometry::residue_center_of_mass( pose, 1, cutpoint ) );
	Size jump_pos2 ( geometry::residue_center_of_mass( pose, cutpoint+1, pose.conformation().chain_end( pose.chain(cutpoint+1) ) ) );

	//setup fold tree based on cutpoints and jump points
	LA_H_foldtree.clear();
	LA_H_foldtree.simple_tree( pose.total_residue() );
	LA_H_foldtree.new_jump( jump_pos1, jump_pos2, cutpoint);

	Size chain_begin(0), chain_end(0);

	//rebuild jumps between the light chain and antigen chains
	chain_begin = cutpoint+1;
	chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	while (chain_end != pose.total_residue()) {
		chain_begin = chain_end+1;
		if (!lchain_jump) {
			LA_H_foldtree.new_jump( pose.conformation().chain_end( pose.chain(1) ), chain_begin, chain_end);
			lchain_jump = true;
		} else {
			LA_H_foldtree.new_jump( chain_end, chain_begin, chain_end);
		}
		chain_end = pose.conformation().chain_end( pose.chain(chain_begin) );
	}

	LA_H_foldtree.reorder( 1 );
	LA_H_foldtree.check_fold_tree();

	return LA_H_foldtree;


}

kinematics::MoveMap
AntibodyInfo::get_MoveMap_for_Loops(pose::Pose const & pose,
                                    loops::Loops const & the_loops,
                                    bool const & bb_only,
                                    bool const & include_nb_sc,
                                    Real const & nb_dist) const {
	kinematics::MoveMap move_map ;
	
	move_map.clear();
	move_map.set_chi( false );
	move_map.set_bb( false );
	utility::vector1< bool> bb_is_flexible( pose.total_residue(), false );
	utility::vector1< bool> sc_is_flexible( pose.total_residue(), false );
	
	select_loop_residues( pose, the_loops, false/*include_neighbors*/, bb_is_flexible, nb_dist);
	move_map.set_bb( bb_is_flexible );
	if (bb_only==false) {
		select_loop_residues( pose, the_loops, include_nb_sc/*include_neighbors*/, sc_is_flexible, nb_dist);
		move_map.set_chi( sc_is_flexible );
	}
	for( Size ii = 1; ii <= the_loops.num_loop(); ++ii ) {
		move_map.set_jump( ii, false );
	}

	return move_map;
}

kinematics::MoveMap
AntibodyInfo::get_MoveMap_for_AllCDRsSideChains_and_H3backbone(pose::Pose const & pose,
        bool const & include_nb_sc ,
        Real const & nb_dist ) const {

	kinematics::MoveMap move_map ;

	move_map.clear();
	move_map.set_chi( false );
	move_map.set_bb( false );
	utility::vector1< bool> bb_is_flexible( pose.total_residue(), false );
	utility::vector1< bool> sc_is_flexible( pose.total_residue(), false );

	//TR<<"start: "<<get_CDR_start(h3, pose)<<std::endl;
	//TR<<"end  : "<<get_CDR_end(h3, pose)<<std::endl;
	for(Size ii=get_CDR_start(h3, pose); ii<=get_CDR_end(h3, pose); ii++){
		bb_is_flexible[ii] = true;
		TR<<"Setting Residue "<< ii<<" to be true"<<std::endl;
	}
	move_map.set_bb( bb_is_flexible );

	select_loop_residues( pose, *(get_AllCDRs_in_loopsop()), include_nb_sc, sc_is_flexible, nb_dist);
	move_map.set_chi( sc_is_flexible );

	for( Size ii = 1; ii <= (get_AllCDRs_in_loopsop())->num_loop(); ++ii ){
		move_map.set_jump( ii, false );
	}


	return move_map;
}

void
AntibodyInfo::add_CDR_to_MoveMap(pose::Pose const & pose,
                                 kinematics::MoveMapOP movemap,
                                 CDRNameEnum const & cdr_name,
                                 bool const & bb_only,
                                 bool const & include_nb_sc,
                                 Real const & nb_dist) const {

	core::Size start = get_CDR_start(cdr_name, pose);
	core::Size stop =  get_CDR_end(cdr_name, pose);
	core::Size cut = (stop-start+1)/2+start;

	loops::Loop cdr_loop = loops::Loop(start, stop, cut);
	utility::vector1< bool> bb_is_flexible( pose.total_residue(), false );
	select_loop_residues( pose, cdr_loop, include_nb_sc/*include_neighbors*/, bb_is_flexible, nb_dist);
	for (core::Size i=1; i<=pose.total_residue(); ++i) {
		if (bb_is_flexible[i]) {
			movemap->set_bb(i, true);
			if (! bb_only) {
				movemap->set_chi(i, true);
			}
		}
	}
}

kinematics::MoveMap
AntibodyInfo::get_MoveMap_for_LoopsandDock(pose::Pose const & pose,
        loops::Loops const & the_loops,
        bool const & bb_only,
        bool const & include_nb_sc,
        Real const & nb_dist) const {

	kinematics::MoveMap move_map = get_MoveMap_for_Loops(pose, the_loops, bb_only, include_nb_sc, nb_dist);

	move_map.set_jump( 1, true );
	for( Size ii = 2; ii <= the_loops.num_loop() +1 ; ++ii ) {
		move_map.set_jump( ii, false );
	}

	return move_map;
}




//JQX: doesn't matter only antibody or antibody-antigen complex, just include CDRs and their neighbors
pack::task::TaskFactoryOP
AntibodyInfo::get_TaskFactory_AllCDRs(pose::Pose & pose)  const {

	vector1< bool> sc_is_packable( pose.total_residue(), false );
	select_loop_residues( pose, *get_AllCDRs_in_loopsop(), true/*include_neighbors*/, sc_is_packable);

	using namespace pack::task;
	using namespace pack::task::operation;
	// selecting movable c-terminal residues
	ObjexxFCL::FArray1D_bool loop_residues( pose.total_residue(), false );
	for( Size i = 1; i <= pose.total_residue(); ++i ) {
		loop_residues(i) = sc_is_packable[i];
	} // check mapping

	using namespace protocols::toolbox::task_operations;
	pack::task::TaskFactoryOP tf ;
	tf= setup_packer_task(pose);
	//	tf->push_back( new RestrictToInterface(loop_residues) ); //JQX: not sure why we use loop_residues, in stead of sc_is_packable
	tf->push_back( new RestrictToInterface(sc_is_packable) );



	//pack::task::PackerTaskOP my_task2(tf->create_task_and_apply_taskoperations(pose));
	//TR<<*my_task2<<std::endl; //exit(-1);

	return tf;
}

pack::task::TaskFactoryOP
AntibodyInfo::get_TaskFactory_OneCDR(pose::Pose & pose, CDRNameEnum const & cdr_name)  const {
	vector1< bool> sc_is_packable( pose.total_residue(), false );

	select_loop_residues( pose, *get_CDR_in_loopsop(cdr_name), true/*include_neighbors*/, sc_is_packable);
	using namespace protocols::toolbox::task_operations;

	pack::task::TaskFactoryOP tf ;
	tf= setup_packer_task(pose);
	tf->push_back( new RestrictToInterface(sc_is_packable) );

	return tf;
}

/// TODO:
//JQX: make Daisuke's code compatible with my code
//JAB: Expand using HMMER antibody/cdr identification

///////////////////////////////////////////////////////////////////////////
/// @author: Daisuke Kuroda (dkuroda1981@gmail.com) 06/18/2012
///
/// @brief: Identify 3 CDRs from a sequence. Automatically judge heavy or light chains (I hope!).
///         The input can be either a light chain, a heavy chain or another sequence.
///
/// @last_modified 08/28/2012 by DK
///////////////////////////////////////////////////////////////////////////
void
AntibodyInfo::identify_CDR_from_a_sequence(std::string const & querychain) {

	int l1found = 0, l2found = 0, l3found = 1, h1found = 1, h2found = 0, h3found = 1; // 0 if exist; otherwise 1.
	int lenl1 = 0, lenl2 = 0, lenl3 = 0, lenh1 = 0, lenh2 = 0, lenh3 = 0;
	int posl1_s = 0, posl1_e = 0, posl2_s = 0, posl2_e = 0, posl3_s = 0, posl3_e = 0;
	int posh1_s = 0, posh1_e = 0, posh2_s = 0, posh2_e = 0, posh3_s = 0, posh3_e = 0;
	// int i = 0, // Unused variable causes warning.
	int k = 0, l = 0, m = 0, n = 0;

	int pos_fr1_s = 0, pos_fr1_e = 0, pos_fr2_s = 0, pos_fr2_e = 0;
	int pos_fr3_s = 0, pos_fr3_e = 0, pos_fr4_s = 0, pos_fr4_e = 0;
	int len_fr1 = 0, len_fr2 = 0, len_fr3 = 0, len_fr4 = 0;

	int len;
	std::string check;

	std::string seql1, seql2, seql3, seqh1, seqh2, seqh3;
	std::string frl3, frh1, frh3;

	std::string seq_fr1, seq_fr2, seq_fr3, seq_fr4;

	// For L3:	[FVI]-[GAEV]-X-[GY]
	std::string p1_l3[] = {"F","V","I"};
	std::string p2_l3[] = {"G","A","E","V"};
	std::string p3_l3[] = {"G","A","P","C","D","E","Q","N","R","K","H","W","Y","F","M","T","V","I","S","L"};
	std::string p4_l3[] = {"G","Y"};

	// For H1
	std::string p1_h1[] = {"W","L"};
	std::string p2_h1[] = {"I","V","F","Y","A","M","L","N","G","E","W"};
	std::string p3_h1[] = {"R","K","Q","V","N","C"};
	std::string p4_h1[] = {"Q","K","H","E","L","R"};

	// For H3	W-[GA]-X-[DRG]
	// std::string p1_h3[] = {"W","V"}; // Unused variable causes warning.
	std::string p2_h3[] = {"G","A","C"};
	std::string p3_h3[] = {"A","P","C","D","E","Q","N","R","K","H","W","Y","F","M","T","V","I","S","L","G"};
	std::string p4_h3[] = {"G","R","D","Q","V"};

	// Input sequence is here
	std::string querychain2(querychain, 0,140);
	std::string querychain3(querychain, 0,110);

	std::string querychain_first(querychain, 0, 50);
	std::string querychain_last(querychain, 70);

	//TR << querychain2 << endl;
	//TR << querychain_last << endl;

	len = querychain.length();
	if(len < 130) {
		check = "Fv";
	} else if(len < 250) {
		check = "Fab";
	} else {
		check = "Weird";
	}

	//TR << "*** Query sequence ***" << endl;
	//TR << querychain << endl;
	//TR << endl;

	/*****************************************************/
	/***************** Is it light chain? ****************/
	/*****************************************************/
	/* L1 search Start */
	if(querychain_first.find("WYL") != std::string::npos) {
		posl1_e = querychain_first.find("WYL") - 1;
	} else if(querychain_first.find("WLQ") != std::string::npos) {
		posl1_e = querychain_first.find("WLQ") - 1;
	} else if(querychain_first.find("WFQ") != std::string::npos) {
		posl1_e = querychain_first.find("WFQ") - 1;
	} else if(querychain_first.find("WYQ") != std::string::npos) {
		posl1_e = querychain_first.find("WYQ") - 1;
	} else if(querychain_first.find("WYH") != std::string::npos) {
		posl1_e = querychain_first.find("WYH") - 1;
	} else if(querychain_first.find("WVQ") != std::string::npos) {
		posl1_e = querychain_first.find("WVQ") - 1;
	} else if(querychain_first.find("WVR") != std::string::npos) {
		posl1_e = querychain_first.find("WVR") - 1;
	} else if(querychain_first.find("WWQ") != std::string::npos) {
		posl1_e = querychain_first.find("WWQ") - 1;
	} else if(querychain_first.find("WVK") != std::string::npos) {
		posl1_e = querychain_first.find("WVK") - 1;
	} else if(querychain_first.find("WLL") != std::string::npos) {
		posl1_e = querychain_first.find("WLL") - 1;
	} else if(querychain_first.find("WFL") != std::string::npos) {
		posl1_e = querychain_first.find("WFL") - 1;
	} else if(querychain_first.find("WVF") != std::string::npos) {
		posl1_e = querychain_first.find("WVF") - 1;
	} else if(querychain_first.find("WIQ") != std::string::npos) {
		posl1_e = querychain_first.find("WIQ") - 1;
	} else if(querychain_first.find("WYR") != std::string::npos) {
		posl1_e = querychain_first.find("WYR") - 1;
	} else if(querychain_first.find("WNQ") != std::string::npos) {
		posl1_e = querychain_first.find("WNQ") - 1;
	} else if(querychain_first.find("WHL") != std::string::npos) {
		posl1_e = querychain_first.find("WHL") - 1;
	} else if(querychain_first.find("WYM") != std::string::npos) {	// Add 06/10/2012
		posl1_e = querychain_first.find("WYM") - 1;
	} else {
		l1found = 1;
	}

	if(l1found != 1) {
		posl1_s   = querychain_first.find("C") + 1;
		lenl1     = posl1_e - posl1_s + 1;
		seql1     = querychain_first.substr(posl1_s,lenl1);

		pos_fr1_s = 0;
		pos_fr1_e = posl1_s - 1;
		len_fr1   = pos_fr1_e - pos_fr1_s + 1;
		seq_fr1   = querychain_first.substr(pos_fr1_s,len_fr1);
	}
	/* L1 search Finish */

	/* L2 search start */
	if(l1found != 1) {
		posl2_s   = posl1_e + 16;
		posl2_e   = posl2_s + 6;
		lenl2     = posl2_e - posl2_s + 1;
		seql2     = querychain.substr(posl2_s,lenl2);

		pos_fr2_s = posl1_e + 1;
		pos_fr2_e = posl2_s - 1;
		len_fr2   = pos_fr2_e - pos_fr2_s + 1;
		seq_fr2   = querychain.substr(pos_fr2_s,len_fr2);
	} else {
		l2found = 1;
	}
	/* L2 search end */

	/* L3 search Start */
	//string p1_l3[] = {"F","V","I"};
	//string p2_l3[] = {"G","A","E","V"};
	//string p3_l3[] = {"G","A","P","C","D","E","Q","N","R","K","H","W","Y","F","M","T","V","I","S","L"};
	//string p4_l3[] = {"G","Y"};
	for(l = 0; l < 3; ++l) {
		for(m = 0; m < 4; ++m) {
			for(n = 0; n < 2; ++n) {
				for(k = 0; k < 20; ++k) {
					//frl3 = "FG" + p3_l3[k] + "G";
					//frl3 = p1_l3[l] + "G" + p3_l3[k] + "G";			//[VF]GXG
					frl3 = p1_l3[l] + p2_l3[m] + p3_l3[k] + p4_l3[n];	//[VF][AG]XG

					if(querychain3.find(frl3, 80) != std::string::npos) {
						posl3_e = querychain3.find(frl3,80) - 1;
						posl3_s = querychain3.find("C",80) + 1;
						lenl3   = posl3_e - posl3_s + 1;

						//TR << frl3 << "\t" << posl3_s << "\t" << lenl3 << endl;

						seql3   = querychain3.substr(posl3_s,lenl3);

						if(seql3.length() > 4) {
							l3found = 0;
							break;
						} else {
							l3found = 1;
						}

						pos_fr3_s = posl2_e + 1;
						pos_fr3_e = posl3_s - 1;
						pos_fr4_s = posl3_e + 1;
						pos_fr4_e = pos_fr4_s + 5;
						len_fr3   = pos_fr3_e - pos_fr3_s + 1;
						len_fr4   = pos_fr4_e - pos_fr4_s + 1;
						seq_fr3   = querychain3.substr(pos_fr3_s,len_fr3);
						seq_fr4   = querychain3.substr(pos_fr4_s,len_fr4);
					}
				}

				l = 3;
				m = 4;
				n = 2;
				k = 20;
			}
		}
	}
	/* L3 search Finish */

	/*****************************************************/
	/***************** Is it heavy chain? ****************/
	/*****************************************************/
	if(l1found == 1 || l2found == 1 || l3found == 1) {
		/* H1 search Start */
		//string p1_h1[] = {"W","L"};
		//string p2_h1[] = {"I","V","F","Y","A","M","L","N","G","E"};
		//string p3_h1[] = {"R","K","Q","V","N","C"};
		//string p4_h1[] = {"Q","K","H","E","L","R"};
		for(n = 0; n < 2; ++n) {
			for(l = 0; l < 6; ++l) {
				for(m = 0; m < 6; ++m) {
					for(k = 0; k < 10; ++k) {
						//frh1 = "W" + p2_h1[k] + p3_h1[l] + p4_h1[m];
						frh1 = p1_h1[n] + p2_h1[k] + p3_h1[l] + p4_h1[m];

						if(querychain_first.find(frh1, 0) != std::string::npos) {
							posh1_e = querychain_first.find(frh1, 0) - 1;
							h1found = 0;
							n = 2;
							l = 6;
							m = 6;
							k = 10;
						}
					}
				}
			}
		}

		if(h1found != 1) {
			posh1_s  = querychain_first.find("C") + 4;
			lenh1    = posh1_e - posh1_s + 1;
			seqh1    = querychain_first.substr(posh1_s, lenh1);

			pos_fr1_s = 0;
			pos_fr1_e = posh1_s - 1;
			len_fr1   = pos_fr1_e - pos_fr1_s + 1;
			seq_fr1   = querychain_first.substr(pos_fr1_s,len_fr1);
		}
		/* H1 search Finish */

		/* H3 search Start */
		//string p1_h3[] = {"W","V"};
		//string p2_h3[] = {"G","A","C"};
		//string p3_h3[] = {"A","P","C","D","E","Q","N","R","K","H","W","Y","F","M","T","V","I","S","L","G"};
		//string p4_h3[] = {"G","R","D","Q","V"};
		for(m = 0; m < 3; ++m) {
			for(l = 0; l < 5; ++l) {
				for(k = 0; k < 20; ++k) {
					//frh3 = "WG" + p3_h3[k] + p4_h3[l];
					frh3 = "W" + p2_h3[m] + p3_h3[k] + p4_h3[l];

					if(querychain2.find(frh3, 80) != std::string::npos) {
						posh3_e = querychain2.find(frh3,80) - 1;
						h3found = 0;
						m = 3;
						l = 5;
						k = 20;
					}
				}
			}
		}

		if(querychain2.find("C", 80) != std::string::npos) {
			posh3_s = querychain2.find("C", 80) + 3;
		} else {
			h3found = 1;
		}

		if(h3found != 1) {
			lenh3 = posh3_e - posh3_s + 1;
			seqh3 = querychain2.substr(posh3_s,lenh3);
		}
		/* H3 search Finish */

		/* H2 search start */
		if(h1found != 1 && h3found != 1) {
			posh2_s = posh1_e + 15;
			posh2_e = posh3_s - 33;
			lenh2   = posh2_e - posh2_s + 1;
			seqh2   = querychain.substr(posh2_s,lenh2);

			pos_fr2_s = posh1_e + 1;
			pos_fr2_e = posh2_s - 1;
			pos_fr3_s = posh2_e + 1;
			pos_fr3_e = posh3_s - 1;
			pos_fr4_s = posh3_e + 1;
			pos_fr4_e = pos_fr4_s + 5;
			len_fr2   = pos_fr2_e - pos_fr2_s + 1;
			len_fr3   = pos_fr3_e - pos_fr3_s + 1;
			len_fr4   = pos_fr4_e - pos_fr4_s + 1;
			seq_fr2   = querychain.substr(pos_fr2_s,len_fr2);
			seq_fr3   = querychain.substr(pos_fr3_s,len_fr3);
			seq_fr4   = querychain.substr(pos_fr4_s,len_fr4);
		}
		/* H2 search end */
	}

	if(l1found == 0 && l2found == 0 && l3found == 0) {
		TR << lenl1 << "\t" << lenl2 << "\t" << lenl3 << "\t";
		TR << seql1 << "\t" << seql2 << "\t" << seql3 << "\t" << check << "\tLIGHT" << std::endl;
	} else if(h1found == 0 && h2found == 0 && h3found == 0) {
		TR << lenh1 << "\t" << lenh2 << "\t" << lenh3 << "\t";
		TR << seqh1 << "\t" << seqh2 << "\t" << seqh3 << "\t" << check << "\tHEAVY" << std::endl;
	} else if(l1found == 0 && l2found == 0 && l3found == 0 && h1found == 0 && h2found == 0 && h3found == 0) {
		TR << lenl1 << "\t" << lenl2 << "\t" << lenl3 << "\t";
		TR << lenh1 << "\t" << lenh2 << "\t" << lenh3 << "\t";
		TR << seql1 << "\t" << seql2 << "\t" << seql3 << "\t" << check << "\tLIGHT" << "\t";
		TR << seqh1 << "\t" << seqh2 << "\t" << seqh3 << "\t" << check << "\tHEAVY" << std::endl;
	} else {
		TR << "Some CDRs seem to be missing!\t" << querychain << "\t";
		TR << lenh1 << "\t" << lenh2 << "\t" << lenh3 << "\t";
		TR << "H1:" << seqh1 << "\tH2: " << seqh2 << "\tH3 " << seqh3 << "\t" << posh3_s << std::endl;
	}

	// return 0;
}




std::string
AntibodyInfo::get_CDR_sequence_with_stem(CDRNameEnum const & cdr_name,
        Size left_stem ,
        Size right_stem ) const {
	vector1<char> sequence;
	loops::Loop the_loop = get_CDR_loop(cdr_name);

	for (Size i=the_loop.start()-left_stem; i<=the_loop.stop()+right_stem; ++i) {
		std::map< Size, char >::const_iterator iter(sequence_map_.find(i)); //To keep const
		sequence.push_back(iter->second);
	}
	std::string seq(sequence.begin(), sequence.end());
	return seq ;
}

std::string
AntibodyInfo::get_CDR_sequence_with_stem(CDRNameEnum const & cdr_name, core::pose::Pose const & pose, CDRDefinitionEnum const & transform, Size left_stem , Size right_stem) const {

	vector1<char> sequence;
	loops::Loop the_loop = get_CDR_loop(cdr_name, pose, transform);

	for (Size i=the_loop.start()-left_stem; i<=the_loop.stop()+right_stem; ++i) {
		std::map< Size, char >::const_iterator iter(sequence_map_.find(i)); //To keep const
		sequence.push_back(iter->second);
	}
	std::string seq(sequence.begin(), sequence.end());
	return seq ;
}

std::string
AntibodyInfo::get_antibody_sequence() const {
	std::string seq(ab_sequence_.begin(), ab_sequence_.end());
	return seq;
}

scoring::ScoreFunctionCOP
get_Pack_ScoreFxn(void) {
	static scoring::ScoreFunctionOP pack_scorefxn = 0;
	if(pack_scorefxn == 0) {
		pack_scorefxn = core::scoring::getScoreFunctionLegacy( core::scoring::PRE_TALARIS_2013_STANDARD_WTS );
	}
	return pack_scorefxn;
}
scoring::ScoreFunctionCOP
get_Dock_ScoreFxn(void) {
	static scoring::ScoreFunctionOP dock_scorefxn = 0;
	if(dock_scorefxn == 0) {
		dock_scorefxn = core::scoring::ScoreFunctionFactory::create_score_function( "docking", "docking_min" );
		 dock_scorefxn->set_weight( core::scoring::chainbreak, 1.0 );
		dock_scorefxn->set_weight( core::scoring::overlap_chainbreak, 10./3. );
	}
	return dock_scorefxn;
}
scoring::ScoreFunctionCOP
get_LoopCentral_ScoreFxn(void) {
	static scoring::ScoreFunctionOP loopcentral_scorefxn = 0;
	if(loopcentral_scorefxn == 0) {
		loopcentral_scorefxn = core::scoring::ScoreFunctionFactory::create_score_function( "cen_std", "score4L" );
		loopcentral_scorefxn->set_weight( scoring::chainbreak, 10./3. );
	}
	return loopcentral_scorefxn;
}
scoring::ScoreFunctionCOP
get_LoopHighRes_ScoreFxn(void) {
	static scoring::ScoreFunctionOP loophighres_scorefxn = 0;
	if(loophighres_scorefxn == 0) {
		loophighres_scorefxn = scoring::getScoreFunction();
		loophighres_scorefxn->set_weight( scoring::chainbreak, 1.0 );
		loophighres_scorefxn->set_weight( scoring::overlap_chainbreak, 10./3. );
		loophighres_scorefxn->set_weight(scoring::dihedral_constraint, 1.0);
	}
	return loophighres_scorefxn;
}
	
AntibodyEnumManagerOP
AntibodyInfo::get_antibody_enum_manager() const {
	return enum_manager_;
}

CDRClusterEnumManagerOP
AntibodyInfo::get_cdr_cluster_enum_manager() const {
	return cdr_cluster_manager_;
}


/// @details  Show the complete setup of the docking protocol
void AntibodyInfo::show( std::ostream & out ) {
	out << *this;
}

std::ostream & operator<<(std::ostream& out, const AntibodyInfo & ab_info )  {

	using namespace ObjexxFCL::format;
	std::string line_marker = "///";
	out << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
	out << line_marker << A( 47, "Rosetta Antibody Info" ) << space( 27 ) << line_marker << std::endl;
	out << line_marker << space( 74 ) << line_marker << std::endl;

	out << line_marker << "             Antibody Type:";
	if(ab_info.is_camelid()) {
		out << "  Camelid Antibody"<< std::endl;
	} else                    {
		out << "  Regular Antibody"<< std::endl;
	}

	out << line_marker << " Predict H3 Cterminus Base:";
	out <<"  "<<ab_info.enum_manager_->h3_base_type_enum_to_string(ab_info.predicted_H3_base_type_)<<std::endl;

	out << line_marker << space( 74 ) << std::endl;
	for (CDRNameEnum i=start_cdr_loop; i<=ab_info.total_cdr_loops_; i=CDRNameEnum(i+1) ) {
		out << line_marker << " "+ab_info.get_CDR_name(i)+" info: "<<std::endl;
		out << line_marker << "           length:  "<< ab_info.get_CDR_loop(i).length() <<std::endl;
		out << line_marker << "         sequence:  ";
		//out<<  line_marker << "  north_cluster: "<< ab_info.cdr_cluster_manager_->cdr_cluster_enum_to_string(ab_info.get_CDR_cluster(i)->cluster()) <<std::endl;
		out << ab_info.get_CDR_sequence_with_stem(i,0,0) ;
		out <<std::endl;
		out << line_marker << "        loop_info:  "<< ab_info.get_CDR_loop(i)<<std::endl;
	}
	out << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
	return out;
}



} // namespace antibody
} // namespace protocols
