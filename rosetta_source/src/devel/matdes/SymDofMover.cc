// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.
//
/// @file 
/// @brief 
/// @author Jacob Bale ( balej@uw.edu )

// Unit headers
#include <devel/matdes/SymDofMover.hh>
#include <devel/matdes/SymDofMoverCreator.hh>

// Package headers
#include <devel/matdes/SymDofMoverSampler.hh>

// project headers
#include <protocols/moves/Mover.hh>
#include <core/id/AtomID_Map.hh>
#include <core/pose/Pose.hh>
#include <core/pose/util.hh>
#include <core/pose/symmetry/util.hh>
#include <core/conformation/symmetry/util.hh>
#include <core/conformation/symmetry/VirtualCoordinate.hh>
#include <core/conformation/symmetry/SymmData.hh>
#include <numeric/xyz.functions.hh>
#include <numeric/random/random.hh>
#include <core/kinematics/Jump.hh>
#include <core/scoring/ScoreFunction.hh>
#include <protocols/moves/DataMap.hh>
#include <protocols/filters/Filter.hh>
#include <utility/tag/Tag.hh>
#include <basic/Tracer.hh>
#include <utility/exit.hh>
#include <basic/options/option.hh>
#include <basic/options/keys/symmetry.OptionKeys.gen.hh>
#include <utility/vector1.hh>

static basic::Tracer TR("devel.matdes.SymDofMover");

static numeric::random::RandomGenerator RG(832156);

namespace devel {
namespace matdes {

using namespace core;
using namespace utility;
using core::pose::Pose;

typedef numeric::xyzVector<Real> Vec;
typedef numeric::xyzMatrix<Real> Mat;

// -------------  Mover Creator -------------
std::string
SymDofMoverCreator::keyname() const
{
	return SymDofMoverCreator::mover_name();
}

protocols::moves::MoverOP
SymDofMoverCreator::create_mover() const {
	return new SymDofMover;
}

std::string
SymDofMoverCreator::mover_name()
{
	return "SymDofMover";
}
// -------------  Mover Creator -------------

SymDofMover::SymDofMover() :
	symm_file_(),
	sym_dof_names_(),
	radial_disps_(),
	angles_(),
	sampling_mode_( "single_dock" )
{ }

/*SymDofMover::SymDofMover(const SymDofMover& rval) :
	symm_file_( rval.symm_file_ ),
	sym_dof_names_( rval.sym_dof_names_ ),
	radial_disps_( rval.radial_disps_ ),
	angles_( rval.angles_ ),
	radial_disps_range_min_( rval.radial_disps_range_min_ ),
	radial_disps_range_max_( rval.radial_disps_range_max_ ),
	angles_range_min_( rval.angles_range_min_ ),
	angles_range_max_( rval.angles_range_max_ ),
	radial_disp_steps_( rval.radial_disp_steps_ ),
	angle_steps_( rval.angle_steps_ ),
	radial_disp_deltas_( rval.radial_disp_deltas_ ),
	angle_deltas_( rval.angle_deltas_ ),
	sampling_mode_( rval.sampling_mode_ )

{ }
*/
protocols::moves::MoverOP 
SymDofMover::clone() const {
	return new SymDofMover( *this );
}

protocols::moves::MoverOP 
SymDofMover::fresh_instance() const {
				return new SymDofMover();
}

utility::vector1<std::string>
SymDofMover::get_sym_dof_names() { 
	return sym_dof_names_;
}

utility::vector1<Real>
SymDofMover::get_angles() { 
	utility::vector1< std::string > sym_dof_names = get_sym_dof_names();
	utility::vector1<Real> angles;
	if(sampling_mode_ == "grid" ) {
		utility::vector1<Real> angle_diffs = SymDofMoverSampler::get_instance().get_angles();
		for (int i = 1; i <= sym_dof_names.size(); i++) {
			angles.push_back(angles_[i] + angle_diffs[i]);
		}
	}
	else {
		for (int i = 1; i <= sym_dof_names.size(); i++) {
			if(sampling_mode_ == "uniform") {
				angles.push_back(angles_[i] + angles_range_min_[i] + ( angles_range_max_[i] - angles_range_min_[i]) * RG.uniform());
			} else if(sampling_mode_ == "gaussian") {
				angles.push_back(angles_[i] + angle_deltas_[i] * RG.gaussian());
			} else {
				angles.push_back(angles_[i]);
			}
		}
	}
	return angles;
}

utility::vector1<Real>
SymDofMover::get_radial_disps() { 
	utility::vector1< std::string > sym_dof_names = get_sym_dof_names();
	utility::vector1<Real> radial_disps;
	if(sampling_mode_ == "grid" ) {
		utility::vector1<Real> radial_diffs = SymDofMoverSampler::get_instance().get_radial_disps();
		for (int i = 1; i <= sym_dof_names.size(); i++) {
			radial_disps.push_back(radial_disps_[i] + radial_diffs[i]);
		}
	}
	else {
		for (int i = 1; i <= sym_dof_names.size(); i++) {
			if(sampling_mode_ == "uniform") {
				radial_disps.push_back(radial_disps_[i] + radial_disps_range_min_[i] + ( radial_disps_range_max_[i] - radial_disps_range_min_[i]) * RG.uniform());
			} else if(sampling_mode_ == "gaussian") {
				radial_disps.push_back(radial_disps_[i] + radial_disp_deltas_[i] * RG.gaussian());
			} else {
				radial_disps.push_back(radial_disps_[i]);
			}
		}
	}
	return radial_disps;
}

// pose manipulation. Consider moving to util.cc ? //

void SymDofMover::trans_pose( Pose & pose, Vec const & trans, Size start, Size end ) {
	for(Size ir = start; ir <= end; ++ir) {
		for(Size ia = 1; ia <= pose.residue_type(ir).natoms(); ++ia) {
			core::id::AtomID const aid(core::id::AtomID(ia,ir));
			pose.set_xyz( aid, pose.xyz(aid) + (Vec)trans );
		}
	}
}
void SymDofMover::rot_pose( Pose & pose, Mat const & rot, Size start, Size end ) {
	for(Size ir = start; ir <= end; ++ir) {
		for(Size ia = 1; ia <= pose.residue_type(ir).natoms(); ++ia) {
			core::id::AtomID const aid(core::id::AtomID(ia,ir));
			pose.set_xyz( aid, rot * pose.xyz(aid) );
		}
	}
}
void SymDofMover::rot_pose( Pose & pose, Mat const & rot, Vec const & cen, Size start, Size end ) {
	trans_pose(pose,-cen,start,end);
	rot_pose(pose,rot,start,end);
	trans_pose(pose,cen,start,end);
}
void SymDofMover::rot_pose( Pose & pose, Vec const & axis, double const & ang, Size start, Size end ) {
	rot_pose(pose,rotation_matrix_degrees(axis,ang),start,end);
}
void SymDofMover::rot_pose( Pose & pose, Vec const & axis, double const & ang, Vec const & cen, Size start, Size end ) {
	rot_pose(pose,rotation_matrix_degrees(axis,ang),cen,start,end);
}
void SymDofMover::alignaxis(core::pose::Pose & pose, Vec newaxis, Vec oldaxis, Vec cen, Size start, Size end ) {
	newaxis.normalize();
	oldaxis.normalize();
	if (newaxis.normalize() != oldaxis.normalize()) {
		Vec axis = newaxis.cross(oldaxis).normalized();
		Real ang = -acos(numeric::max(-1.0,numeric::min(1.0,newaxis.dot(oldaxis))))*180/numeric::constants::d::pi;
		rot_pose(pose,axis,ang,cen,start,end);
	}
}

// Do stuff //

void
SymDofMover::apply(Pose & pose) {
	using core::pose::Pose;

// Read in user info //

	utility::vector1< std::string > sym_dof_names = get_sym_dof_names();
	utility::vector1<Real> radial_disps = get_radial_disps();
	utility::vector1<Real> angles = get_angles();
	std::string symm_file = symm_file_;

// Read in symmetry info from symmetry definition file //
	
	core::conformation::symmetry::SymmData symmdata( pose.n_residue(), pose.num_jump() );
	symmdata.read_symmetry_data_from_file(symm_file);
	std::map< std::string, core::conformation::symmetry::VirtualCoordinate > coords = symmdata.get_virtual_coordinates();
	std::map< std::string, std::pair< std::string, std::string > > virt_connects = symmdata.get_virtual_connects();

	for (int i = 1; i <= sym_dof_names.size(); i++) {

		core::Size sub_start= pose.conformation().chain_begin(i);
		core::Size sub_end= pose.conformation().chain_end(i);
	
// translate each subunit along the z-axis by user defined values //
		trans_pose(pose,Vec(0,0,radial_disps[i]),sub_start,sub_end);

// rotate each subunit along the z-axis by user defined values //
		rot_pose(pose,Vec(0,0,1),angles[i],sub_start,sub_end);

// read in the axes for each subunit //
		std::string tag (virt_connects.find( sym_dof_names[i])->second.first );
		TR << sym_dof_names[i] << " -> " << tag << std::endl;
		conformation::symmetry::VirtualCoordinate virt_coord( coords.find( tag )->second );

// align the z-axis of each subunit with the appropriate axis of the symdof jump from the symmetry definition file //
		alignaxis(pose,virt_coord.get_x(),Vec(0,0,1),Vec(0,0,0),sub_start,sub_end);
	}

// symmetrize pose //

	core::pose::symmetry::make_symmetric_pose(pose, symmdata);

	if( sampling_mode_ == "grid" ) {
		SymDofMoverSampler::get_instance().step();
	}

}

// Parse info from xml //
void 
SymDofMover::parse_my_tag( TagPtr const tag,
										 DataMap & data,
										 Filters_map const &,
										 Movers_map const &,
										 Pose const & ) {

	// Turn symmetry hacks on
	basic::options::option[basic::options::OptionKeys::symmetry::symmetry_definition].value( "dummy" );

	using std::string;
	symm_file_ = tag->getOption<string>( "symm_file" );
	sym_dof_names_ = utility::string_split( tag->getOption< std::string >( "sym_dof_names" ), ',' );

	sampling_mode_ = tag->getOption<std::string>("sampling_mode", "single_dock");
	TR << "Setting sampling mode to " << sampling_mode_ << std::endl;

	utility::vector1< std::string > radial_disp_strings = utility::string_split( tag->getOption< std::string >( "radial_disps" ), ',' );
	utility::vector1<Real> radial_disps;
	Real real_disp;
	for(Size i = 1; i <= radial_disp_strings.size(); i++) {
		real_disp = std::atof( radial_disp_strings[i].c_str() );
		radial_disps.push_back( real_disp );
	}
	radial_disps_ = radial_disps;

	utility::vector1< std::string > angle_strings = utility::string_split( tag->getOption< std::string >( "angles" ), ',' );
	utility::vector1<Real> angles;
	Real real_angle;
	for(Size i = 1; i <= angle_strings.size(); i++) {
		real_angle = std::atof( angle_strings[i].c_str() );
		angles.push_back( real_angle );
	}
	angles_ = angles;

	if( sampling_mode_ == "grid" || sampling_mode_ == "uniform") {

		utility::vector1< std::string > radial_disp_range_min_strings = utility::string_split( tag->getOption< std::string >( "radial_disps_range_min" ), ',' );
		utility::vector1<Real> radial_disps_range_min;
		Real real_disps_range_min;
		for(Size i = 1; i <= radial_disp_range_min_strings.size(); i++) {
			real_disps_range_min = std::atof( radial_disp_range_min_strings[i].c_str() );
			radial_disps_range_min.push_back( real_disps_range_min );
		}
		radial_disps_range_min_ = radial_disps_range_min;

		utility::vector1< std::string > radial_disp_range_max_strings = utility::string_split( tag->getOption< std::string >( "radial_disps_range_max" ), ',' );
		utility::vector1<Real> radial_disps_range_max;
		Real real_disps_range_max;
		for(Size i = 1; i <= radial_disp_range_max_strings.size(); i++) {
			real_disps_range_max = std::atof( radial_disp_range_max_strings[i].c_str() );
			radial_disps_range_max.push_back( real_disps_range_max );
		}
		radial_disps_range_max_ = radial_disps_range_max;

		utility::vector1< std::string > angle_range_min_strings = utility::string_split( tag->getOption< std::string >( "angles_range_min" ), ',' );
		utility::vector1<Real> angles_range_min;
		Real real_angles_range_min;
		for(Size i = 1; i <= angle_range_min_strings.size(); i++) {
			real_angles_range_min = std::atof( angle_range_min_strings[i].c_str() );
			angles_range_min.push_back( real_angles_range_min );
		}
		angles_range_min_ = angles_range_min;

		utility::vector1< std::string > angle_range_max_strings = utility::string_split( tag->getOption< std::string >( "angles_range_max" ), ',' );
		utility::vector1<Real> angles_range_max;
		Real real_angles_range_max;
		for(Size i = 1; i <= angle_range_max_strings.size(); i++) {
			real_angles_range_max = std::atof( angle_range_max_strings[i].c_str() );
			angles_range_max.push_back( real_angles_range_max );
		}
		angles_range_max_ = angles_range_max;

	}

	if( sampling_mode_ == "grid" ) {

		utility::vector1< std::string > angle_step_strings = utility::string_split( tag->getOption< std::string >( "angle_steps" ), ',' );
		utility::vector1<Real> angle_steps;
		Real real_angle_steps;
		for(Size i = 1; i <= angle_step_strings.size(); i++) {
			real_angle_steps = std::atof( angle_step_strings[i].c_str() );
			angle_steps.push_back( real_angle_steps );
		}
		angle_steps_ = angle_steps;

		utility::vector1< std::string > radial_disp_step_strings = utility::string_split( tag->getOption< std::string >( "radial_disp_steps" ), ',' );
		utility::vector1<Real> radial_disp_steps;
		Real real_radial_disp_steps;
		for(Size i = 1; i <= radial_disp_step_strings.size(); i++) {
			real_radial_disp_steps = std::atof( radial_disp_step_strings[i].c_str() );
			radial_disp_steps.push_back( real_radial_disp_steps );
		}
		radial_disp_steps_ = radial_disp_steps;

		TR << "Setting the exploration grid." << std::endl;
	
		SymDofMoverSampler::get_instance().set_angle_ranges(angles_range_min_, angles_range_max_, angle_steps);

		SymDofMoverSampler::get_instance().set_radial_disp_ranges(radial_disps_range_min_, radial_disps_range_max_, radial_disp_steps);

	} 

	if ( sampling_mode_ == "gaussian" ) {

		utility::vector1< std::string > angle_delta_strings = utility::string_split( tag->getOption< std::string >( "angle_deltas" ), ',' );
		utility::vector1<Real> angle_deltas;
		Real real_angle_delta;
		for(Size i = 1; i <= angle_delta_strings.size(); i++) {
			real_angle_delta = std::atof( angle_delta_strings[i].c_str() );
			angle_deltas.push_back( real_angle_delta );
		}
		angle_deltas_ = angle_deltas;
	
		utility::vector1< std::string > radial_disp_delta_strings = utility::string_split( tag->getOption< std::string >( "radial_disp_deltas" ), ',' );
		utility::vector1<Real> radial_disp_deltas;
		Real real_radial_disp_delta;
		for(Size i = 1; i <= radial_disp_delta_strings.size(); i++) {
			real_radial_disp_delta = std::atof( radial_disp_delta_strings[i].c_str() );
			radial_disp_deltas.push_back( real_radial_disp_delta );
		}
		radial_disp_deltas_ = radial_disp_deltas;

	}
}

} // matdes
} // devel
