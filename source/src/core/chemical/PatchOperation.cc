// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   src/core/chemical/PatchOperation.cc
/// @brief  Polymorphic classes representing the contents of a residue-type patch file
/// @author Phil Bradley

// Unit headers
#include <core/chemical/PatchOperation.hh>

// Package headers
#include <core/chemical/ResidueType.hh>

// Project headers
#include <core/chemical/Atom.hh>

// Numeric headers
#include <numeric/conversions.hh>

// Basic headers
#include <basic/Tracer.hh>

// Utility headers
#include <utility/vector1.hh>

// ObjexxFCL headers
#include <ObjexxFCL/string.functions.hh>


namespace core {
namespace chemical {

/// @details Auto-generated virtual destructor
PatchOperation::~PatchOperation() {}

static basic::Tracer tr("core.chemical");
static basic::Tracer TR_PatchOperations("core.chemical.PatchOperations.hh");

DeleteAtom::DeleteAtom( std::string const & atom_name_in ) :
	atom_name_( atom_name_in )
{}

bool
DeleteAtom::apply( ResidueType & rsd ) const
{

	if ( !rsd.has( atom_name_ )  ) {
		TR_PatchOperations.Debug << "DeleteAtom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ << std::endl;
		return true; // failure
	} else {
		//std::cout << "DeleteAtom::apply: deleting: " << atom_name_ << std::endl;
		rsd.delete_atom( atom_name_ );
	}
	return false;
}

SetBackboneHeavyatom::SetBackboneHeavyatom( std::string const & atom_name_in ) :
	atom_name_( atom_name_in )
{}

bool
SetBackboneHeavyatom::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "SetBackboneHeavyatom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ <<
			std::endl;
		return true; // failure
	} else {
		//std::cout << "SetBackboneHeavyatom::apply: " << atom_name_ << std::endl;
		rsd.set_backbone_heavyatom( atom_name_ );
	}
	return false;
}

/// @brief constructor the type of connection is "LOWER" or "UPPER"
SetPolymerConnectAtom::SetPolymerConnectAtom( std::string const & atom_name_in, std::string const & upper_lower_in ) :
	atom_name_( atom_name_in )
{
	if ( upper_lower_in == "LOWER" ) {
		upper_lower_ = -1;
	} else if ( upper_lower_in == "UPPER" ) {
		upper_lower_ = 1;
	} else {
		utility_exit_with_message( "SetPolymerConnectAtom: unrecognized switch "+upper_lower_in );
	}
}

bool
SetPolymerConnectAtom::apply( ResidueType & rsd ) const
{
	if ( atom_name_ == "NONE" || rsd.has( atom_name_ ) ) {
		//std::cout << "SetPolymerConnectAtom::apply: " << atom_name_ << ' ' << upper_lower_ << std::endl;
		if ( upper_lower_ == -1 ) {
			rsd.set_lower_connect_atom( atom_name_ );
		} else {
			assert( upper_lower_ == 1 );
			rsd.set_upper_connect_atom( atom_name_ );
		}
	} else {
		TR_PatchOperations.Debug << "SetPolymerConnectAtom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ <<
			std::endl;
		return true; // failure
	}
	return false;
}

AddConnect::AddConnect( std::string const & connect_atom,
	Real const phi, Real const theta, Real const d,
	std::string const & parent_atom,
	std::string const & angle_atom,
	std::string const & torsion_atom
):
	connect_atom_( connect_atom ),
	phi_( phi ), theta_( theta ), d_( d ),
	parent_atom_ (  parent_atom ),
	angle_atom_  (   angle_atom ),
	torsion_atom_( torsion_atom )
{}


bool
AddConnect::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( connect_atom_ ) ||
			 !rsd.has(  parent_atom_ ) ||
			 !rsd.has(   angle_atom_ ) ||
			 !rsd.has( torsion_atom_ ) ) return true; // failure!

	Size const connid( rsd.add_residue_connection( connect_atom_ ) );
	rsd.set_icoor( "CONN"+ObjexxFCL::string_of( connid ), phi_, theta_, d_, parent_atom_, angle_atom_, torsion_atom_ );
	return false;
}

AddProperty::AddProperty( std::string const & property_in ):
	property_( property_in )
{}

bool
AddProperty::apply( ResidueType & rsd ) const
{
	rsd.add_property( property_ );
	//TR_PatchOperations.Debug << "AddProperty::apply: " << property_ << std::endl;
	return false;
}

DeleteProperty::DeleteProperty( std::string const & property_in ):
	property_( property_in )
{}

/// add a property
bool
DeleteProperty::apply( ResidueType & rsd ) const
{
	rsd.delete_property( property_ );
	//std::cout << "DeleteProperty::apply: " << property_ << std::endl;
	return false;
}


// AddChi ////////////////////////////////////////////////////////////////////
// Constructor for when the chi index is specified.
AddChi::AddChi(Size const & chino_in,
		std::string const & atom1_in,
		std::string const & atom2_in,
		std::string const & atom3_in,
		std::string const & atom4_in) :
	no_index_(false),
	chino_( chino_in ),
	atom1_( atom1_in ),
	atom2_( atom2_in ),
	atom3_( atom3_in ),
	atom4_( atom4_in )
{}

// Constructor for when the chi index is not specified.
/// @details In this case, the new chi will be added to the end of the list when this operation is applied.
/// @author  Labonte
AddChi::AddChi(std::string const & atom1_in,
		std::string const & atom2_in,
		std::string const & atom3_in,
		std::string const & atom4_in) :
	no_index_(true),
	chino_(),
	atom1_(atom1_in),
	atom2_(atom2_in),
	atom3_(atom3_in),
	atom4_(atom4_in)
{}

bool
AddChi::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom1_ ) || !rsd.has( atom2_ ) || !rsd.has( atom3_ ) || !rsd.has( atom4_ ) ) {
		TR_PatchOperations.Debug << "AddChi::apply failed: " << rsd.name() << " is missing atom(s) " <<
				atom1_ << ' ' << rsd.has( atom1_ ) << ' ' << atom2_ << ' ' << rsd.has( atom2_ ) <<
				atom3_ << ' ' << rsd.has( atom3_ ) << ' ' << atom4_ << ' ' << rsd.has( atom4_ ) << std::endl;
		return true; // failure
	} else {
		if (no_index_) {
			rsd.add_chi(atom1_, atom2_, atom3_, atom4_);
		} else {
			rsd.add_chi(chino_, atom1_ , atom2_, atom3_, atom4_);
		}
		//TR_PatchOperations.Debug << "AddChi::apply: " <<
		//		atom1_ << ' ' << atom2_ << ' ' << atom3_ << ' ' << atom4_ << std::endl;
		return false;  // success
	}
}


// AddProtonChi //////////////////////////////////////////////////////////////
AddProtonChi::AddProtonChi(
	Size const & chino_in, utility::vector1<core::Real> const & samples, utility::vector1<core::Real> const & extrasamples
):
	chino_( chino_in ), samples_(samples), extrasamples_(extrasamples)
{}

bool
AddProtonChi::apply( ResidueType & rsd ) const
{
	rsd.set_proton_chi( chino_, samples_, extrasamples_ );
	return false;
}


RedefineChi::RedefineChi(Size const & chino_in,
		std::string const & atom1_in,
		std::string const & atom2_in,
		std::string const & atom3_in,
		std::string const & atom4_in):
	chino_( chino_in ), atom1_( atom1_in ), atom2_( atom2_in ), atom3_( atom3_in ), atom4_( atom4_in )
{}

bool
RedefineChi::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom1_ ) || !rsd.has( atom2_ ) || !rsd.has( atom3_ ) || !rsd.has( atom4_ ) )
	{
		TR_PatchOperations.Debug << "RedefineChi::apply failed: " <<
				rsd.name() << " is missing atom(s) " <<
				atom1_ << ' ' << rsd.has( atom1_ ) << ' ' << atom2_ << ' ' << rsd.has( atom2_ ) <<
				atom3_ << ' ' << rsd.has( atom3_ ) << ' ' << atom4_ << ' ' << rsd.has( atom4_ ) << std::endl;
		return true; // failure
	}
	else
	{
		rsd.redefine_chi( chino_, atom1_ , atom2_, atom3_, atom4_ );
		//TR_PatchOperations.Debug << "RedefineChi::apply: " << atom1_ << ' ' << atom2_
		//	<< ' ' << atom3_ << ' ' << atom4_ << std::endl;
		return false;
	}
	return false;
}


// AddChiRotamer /////////////////////////////////////////////////////////////
// Constructor for when the chi index is specified
AddChiRotamer::AddChiRotamer(Size const & chino_in, Angle const & mean_in, Angle const & sdev_in) :
	no_index_(false),
	chino_(chino_in),
	mean_(mean_in),
	sdev_(sdev_in)
{}

/// @details In this case, the rotamer sample will be added to the last chi when this operation is applied.
/// @author  Labonte
AddChiRotamer::AddChiRotamer(Angle const & mean_in, Angle const & sdev_in) :
	no_index_(true),
	chino_(),
	mean_(mean_in),
	sdev_(sdev_in)
{}

bool
AddChiRotamer::apply( ResidueType & rsd ) const
{
	if (no_index_) {
		rsd.add_chi_rotamer_to_last_chi(mean_ , sdev_);
	} else {
		rsd.add_chi_rotamer(chino_, mean_ , sdev_);
	}
	//TR_PatchOperations.Debug << "AddChiRotamer::apply: " << chino_ << ' ' << mean_
	//	<< ' ' << sdev_ << std::endl;
	return false;  // success
}


// AddAtom ///////////////////////////////////////////////////////////////////
AddAtom::AddAtom(
	std::string const & atom_name_in,
	std::string const & atom_type_name_in,
	std::string const & mm_atom_type_name_in,
	Real const charge
):
	atom_name_( atom_name_in ),
	atom_type_name_( atom_type_name_in ),
	mm_atom_type_name_( mm_atom_type_name_in ),
	charge_( charge )
{}

bool
AddAtom::apply( ResidueType & rsd ) const
{
	rsd.add_atom( atom_name_, atom_type_name_, mm_atom_type_name_, charge_ );
	//std::cout << "AddAtom::apply: " << atom_name_ << ' ' << atom_type_name_ << ' ' << mm_atom_type_name_ << ' ' <<
	//	charge_ << std::endl;
	return false;
}

AddBond::AddBond(
	std::string const & atom1_in,
	std::string const & atom2_in
):
	atom1_( atom1_in ),
	atom2_( atom2_in )
{}

bool
AddBond::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom1_ ) || !rsd.has( atom2_ ) ) {
		TR_PatchOperations.Debug << "AddBond::apply failed: " << rsd.name() << " is missing atom(s) " << atom1_ << ' ' <<
			rsd.has( atom1_ ) << ' ' << atom2_ << ' ' << rsd.has( atom2_ ) << std::endl;
		return true; // failure
	} else {
		//TR_PatchOperations.Debug << "AddBond::apply: " << atom1_ << ' ' << atom2_ << std::endl;
		rsd.add_bond( atom1_, atom2_ );
	}
	return false;
}

SetAtomicCharge::SetAtomicCharge(
	std::string const & atom_name_in,
	Real const charge_in
):
	atom_name_( atom_name_in ),
	charge_( charge_in )
{}

bool
SetAtomicCharge::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "SetAtomicCharge::apply failed: " << rsd.name() << " is missing atom: " << atom_name_ << std::endl;
		return true; // failure
	} else {
		//std::cout << "SetAtomicCharge::apply: " << atom_name_ << ' ' << charge_ << std::endl;
		rsd.atom( atom_name_ ).charge( charge_ );
	}
	return false;
}

SetAtomType::SetAtomType(
	std::string const & atom_name_in,
	std::string const & atom_type_name_in
):
	atom_name_( atom_name_in ),
	atom_type_name_( atom_type_name_in )
{}

bool
SetAtomType::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "SetAtomType::apply failed: " << rsd.name() << " is missing atom: " << atom_name_ << std::endl;
		return true; // failure
	} else {
		//std::cout << "SetAtomType::apply: " << atom_name_ << ' ' << atom_type_name_ << std::endl;
		rsd.set_atom_type( atom_name_, atom_type_name_ );
	}
	return false;
}

SetIO_String::SetIO_String(
	std::string const & name3,
	char const name1
):
	name3_( name3 ),
	name1_( name1 )
{}

bool
SetIO_String::apply( ResidueType & rsd ) const
{
	rsd.name3( name3_ );
	rsd.name1( name1_ );
	return false;
}

SetInterchangeabilityGroup_String::SetInterchangeabilityGroup_String(
	std::string const & intgrp
):
	intgrp_( intgrp )
{}

bool
SetInterchangeabilityGroup_String::apply( ResidueType & rsd ) const
{
	rsd.interchangeability_group( intgrp_ );
	return false;
}

AppendInterchangeabilityGroup_String::AppendInterchangeabilityGroup_String(
	std::string const & intgrp_addendum
):
	intgrp_addendum_( intgrp_addendum )
{}

bool
AppendInterchangeabilityGroup_String::apply( ResidueType & rsd ) const
{
	std::string new_intgrp = rsd.interchangeability_group() + intgrp_addendum_;
	rsd.interchangeability_group( new_intgrp );
	return false;
}


SetMMAtomType::SetMMAtomType(
	std::string const & atom_name_in,
	std::string const & mm_atom_type_name_in
):
	atom_name_( atom_name_in ),
	mm_atom_type_name_( mm_atom_type_name_in )
{}

bool
SetMMAtomType::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "SetAtomType::apply failed: " << rsd.name() << " is missing atom: " << atom_name_ << std::endl;
		return true; // failure
	} else {
		//std::cout << "SetAtomType::apply: " << atom_name_ << ' ' << mm_atom_type_name_ << std::endl;
		rsd.set_mm_atom_type( atom_name_, mm_atom_type_name_ );
	}
	return false;
}


/// helper function
std::string
expand_icoor_atom_name( std::string name, ResidueType const & rsd )
{
	std::string const nconn_tag( "%NCONN" );
	Size pos( name.find( nconn_tag ) );
	if ( pos < name.size() ) {
		//std::cout << "name before replace: " << name << std::endl;
		name.replace( pos, nconn_tag.size(), ObjexxFCL::string_of( rsd.n_residue_connections() ) );
		//std::cout << "name after replace: " << name << std::endl;
	}
	return name;

}

SetICoor::SetICoor(
	std::string const & atom_in,
	Real const phi_in,
	Real const theta_in,
	Real const d_in,
	std::string const & stub1_in,
	std::string const & stub2_in,
	std::string const & stub3_in
):
	atom_( atom_in ),
	phi_( phi_in ),
	theta_( theta_in ),
	d_( d_in ),
	stub1_( stub1_in ),
	stub2_( stub2_in ),
	stub3_( stub3_in )
{}

bool
SetICoor::apply( ResidueType & rsd ) const
{
	//std::cout << "SetICoor::apply: " << atom_ << ' ' << stub1_ << ' ' << stub2_ << ' ' << stub3_ <<
	//	std::endl;
	std::string const atom ( expand_icoor_atom_name( atom_ , rsd ) );
	std::string const stub1( expand_icoor_atom_name( stub1_, rsd ) );
	std::string const stub2( expand_icoor_atom_name( stub2_, rsd ) );
	std::string const stub3( expand_icoor_atom_name( stub3_, rsd ) );
	// 	bool const rebuild_icoor_xyz( ICoorAtomID( stub1, rsd ).is_internal() &&
	// 																ICoorAtomID( stub2, rsd ).is_internal() &&
	// 																ICoorAtomID( stub3, rsd ).is_internal() );
	bool const rebuild_icoor_xyz( true );
	rsd.set_icoor( atom, phi_, theta_, d_, stub1, stub2, stub3, rebuild_icoor_xyz );
	return false;
}

PrependMainchainAtom::PrependMainchainAtom( std::string const & atom_name_in ) :
	atom_name_( atom_name_in )
{}

bool
PrependMainchainAtom::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "PrependMainchainAtom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ <<	std::endl;
		return true; // failure
	} else {
		AtomIndices const & old_mainchain_atoms( rsd.mainchain_atoms() );
		AtomIndices new_mainchain_atoms;
		new_mainchain_atoms.push_back( rsd.atom_index( atom_name_ ) );
		for ( Size i = 1; i <= old_mainchain_atoms.size(); ++i ) {
			new_mainchain_atoms.push_back( old_mainchain_atoms[i] );
		}
		rsd.set_mainchain_atoms( new_mainchain_atoms );
	}
	return false;
}

AppendMainchainAtom::AppendMainchainAtom( std::string const & atom_name_in ) :
	atom_name_( atom_name_in )
{}

bool
AppendMainchainAtom::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "AppendMainchainAtom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ <<	std::endl;
		return true; // failure
	} else {
		AtomIndices new_mainchain_atoms( rsd.mainchain_atoms() );
		new_mainchain_atoms.push_back( rsd.atom_index( atom_name_ ) );
		rsd.set_mainchain_atoms( new_mainchain_atoms );
	}
	return false;
}


SetNbrAtom::SetNbrAtom( std::string const & atom_name_in ) :
	atom_name_( atom_name_in )
{}

bool
SetNbrAtom::apply( ResidueType & rsd ) const
{
	if ( !rsd.has( atom_name_ ) ) {
		TR_PatchOperations.Debug << "SetNbrAtom::apply failed: " << rsd.name() << " is missing atom " << atom_name_ <<	std::endl;
		return true; // failure
	} else {
		rsd.nbr_atom( atom_name_ );
	}
	return false;
}

SetNbrRadius::SetNbrRadius( Real const & radius ) :
	radius_( radius )
{}

bool
SetNbrRadius::apply( ResidueType & rsd ) const
{
	rsd.nbr_radius( radius_ );
	return false;
}


SetOrientAtom::SetOrientAtom(bool force_nbr_atom_orient):
	force_nbr_atom_orient_(force_nbr_atom_orient)
{}

bool
SetOrientAtom::apply( ResidueType & rsd ) const
{
	rsd.force_nbr_atom_orient( force_nbr_atom_orient_ );
	return false;
}

NCAARotLibPath::NCAARotLibPath( std::string const & path_in ) :
	path_( path_in )
{}

/// @brief set the NCAA rotamer library path in the residue type
bool
NCAARotLibPath::apply( ResidueType & rsd ) const
{
	rsd.set_ncaa_rotlib_path( path_ );
	rsd.set_use_ncaa_rotlib( true );
	return false;
}


PatchOperationOP
patch_operation_from_patch_file_line( std::string const & line ) {
	using numeric::conversions::radians;
	std::istringstream l( line );
	std::string tag, atom1, atom2, atom3, atom4, atom_name, atom_type_name, mm_atom_type_name, property, dummy;
	Real charge, mean, sdev, radius;
	Size chino;
	l >> tag;
	if ( l.fail() || tag[0] == '#' ) return 0;
	if ( tag == "ADD_ATOM" ) {
		if ( line.size() < 25 ) return 0;
		atom_name = line.substr( 9,4); l >> tag;
		l >> atom_type_name; // = line.substr( 14,4);
		l >> mm_atom_type_name; // = line.substr( 19,4);
		l >> charge;
		if ( l.fail() ) return 0;
		return new AddAtom( atom_name, atom_type_name, mm_atom_type_name, charge );

	} else if ( tag == "DELETE_ATOM" ) {
		l >> atom_name;
		if ( l.fail() ) return 0;
		return new DeleteAtom( atom_name );

	} else if ( tag == "SET_BACKBONE_HEAVYATOM" ) {
		l >> atom_name;
		if ( l.fail() ) return 0;
		return new SetBackboneHeavyatom( atom_name );

	} else if ( tag == "SET_IO_STRING" ) { // 13 character tag
		// NOTE - USE FIXED WIDTH IO SINCE NAME3 CAN CONTAIN INTERNAL WHITESPACE (EG DNA,RNA)
		if ( line.size() < 19 ) return 0;
		std::string const three_letter_code( line.substr(14,3) ), one_letter_code( line.substr(18,1) );
		return new SetIO_String( three_letter_code, one_letter_code[0] );

	} else if ( tag == "SET_INTERCHANGEABILITY_GROUP" ) {
		std::string intgrp;
		l >> intgrp;
		return new SetInterchangeabilityGroup_String( intgrp );

	} else if ( tag == "APPEND_INTERCHANGEABILITY_GROUP" ) {
		std::string intgrp_addendum;
		l >> intgrp_addendum;
		return new AppendInterchangeabilityGroup_String( intgrp_addendum );

	} else if ( tag == "NBR_ATOM" ) {
		l >> atom_name;
		if ( l.fail() ) return 0;
		return new SetNbrAtom( atom_name );

	} else if ( tag == "NBR_RADIUS" ) {
		Real radius;
		l >> radius;
		return new SetNbrRadius( radius );

	} else if ( tag == "ADD_PROPERTY" ) {
		l >> property;
		if ( l.fail() ) return 0;
		return new AddProperty( property );

	} else if ( tag == "DELETE_PROPERTY" ) {
		l >> property;
		if ( l.fail() ) return 0;
		return new DeleteProperty( property );

		// Added by Andy M. Chen in June 2009
		// This is needed for adding new side-chain torsions, which occurs in certain PTMs.
	} else if ( tag == "ADD_CHI" ) {
		if (line.substr(8, 3) == "N+1") {
			l >> dummy >> atom1 >> atom2 >> atom3 >> atom4;
			if ( l.fail() ) return 0;
			return new AddChi(atom1, atom2, atom3, atom4);
		} else {
			l >> chino >> atom1 >> atom2 >> atom3 >> atom4;
			if ( l.fail() ) return 0;
			return new AddChi(chino, atom1, atom2, atom3, atom4);
		}

		// Added by Andy M. Chen in June 2009
		// This is needed for PTMs, which often result in one or more extra chi angles.
	} else if ( tag == "ADD_PROTON_CHI") {
		Size chino, nsamples, nextra_samples;
		std::string dummy;
		l >> chino;
		l >> dummy; // should be "SAMPLES"
		l >> nsamples;
		utility::vector1< Real > samples( nsamples );
		for ( Size ii = 1; ii <= nsamples; ++ii ) {
			l >> samples[ ii ];
		}
		l >> dummy; // should be "EXTRA"
		l >> nextra_samples;
		utility::vector1< Real > extra_samples( nextra_samples );
		for ( Size ii = 1; ii <= nextra_samples; ++ii ) {
			l >> extra_samples[ ii ];
		}
		if ( l.fail() ) return 0;
		return new AddProtonChi( chino, samples, extra_samples );

	} else if ( tag == "REDEFINE_CHI" ) {
		l >> chino >> atom1 >> atom2 >> atom3 >> atom4;
		if ( l.fail() ) return 0;
		return new RedefineChi( chino, atom1, atom2, atom3, atom4 );

		//Added by Andy M. Chen in June 2009
		//    This is needed for PTM's
	} else if ( tag == "ADD_CHI_ROTAMER" ) {
		if (line.substr(16, 1) == "N") {
			l >> dummy >> mean >> sdev;
			if ( l.fail() ) return 0;
			return new AddChiRotamer(mean, sdev);
		} else {
			l >> chino >> mean >> sdev;
			if ( l.fail() ) return 0;
			return new AddChiRotamer(chino, mean, sdev);
		}

		//Added by Andy M. Chen in June 2009
		//    This is needed for PTM's
	} else if ( tag == "ADD_BOND" ) {
		l >> atom1 >> atom2;
		if ( l.fail() ) return 0;
		return new AddBond( atom1, atom2 );

	} else if ( tag == "ADD_CONNECT" ) {
		std::string connect_atom;
		l >> connect_atom;
		if ( l.fail() ) return 0;
		l >> tag;
		if ( l.fail() ) {
			return new AddConnect( connect_atom, 0.0, 0.0, 0.0, connect_atom, connect_atom, connect_atom );
		} else {
			Real phi, theta, d;
			std::string parent_atom, angle_atom, torsion_atom;
			l >> phi >> theta >> d >> parent_atom >> angle_atom >> torsion_atom;
			if ( l.fail() || tag != "ICOOR" ) {
				utility_exit_with_message( "bad line in patchfile: "+line );
			}
			return new AddConnect( connect_atom, radians(phi), radians(theta), d, parent_atom, angle_atom, torsion_atom );
		}

	} else if ( tag == "SET_ATOM_TYPE" ) {
		l >> atom_name >> atom_type_name;
		if ( l.fail() ) return 0;
		return new SetAtomType( atom_name, atom_type_name );

	} else if ( tag == "SET_MM_ATOM_TYPE" ) {
		l >> atom_name >> mm_atom_type_name;
		if ( l.fail() ) return 0;
		return new SetMMAtomType( atom_name, mm_atom_type_name );

	} else if ( tag == "SET_ATOMIC_CHARGE" ) {
		l >> atom_name >> charge;
		if ( l.fail() ) return 0;
		return new SetAtomicCharge( atom_name, charge );

	} else if ( tag == "SET_POLYMER_CONNECT" ) {
		l >> tag >> atom_name; // tag should be "UPPER" or "LOWER"
		if ( l.fail() ) return 0;
		return new SetPolymerConnectAtom( atom_name, tag );

	} else if ( tag == "SET_ICOOR" ) {
		Real phi,theta,d;
		std::string stub1, stub2, stub3;
		l >> atom_name >> phi >> theta >> d >> stub1 >> stub2 >> stub3;
		if ( l.fail() ) return 0;
		return new SetICoor( atom_name, radians(phi), radians(theta), d, stub1, stub2, stub3 );

	} else if ( tag == "PREPEND_MAINCHAIN_ATOM" ) {
		l >> atom_name;
		return new PrependMainchainAtom( atom_name );

	} else if ( tag == "APPEND_MAINCHAIN_ATOM" ) {
		l >> atom_name;
		return new AppendMainchainAtom( atom_name );

	} else if ( tag == "NCAA_ROTLIB_PATH" ) {
		std::string path;
		l >> path;
		return new NCAARotLibPath( path );

	} else if ( tag == "SET_NBR_ATOM") {
		l >> atom_name;
		if ( l.fail() ) return 0;
		return new SetNbrAtom( atom_name );

	} else if ( tag == "SET_NBR_RADIUS") {
		l >> radius;
		if ( l.fail() ) return 0;
		return new SetNbrRadius( radius );

	} else if ( tag == "SET_ORIENT_ATOM") {
		l >> tag;
		if ( tag == "NBR" ) {
			return new SetOrientAtom(true);
		} else if ( tag == "DEFAULT" ) {
			return new SetOrientAtom(false);
		} else {
			tr.Warning << "Unknown SET_ORIENT ATOM tag: " << tag << std::endl;
			return 0;
		}
	}
	tr.Warning << "patch_operation_from_patch_file_line: bad line: " << line << std::endl;

	return 0;
}

} // chemical
} // core
