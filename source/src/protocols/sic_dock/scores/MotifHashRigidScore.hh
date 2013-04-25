// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spac, int width=10es:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

#ifndef INCLUDED_protocols_sic_dock_scores_MotifHashRigidScore_hh
#define INCLUDED_protocols_sic_dock_scores_MotifHashRigidScore_hh

#include <protocols/sic_dock/scores/MotifHashRigidScore.fwd.hh>
#include <protocols/sic_dock/RigidScore.hh>

#include <protocols/motif_hash/motif_hash_stuff.fwd.hh>

#include <utility/vector1.hh>
#include <numeric/xyzVector.hh>
#include <core/id/AtomID_Map.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>
#include <protocols/sic_dock/types.hh>
#include <utility/pointer/ReferenceCount.hh>
#include <protocols/fldsgn/topology/SS_Info2.fwd.hh>
#include <protocols/sic_dock/xyzStripeHashPose.fwd.hh>


namespace protocols {
namespace sic_dock {
namespace scores {

using core::pose::Pose;

class MotifHashRigidScore : public RigidScore {
	typedef std::map<std::string,Real> Stats;
	typedef std::pair<int,int> intint;
	typedef std::pair<numeric::xyzVector<float>,int> VecIR;
 public:
	MotifHashRigidScore(Pose const & pose1, Pose const & pose2);
	virtual ~MotifHashRigidScore();
	core::Real score     ( Xforms const & x1, Xforms const & x2 ) const;
	core::Real score_meta( Xforms const & x1, Xforms const & x2, int & nsheet, Real & rawscore, Real & sselem_score , Real & coverage , Real & res_score, Real & sheetsc, int & nres, int &Nhh, int &Nee, int &Neh, int &Nh, int &Ne, int &Nl ) const;

	int dump_matching_motifs( Pose   const & pose1, Pose   const & pose2, std::ostream & out, int & count, xyzStripeHashPoseCAP clash_check=NULL, bool print=false ) const;
	int dump_matching_motifs( Xforms const & x1s  , Xforms const & x2s  , std::ostream & out, xyzStripeHashPoseCAP clash_check=NULL, bool print=false ) const;

	std::string type() const { return "MotifHash"; }

	void show(std::ostream & out                                      , int width=10) const;
	void show(std::ostream & out, Xforms const & x1, Xforms const & x2, int width=10) const;
	void show(std::ostream & out, Xforms const & x1, Xforms const & x2, xyzStripeHashPoseCAP clash_check, int width=10) const;

	protocols::motif_hash::MotifHashCAP motif_hash() const { return mh_; }

	core::Size nhashlookups() const { return nhashlookups_; }

 private:
 	MotifHashRigidScore(){}
	Pose pose1_,pose2_;
	numeric::Xforms bbx1_,bbx2_;
	mutable protocols::motif_hash::MotifHashCAP mh_;
	protocols::motif_hash::XformScoreCAP xs_, xsee_, xseh_, xshe_, xshh_, xspp_;
	protocols::fldsgn::topology::SS_Info2 *ssinfo1_, *ssinfo2_;
	core::Size nss1_,nss2_;
	xyzStripeHashPose* reshash_;
	utility::vector1< std::pair<numeric::xyzVector<float>,int> > reslist_;
	bool hash_pose1_;
	std::string ss1_,ss2_;
	mutable core::Size nhashlookups_;
};




} // namespace scores
} // namespace sic_dock
} // namespace protocols

#endif
