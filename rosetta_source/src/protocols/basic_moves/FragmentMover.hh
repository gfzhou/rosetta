// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/fragments/FragSet.hh
/// @brief  set of fragments
/// @author Oliver Lange
/// @date   Wed Aug 22 12:08:31 2007
///

#ifndef protocols_basic_moves_FragmentMover_HH
#define protocols_basic_moves_FragmentMover_HH

// Unit Headers
#include <protocols/basic_moves/FragmentMover.fwd.hh>

// Package Headers
// AUTO-REMOVED #include <core/fragment/FragData.fwd.hh>
#include <core/fragment/FragSet.fwd.hh>
#include <core/fragment/Frame.fwd.hh>
#include <core/fragment/FrameList.fwd.hh>
#include <core/fragment/ConstantLengthFragSet.fwd.hh>

// Project Headers
#include <core/kinematics/MoveMap.fwd.hh>
#include <core/pose/Pose.fwd.hh>
#include <protocols/moves/Mover.hh>


// ObjexxFCL Headers

// Utility headers
#include <utility/vector1.fwd.hh>

#include <utility/vector1.hh>


namespace protocols {
namespace basic_moves {
// might want to live in protocols::moves

///@brief abstract base class for FragmentMovers
class FragmentMover : public moves::Mover {
public:
	~FragmentMover();

	///@brief choose and insert a fragment --> has to be overloaded
	virtual void apply( core::pose::Pose& ) = 0;


	///@broef apply a fragment at pos to movemable dofs
	virtual bool apply( core::pose::Pose&, Size pos ) const; // apply fragment at seqpos ( if possible )
	virtual std::string get_name() const;


	///@brief apply at all movemable positions --- honors movemap
	virtual Size apply_at_all_positions( core::pose::Pose& ) const; //apply one fragment at each insertable position

	///@brief accessor to the fragment set
	core::fragment::FragSetCOP fragments() const;

	///@brief setter for the fragment set
	virtual void set_fragments( core::fragment::FragSetCOP new_frags_ );

	///@brief setter for the movemap
	virtual void set_movemap( core::kinematics::MoveMapCOP movemap );

	core::kinematics::MoveMapCOP movemap() const;

	core::fragment::InsertMap const& insert_map() const {
		return insert_map_;
	}

	core::fragment::InsertSize const& insert_size() const {
		return insert_size_;
	}

protected:
	FragmentMover(
		core::fragment::FragSetCOP fragset,
		std::string type
	);


	///@brief constructor
	FragmentMover(
		core::fragment::FragSetCOP fragset,
		core::kinematics::MoveMapCOP movemap,
		std::string type
	);

 	virtual void on_new_fragments() {
		update_insert_map();
		//invalid_insert_map();
	}

// 	void invalid_insert_map() {
// 		bValidInsertMap_ = false;
// 		insert_map_.clear();
// 		insert_size_.clear();
// 	}

	void update_insert_map();

	virtual bool apply_frames( core::pose::Pose &, core::fragment::FrameList const& ) const = 0;

	core::fragment::FragSetCOP fragset_;
	core::kinematics::MoveMapCOP movemap_;

	//	bool bValidInsertMap_;
	core::fragment::InsertMap insert_map_;
	core::fragment::InsertSize insert_size_;

};

/// @brief A FragmentMover that applies uniform sampling of fragments
class ClassicFragmentMover : public FragmentMover {
public:

	/// @brief
	ClassicFragmentMover(
		core::fragment::FragSetCOP fragset
	);


	/// @brief
	ClassicFragmentMover(
		core::fragment::FragSetCOP fragset,
		core::kinematics::MoveMapCOP movemap
	);


	/// @brief
	/// @note: temp work around for PyRosetta code, until we find a way to handle owning pointers in this case
	ClassicFragmentMover(
		core::fragment::ConstantLengthFragSet const & fragset,
		core::kinematics::MoveMap const & movemap
	);

	~ClassicFragmentMover();

	/// @brief Applies classic fragment insertion based on a FragSet
	///
	/// example:
	///     mover_3mer.apply(pose)
	/// See also:
	///     ClassicFragmentMover
	///     Pose
	///     ConstantLengthFragSet
	void apply( core::pose::Pose & );
	virtual std::string get_name() const;

	// Not defined, commenting out to make Python binding compile
	//bool apply( core::pose::Pose &, Size pos );

	///@brief check_ss controls whether fragment insertions are rejected that create short helices (<3) or strands (<2)
	void set_check_ss( bool setting ) {
		check_ss_=setting;
	}

	///@brief accessor
	bool
	check_ss() const {
		return check_ss_;
	}

	core::Real end_bias() const {
		return end_bias_;
	}

	void set_end_bias( core::Real setting ) {
		end_bias_ = setting;
	}
	bool end_bias_check_enabled() const {
		return bApplyEndBias_;
	}

	void enable_end_bias_check( bool setting = true ) {
		bApplyEndBias_ = setting;
	}

	///@brief min_overlap controls the behaviour  fragset->region()
	void set_min_overlap( Size setting ) {
		min_overlap_ = setting;
	}

	///@brief min_frag_length controls the behaviour  fragset->region()
	void set_min_frag_length( Size setting ) {
		min_frag_length_ = setting;
	}


protected:

	///@brief alternative Constructor to be used by derived classes
	ClassicFragmentMover(
		core::fragment::FragSetCOP fragset,
		core::kinematics::MoveMapCOP movemap,
		std::string type
	);

	///@brief alternative Constructor to be used by derived classes
	ClassicFragmentMover(
		core::fragment::FragSetCOP fragset,
		std::string type
	);

	virtual bool apply_frames( core::pose::Pose &, core::fragment::FrameList const& ) const;

public: //this is actually protected: but need public for some unit-testing
	/// @brief apply the chosen fragment,
	/// @detail this can be overloaded to change action, e.,g., WobbleMover introduces chain-break
	/// before fragment insertion and does ccd-loop closure afterwards
	virtual
	bool
	apply_fragment(
		core::fragment::Frame const& frame,
		Size frag_num,
		core::kinematics::MoveMap const& movemap,
		core::pose::Pose &pose
	) const;

	/// @brief set defaults
	virtual void set_defaults();

	// A Fragment mover has to make choices according which fragments are retrieved from the fragset_.
	// the following virtual functions may be overloaded to influence these choices.
	// generally they return bool if they were successfully in choosing a value
	// and return their choice(s) in the last by-value parameter(s) of the calls

	/// @brief given FrameList and pose, this method yields a fragment ( frame_num/ frag_num);
	/// return false if nothing suitable is found
	virtual
	bool choose_fragment(
		core::fragment::FrameList const&,
		core::pose::Pose const&,
		Size &frame_num,
		Size &frag_num
	) const;

	/// @brief apply fragment at predefined position
	void define_start_window( Size window_start );

	/// @brief yields a start position (window_start) for fragment window to sample from
	/// ( return false, if nothing suitable is found )
	virtual bool choose_window_start( core::pose::Pose const&, Size window_length, Size &window_start ) const;

	/// @brief yields a length (window_length) of the fragment window to sample from
	/// ( return false, if no suitable length is found )
	virtual bool choose_window_length( core::pose::Pose const&, Size &window_length) const {
		window_length = 1 ; //traditionally a constant value
		return true;
	}

	///@brief is called from set_fragments()
	void on_new_fragments() {
	}

	virtual bool end_bias_check( core::pose::Pose const&, Size window_start ) const;

	///@brief returns true if the ss string is acceptable
	virtual bool
	valid_ss( std::string const & new_ss ) const;



private:

	// these parameters seem to be more like constant options and therefore I didn't
	// wrap them in virtual function calls... but of course one may change that...
	Size min_overlap_;
	Size min_frag_length_;
	bool check_ss_;
	core::Real end_bias_;

	// @brief if true, fragments are less likely to be inserted at tails of pose
	// IN JUMPING MODE THIS SHOULD PROBABLY BE MADE RELATIVE TO DIST TO CHAINBREAK ?
	bool bApplyEndBias_;
	bool use_predefined_window_start_;
	Size predefined_window_start_;
};

class LoggedFragmentMover : public ClassicFragmentMover {
	typedef ClassicFragmentMover Parent;
	struct Item {
		Item( Size a, Size b) : frame_pos( a), frag_num( b) {}
		Size frame_pos;
		Size frag_num;
	};

public:
	~LoggedFragmentMover();

	LoggedFragmentMover(
  	core::fragment::FragSetCOP fragset,
		core::kinematics::MoveMapCOP movemap
	);

	virtual std::string get_name() const;

	virtual
	bool
	apply_fragment(
		core::fragment::Frame const& frame,
		Size frag_num,
		core::kinematics::MoveMap const& movemap,
		core::pose::Pose &pose
	) const;

	void show( std::ostream& ) const;
	void clear();
private:
	typedef utility::vector1< Item > Storage;
	mutable Storage logs_;
};

} //basic_moves
} //protocols
#endif
