// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/fragments/BBTorsionandAnglesSRFD.hh
/// @brief  SRFD that remembers torsion angles as well as arbitrary bond angles
/// SO LONG AS THOSE ANGLES ARE ONLY THE N-CA-C BOND ANGLE.  CANNOT REPRESENT
/// BOND ANGLES TO PREVIOUS OR NEXT RESIDUES.
/// @author Florian Richter (floric@u.washington.edu)
/// @date   all time machine buttons are stuck
///
#ifndef core_fragments_BBTorsionAndAnglesSRFD_HH
#define core_fragments_BBTorsionAndAnglesSRFD_HH

// Unit Headers
#include <core/fragment/BBTorsionSRFD.hh>

// Package Headers
//#include <core/fragment/Frame.fwd.hh>
// AUTO-REMOVED #include <core/id/AtomID.hh>

// Project Headers
//#include <core/pose/Pose.hh>
//#include <core/kinematics/MoveMap.fwd.hh>
//#include <core/types.hh>

//#include <core/conformation/Residue.hh> // for ResidueSRFD

//#include <core/kinematics/types.hh>
//#include <core/id/TorsionID.hh>

// ObjexxFCL Headers

// Utility headers
#include <utility/vector1.hh>
//#include <utility/pointer/ReferenceCount.hh>

// C/C++ headers
#include <vector>
// AUTO-REMOVED #include <utility>

namespace core {
namespace fragment {

//
class BBTorsionAndAnglesSRFD : public BBTorsionSRFD {
	typedef BBTorsionSRFD Parent;
    typedef std::pair<std::vector<Size>, Real> AngleInfo;
public:
	/// @brief default constructor
	BBTorsionAndAnglesSRFD() {}

	BBTorsionAndAnglesSRFD( utility::vector1< AngleInfo > & angles_in, Size const nbb_in = 3, char secstruct = 'X', char sequence = 'X')
		: BBTorsionSRFD(nbb_in, secstruct, sequence), angles_(angles_in)
	{};

	//
	SingleResidueFragDataOP clone() const {
		return new BBTorsionAndAnglesSRFD( *this );
	};

	/// @brief create a new instance of this object
	virtual
	SingleResidueFragDataOP create() const {
		return new BBTorsionAndAnglesSRFD();
	}

	virtual bool apply( pose::Pose&, Size seq_pos ) const;

	/// @brief insert backbone torsions and angles into pose at position seqpos
	///  if all bb torsions are moveable in MoveMap
	/// @return True if *all* torsions and angles are inserted and superclass apply()
	///  is successful, otherwise False.
	/// @remarks This is currently all or nothing -- all torsions for seqpos
	///  must be moveable because it's not entirely clear what the behavior
	///  of partial angle insertion is.  In addition, DOF_IDs are not made
	///  explicitly available within this class, meaning there is no way to
	///  look them up within the MoveMap; the implementation in this class
	///  must be changed if this is desired.
	virtual bool apply( kinematics::MoveMap const & movemap, pose::Pose & pose, Size const seqpos ) const;

	virtual bool steal( pose::Pose const&, Size seq_pos );
	virtual bool is_compatible( SingleResidueFragData const& ) const;
	virtual bool is_applicable( kinematics::MoveMap const&, Size seq_pos ) const;

	virtual
	void show( std::ostream &out ) const;

	virtual
	void read( std::istream &in );

	virtual
	std::string type() const {
		//		return "BBTorsion";
		return _static_type_name();
	}

	static std::string _static_type_name() {
		return "BBTorsionAndAngles";
	}

	core::Size
	nangles() const{
		return angles_.size(); }

private:
	utility::vector1< AngleInfo > angles_;

};

} //fragment
} //core

#endif
