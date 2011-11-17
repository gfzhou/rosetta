// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file calc_pair_stats.cc
/// @brief
/// @author Robert Vernon

#include <core/types.hh>
#include <devel/init.hh>
// AUTO-REMOVED #include <basic/prof.hh>

#include <core/chemical/AA.hh>

#include <core/scoring/dssp/Dssp.hh>

#include <core/pose/Pose.hh>
// Auto-header: duplicate removed #include <devel/init.hh>

#include <core/kinematics/RT.hh>
#include <core/io/pdb/pose_io.hh>
// AUTO-REMOVED #include <core/io/pdb/file_data.hh> //reading remarks
// AUTO-REMOVED #include <core/pose/PDBInfo.hh> //reading remarks

// AUTO-REMOVED #include <protocols/toolbox/pose_manipulation.hh> // ?

// AUTO-REMOVED #include <core/chemical/ChemicalManager.hh>
// AUTO-REMOVED

#include <basic/options/option.hh>
//#include <basic/options/keys/OptionKeys.hh>
#include <basic/options/keys/robert.OptionKeys.gen.hh>

#include <core/conformation/Conformation.fwd.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Atom.hh>

#include <utility/vector1.hh>
#include <utility/io/izstream.hh>
#include <utility/io/ozstream.hh>
#include <utility/file/FileName.hh>
// AUTO-REMOVED #include <utility/file/file_sys_util.hh>

#include <ObjexxFCL/char.functions.hh>
#include <ObjexxFCL/string.functions.hh>

#include <ObjexxFCL/FArray2A.hh>
#include <ObjexxFCL/FArray2A.fwd.hh>

#include <numeric/random/random.hh>

// AUTO-REMOVED #include <numeric/xyzMatrix.io.hh>
// AUTO-REMOVED #include <numeric/xyzVector.io.hh>

#include <numeric/xyzMatrix.hh>
#include <numeric/xyzMatrix.fwd.hh>

// C++ headers
#include <fstream>
#include <iostream>
#include <string>

//Auto Headers
#include <core/import_pose/import_pose.hh>
#include <protocols/jumping/StrandPairing.hh>
#include <numeric/xyz.functions.hh>
#include <ObjexxFCL/format.hh>


///////////////////////////////////////////////////////////////////////////////

using std::string;
using core::Size;
using core::Real;
using utility::vector1;
using utility::file::FileName;
using ObjexxFCL::fmt::A;
using ObjexxFCL::fmt::F;
using ObjexxFCL::fmt::I;
using ObjexxFCL::string_of;
using ObjexxFCL::FArray2A_float;
using ObjexxFCL::FArray2D_float;
//using namespace basic;
//using namespace core::sequence;
//using namespace basic::options;
//using namespace basic::options::OptionKeys;


//FUTURE FILE FORMAT:
//is >> tag >> filename >> pos1 >> pos2 >> cendist >>
//    ss1 >> ss2 >> seq_sep >>
//		Epos1(1,1) >> Epos1(2,1) >> Epos1(3,1) >>
//		Epos1(1,2) >> Epos1(2,2) >> Epos1(3,2) >>
//		Epos1(1,4) >> Epos1(2,4) >> Epos1(3,4) >>
//		Epos2(1,1) >> Epos2(2,1) >> Epos2(3,1) >>
//		Epos2(1,2) >> Epos2(2,2) >> Epos2(3,2) >>
//		Epos2(1,4) >> Epos2(2,4) >> Epos2(3,4) >>


//    CA_posx >> CA_posy >> CA_posz

// CA, N, C



//    phi1 >> psi1 >> omega1 >>
//    phi2 >> psi2 >> omega2;


using namespace core;
using namespace pose;
using namespace conformation;

//namespace protocols {
//namespace jumping {

void
get_coordinate_system(
	numeric::xyzMatrix_double const & p, //FArray2A_double p, // input
	numeric::xyzMatrix_double & m //FArray2A_double m // output
)
{
	using namespace numeric;

	xyzVector_double a1 = p.col_x() - p.col_y();
	xyzVector_double a2 = p.col_z() - p.col_y();
	a1.normalize();
	xyzVector_double a3 = cross( a1, a2 );
	a3.normalize();
	a2 = cross( a3, a1 );

	m = xyzMatrix_double::cols( a1, a2, a3 );
}

void
get_ncac(
	FArray2A_float pos,
	numeric::xyzMatrix_double & p
)
{
	pos.dimension( 3, 5 );
	using namespace numeric;

	xyzVector_double n( &pos(1,1) );
	xyzVector_double ca( &pos(1,2) );
	xyzVector_double c( &pos(1,4) );

	p = xyzMatrix_double::cols( n, ca, c );

}

numeric::xyzMatrix_double
get_ncac ( FArray2A_float pos )
{
	pos.dimension( 3, 5 );
	using namespace numeric;

	xyzVector_double n( &pos(1,1) );
	xyzVector_double ca( &pos(1,2) );
	xyzVector_double c( &pos(1,4) );

	return xyzMatrix_double::cols( n, ca, c );
}

//helper code to make an RT from two Epos
// does this live somewhere else in mini, haven't found it !
using core::kinematics::RT;
RT RT_from_epos( FArray2A_float Epos1, FArray2A_float Epos2)
{
	/// rotation matrix, written in stub1 frame
	RT::Matrix rotation( 0.0 ); // 3x3
	/// tranlsation vector, written in stub1 frame
	RT::Vector translation( 0.0 ); // 3

	Size const MAX_POS( 5 ); // param::MAX_POS
	Epos1.dimension(3,MAX_POS);
	Epos2.dimension(3,MAX_POS);

	//bool const local_debug ( false );

	numeric::xyzMatrix_double p1, p2, m1, m2;

	// get coordinate systems from both residues
	get_ncac(Epos1,p1);
	get_ncac(Epos2,p2);
	get_coordinate_system(p1,m1);
	get_coordinate_system(p2,m2);

	// consider this:       |xx xy xz|
	// coordinate frame M = |yx yy yz|
	//                      |zx zy zz|
	// each column is a unit vector written in genuine frame.
	//
	// vector A in frame M can be rewritten as B in genuine frame
	// by the formula B = M x A, thus A = M^T x B
	// a simple example of this would be: the unit vector (1,0,0) in frame M
	// is actually (xx,yx,zx) in genuine frame. mathematically,
	// |xx|   |xx xy xz|   |1|
	// |yx| = |yx yy yz| x |0| ==> B = M x A
	// |zx|   |zx zy zz|   |0|
	//
	// the above formula has another layer of meaning: rotation
	// keeping the genuine frame fixed, a vector can be rotated by applying
	// matrix M onto it, e.g., (1,0,0) rotated to (xx,yx,zx)

	numeric::xyzVector_double e1( &Epos1(1,2) );
	numeric::xyzVector_double e2( &Epos2(1,2) );

	// ( e2 - e1 ) is the vector in genuine frame,
	// translation is the vector in m1 frame. so m1^T is multiplied.
	translation = m1.transposed() * ( e2 - e1 );

	// let's look at the rotation matrix
	// A, B, C are three vectors in genuine frame and are related by rotation
	// B = M1 x A; C = M2 x A;
	// ==> A = M1^T x B = M2^T x C
	// ==> C = M2 x M1^T x B
	// but note that C and B are both in genuine frame and we want a rotation
	// matrix to be applied onto a vector in M1 frame, so comes another step of
	// conversion -- left-multiply M1^T on both sides:
	// M1^T x C = M1^T x M2 x M1^T x B
	// C' = M1^T x M2 x B', as C' and B' are written in M1 frame.
	// so we get the rotation matrix as M1^T x M2.
	// but wait a minute, what Phil orginally got below is M2^T x M1 and it is
	// impossible for that to be wrong, then what happens?

	// It turns out when this rotation matrix is further applied to a vector,
	// it uses Charlies' (col,row) convention (see Dvect_multiply()
	// in RT::make_jump) which means there is one more transpose to do.
	// Now an agreement is reached:
	//  (M2^T x M1)^T = M1^T x (M2^T)^T = M1^T x M2
	// since xyzMatrix uses the normal (row,col) convention, we will switch to
	// rotation = M1^T x M2

	rotation = m1.transposed() * m2;
	/************************Phil's legacy code *********************/
	// rotation(j,*) is the j-th unit vector of 2nd coord sys written in 1st coord-sys
	// 	for ( int i=1; i<=3; ++i ) {
	// 			for ( int j=1; j<=3; ++j ) {
	// 				// DANGER: not sure about the order... ////////////////////////
	// 				// should sync with make_jump
	//  				rotation(j,i) = Ddotprod( m1(1,i), m2(1,j) ); // 2nd guess
	//  				//rotation(j,i) = Ddotprod( m1(1,j), m2(1,i) ); // 1st guess
	// 			}
	// 		}
	/************************Phil's legacy code ********************/
	RT rt;
	rt.set_translation( translation );
	rt.set_rotation( rotation );

	return rt;
}

int
main( int argc, char* argv [] )
{
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	// options, random initialization
	devel::init( argc, argv );

	std::string const pdb_file_list( option[ robert::pairdata_input_pdb_list ]() );
	//std::string const pdb_file_list("list");

	utility::io::izstream infile(pdb_file_list);
	utility::io::ozstream outfile("disulfide.data");

	Size res1, res2;
	std::string pdb_file_location;
	infile >> pdb_file_location >> res1 >> res2;

	//bool iterate = true;
	while (infile.good()) {

		Size const MAX_POS( 5 );
		FArray2D_float Epos1(3, MAX_POS), Epos2(3,MAX_POS);

		pose::Pose pdb;

		std::cout << "PROCESSING PDB: " << pdb_file_location << std::endl;
		core::import_pose::pose_from_pdb( pdb, pdb_file_location );
		std::cout << "PROCESSING COMPLETE: " << pdb_file_location << std::endl;

		core::scoring::dssp::Dssp dssp( pdb );
		dssp.insert_ss_into_pose( pdb );



		Epos1(1,2) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(1);
 		Epos1(2,2) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(2);
		Epos1(3,2) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(3);

		Epos1(1,1) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(1);
		Epos1(2,1) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(2);
		Epos1(3,1) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(3);

		Epos1(1,4) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(1);
		Epos1(2,4) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(2);
		Epos1(3,4) = pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(3);

		Epos2(1,2) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(1);
		Epos2(2,2) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(2);
		Epos2(3,2) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(3);

		Epos2(1,1) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(1);
		Epos2(2,1) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(2);
		Epos2(3,1) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(3);

		Epos2(1,4) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(1);
		Epos2(2,4) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(2);
		Epos2(3,4) = pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(3);

		core::kinematics::RT this_rt(RT_from_epos(Epos1,Epos2));



		std::cout << "DISULF ";

 		std::cout << pdb_file_location << " ";
 		std::cout << res1 << " " << res2 << " ";

 		std::cout << pdb.residue(res1).aa() << " ";
 		std::cout << pdb.residue(res2).aa() << " ";

		std::cout << pdb.secstruct(res1) << " ";
		std::cout << pdb.secstruct(res2) << " ";

		std::cout << abs(res1-res2) << " ";

		std::cout << this_rt << std::endl;

		// std::cout << "EPOS ";

//  		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("CA")).xyz()(3) << " ";

// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("N")).xyz()(3) << " ";

// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res1).atom(pdb.residue(res1).atom_index("C")).xyz()(3) << " ";

// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("CA")).xyz()(3) << " ";

// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("N")).xyz()(3) << " ";

// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(1) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(2) << " ";
// 		std::cout << pdb.residue(res2).atom(pdb.residue(res2).atom_index("C")).xyz()(3) << " ";

// 		std::cout << std::endl;

	// 	utility::vector1< pose::PoseOP > pdb_chains(pdb.split_by_chain());

// 		for ( utility::vector1< core::pose::PoseOP >::iterator it = pdb_chains.begin(), end = pdb_chains.end(); it != end; ++it ) {
// 			core::scoring::dssp::Dssp dssp(**it);
// 			dssp.insert_ss_into_pose( **it );

// 			core::Size n1(1);
// 			for ( ResidueOPs::iterator res1( (*it)->res_begin() ); res1 != (*it)->res_end(); ++res1, ++n1 ) {
// 				core::Size n2(1);
// 				for ( ResidueOPs::iterator res2( (*it)->res_begin() ); res2 != (*it)->res_end(); ++res2, ++n2 ) {
// 					if (((*res1)->is_protein() == true) and ((*res2)->is_protein() == true) ) {

// 						core::conformation::Atom ca_pos_1 = (*res1)->atom((*res1)->atom_index("CA"));


// 						std::cout << "NUMBERS " << ca_pos_1.xyz()(1) << " " << ca_pos_1.xyz()(2) << " " << ca_pos_1.xyz()(3) << " " << std::endl;

// 					}
// 				}
// 			}
// 		}
//	}


		//std::cout << "DONE!" << std::endl;

		pdb_file_location = "";
		infile >> pdb_file_location >> res1 >> res2;

	}
	//if (pdb_file_location == "") iterate = false;


		//std::string name( option[ in::file::vall ]() );
	//FileName fn( name );
	//VallReader vall( name );
} // int main( int argc, char * argv [] )

//}
//}
