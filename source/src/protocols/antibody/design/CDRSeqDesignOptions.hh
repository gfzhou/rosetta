// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available
// (c) under license. The Rosetta software is developed by the contributing
// (c) members of the Rosetta Commons. For more information, see
// (c) http://www.rosettacommons.org. Questions about this can be addressed to
// (c) University of Washington UW TechTransfer,email:license@u.washington.edu.

/// @file protocols/antibody/design/CDRSeqDesignOptions.hh
/// @brief Option control for Sequence Design of a single CDR
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

#ifndef INCLUDED_protocols_antibody_design_AntibodySeqDesignOptions_hh
#define INCLUDED_protocols_antibody_design_AntibodySeqDesignOptions_hh

#include <protocols/antibody/design/CDRSeqDesignOptions.fwd.hh>
#include <protocols/antibody/AntibodyEnumManager.fwd.hh>
#include <protocols/antibody/AntibodyEnum.hh>
#include <protocols/antibody/design/AntibodyDesignEnum.hh>

// Utility headers
#include <utility/pointer/owning_ptr.hh>
#include <utility/pointer/ReferenceCount.hh>

#include <utility/vector1.hh>

namespace protocols {
namespace antibody {
namespace design {

	typedef utility::vector1<CDRSeqDesignOptionsOP> AntibodyCDRSeqDesignOptions;

class CDRSeqDesignOptions : public utility::pointer::ReferenceCount {
public:

	CDRSeqDesignOptions();
	CDRSeqDesignOptions(CDRNameEnum cdr);
	CDRSeqDesignOptions(CDRSeqDesignOptions const & src);

	virtual ~CDRSeqDesignOptions();

public:
	void
	set_cdr(CDRNameEnum cdr);
	
	bool
	cdr() const {
		return cdr_;
	}
	
	void
	design(bool design);
	
	bool
	design() const {
		return design_;
	}
	
	void
	design_strategy(SeqDesignStrategyEnum strategy);
	
	SeqDesignStrategyEnum
	design_strategy() const {
		return design_strategy_;
	}
	
public:
	CDRSeqDesignOptionsOP
	clone() const;
	
	void
	set_defaults();
	
private:
	CDRNameEnum cdr_;
	bool design_;
	
	SeqDesignStrategyEnum design_strategy_;
};




class CDRSeqDesignOptionsParser : public utility::pointer::ReferenceCount {
public:

	CDRSeqDesignOptionsParser();
	CDRSeqDesignOptionsParser(CDRSeqDesignOptions const & src);

	virtual ~CDRSeqDesignOptionsParser();

	CDRSeqDesignOptionsOP
	parse_options(CDRNameEnum cdr, std::string filename);
	
	///@brief Parse default_instructions (mainly used for AbDesign) then parse user file
	CDRSeqDesignOptionsOP
	parse_default_and_user_options(CDRNameEnum cdr, std::string filename);
	
	
	
	///ALL CDRs
	
	utility::vector1<CDRSeqDesignOptionsOP>
	parse_options(std::string filename);
	
	utility::vector1<CDRSeqDesignOptionsOP>
	parse_default_and_user_options(std::string filename);
	
private:
	void
	check_path();
	
	void
	check_line_len(const utility::vector1<std::string> & lineSP, const core::Size len_check) const;
	
	void
	parse_cdr_option(std::string const mode, vector1<std::string> & lineSP);
	
	void
	parse_cdr_design_option(std::string const adjective, vector1< std::string> & lineSP) ;
	
	void
	parse_cdr_general_option(utility::vector1<std::string> & lineSP) ;
	
	void
	set_cdr_design_profile_option(std::string const option);
	
private:
	std::string instructions_path_;
	AntibodyEnumManagerOP ab_manager_;
	CDRSeqDesignOptionsOP cdr_options_;
	bool default_and_user_;
	
};



}
}
}


#endif	//INCLUDED_ CDRSeqDesignOptions.hh





