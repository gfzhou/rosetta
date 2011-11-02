// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/fragments/ConstantLengthFragSet.hh
/// @brief  yields a simple implementation of a fragset
/// @author Oliver Lange ( olange@u.washington.edu)
/// @date   Wed Aug 22 12:08:31 2007
///

#ifndef core_fragments_ConstantLengthFragSet_HH
#define core_fragments_ConstantLengthFragSet_HH

// Unit Headers
#include <core/fragment/ConstantLengthFragSet.fwd.hh>

// Package Headers
#include <core/fragment/FragSet.hh>
#include <core/fragment/Frame.hh>
#include <core/fragment/FrameList.hh>

// Project Headers
// AUTO-REMOVED #include <utility/io/izstream.hh>
#include <core/kinematics/MoveMap.fwd.hh>
#include <core/types.hh>

// ObjexxFCL Headers

// Utility headers
// AUTO-REMOVED #include <utility/vector1.hh>
#include <utility/pointer/ReferenceCount.hh>

#include <utility/vector1.hh>
#include <utility/io/izstream.fwd.hh>


// Package Headers
// AUTO-REMOVED #include <core/fragment/FrameIterator.hh>

// std Headers
// AUTO-REMOVED #include <iterator>

/* Just a mad thought: with fragments becoming ever more "Residue" like one might want to use the
	 packer to choose a combination of good fragments instead of makeing independent choices.
	 I guess, it is only a question of keeping the combinatorics in control...
	 maybe it makes sense to pack with only "unconfident" regions of the backbone flexible ..
*/

namespace core {
namespace fragment {

/* classic 9mer Frags would be in one of those */
/// @brief A set of fragments that contains fragments of a constant length read
/// in from a fragment file.
/// @note this object is a simple implementation of the FragSet
/// @note for custom fragments, check out https://robetta.bakerlab.org/fragmentsubmit.jsp
///
/// example:
///     ninemers = ConstantLengthFragSet(9)
///     ninemers.read_fragment_file("test9_fragments")
/// See also:
///     Pose
///     ClassicFragmentMover
class ConstantLengthFragSet : public FragSet {
	typedef FragSet Parent;
// ConstantLengthFragSet is a FragSet with only one frame per position!
public:

	ConstantLengthFragSet( Size frag_length ) {
		set_max_frag_length( frag_length );
	};

	ConstantLengthFragSet( Size frag_length, std::string filename );

	ConstantLengthFragSet() {};

	~ConstantLengthFragSet();

	ConstantLengthFragSet( FragSet const& fragments );

	virtual FragSetOP clone() const
	{
		return new ConstantLengthFragSet( *this );
	}

	virtual FragSetOP empty_clone() const
	{
		return new ConstantLengthFragSet();
	}

	/// @brief Loads the contents of  <filename>  into the ConstantLengthFragSet
	///
	/// example:
	///     ninemers.read_fragment_file("test9_fragments")
	/// See also:
	///     ConstantLengthFragSet
	///     Pose
	///     Pose.omega
	///     Pose.phi
	///     Pose.psi
	///     ClassicFragmentMover
	void read_fragment_file( std::string filename, Size top25 = 0, Size ncopies = 1, bool bAnnotation = false  );

	void read_fragment_stream( utility::io::izstream & data, Size top25 = 0, Size ncopies = 1, bool bAnnotation = false  );

	// void print_fragments();
	///@brief there is only one Frame per position, end / max_overlap are ignored
	Size region(
		kinematics::MoveMap const& mm,
		core::Size start,
		core::Size end,
		core::Size , //min_overlap not used
		core::Size , //min_length not used
		FrameList &frames
	) const;

// 	/// @brief Accessor for the Frames at the specified insertion position. Returns 0=false if
// 	/// there is no frame at the specified position.
// 	virtual Size frames(
// 		Size pos,
// 		FrameList & frame
// 	);

	FrameIterator begin() const;

	FrameIterator end() const;

	virtual bool empty() const {
		return frames_.size()==0;
	}
protected:
	virtual void add_( FrameOP aframe );

private:
	FrameList frames_;
};

}
}

#endif
