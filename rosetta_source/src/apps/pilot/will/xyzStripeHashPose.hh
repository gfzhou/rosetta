#ifndef INCLUDED_apps_pilot_will_xyzStripeHashPose_hh
#define INCLUDED_apps_pilot_will_xyzStripeHashPose_hh

#include <apps/pilot/will/xyzStripeHash.hh>
#include <apps/pilot/will/gpu/gpu_bit_utils.hh>

#include <core/pose/Pose.hh>
#include <core/chemical/AtomType.hh>
#include <core/conformation/Residue.hh>
#include <core/id/AtomID.hh>

enum xyzStripeHashPoseMode {
  NBR,
  BB,
  BNP,
  HVY,
  ALL
};

class xyzStripeHashPose : public xyzStripeHash<float> {
public:
  xyzStripeHashPose(float radius, core::pose::Pose p, xyzStripeHashPoseMode m = BB ) : xyzStripeHash<float>(radius) { // makes copy
    int natom = 0;
    for(int ir = 1; ir <= p.n_residue(); ++ir) {
      core::conformation::Residue const & r(p.residue(ir));
      if( NBR==m ) natom++;
      if( BB ==m ) natom += r.has("N")+r.has("CA")+r.has("C")+r.has("O")+r.has("CB");
      if( HVY==m ) natom += r.nheavyatoms();
      if( ALL==m ) natom += r.natoms();
    }
    utility::vector1<numeric::xyzVector<float> > atoms(natom);
    // utility::vector1<float>                      meta (natom);
    uint count = 0;
    for(int ir = 1; ir <= p.n_residue(); ++ir) {
      core::conformation::Residue const & r(p.residue(ir));
      if(NBR==m) {
        int ia = r.nbr_atom();
        core::id::AtomID const aid(ia,ir);;
        atoms[++count] = p.xyz(aid);
        // meta [  count] = aidr_as_float(aid,r.atom_type(ia).lj_radius() );
      } else if(BB==m) {
        if(r.has( "N")){ atoms[++count]=r.xyz( "N"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index( "N"),ir),r.atom_type(r.atom_index( "N")).lj_radius());*/ }
        if(r.has("CA")){ atoms[++count]=r.xyz("CA"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index("CA"),ir),r.atom_type(r.atom_index("CA")).lj_radius());*/ }
        if(r.has( "C")){ atoms[++count]=r.xyz( "C"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index( "C"),ir),r.atom_type(r.atom_index( "C")).lj_radius());*/ }
        if(r.has( "O")){ atoms[++count]=r.xyz( "O"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index( "O"),ir),r.atom_type(r.atom_index( "O")).lj_radius());*/ }
        if(r.has("CB")){ atoms[++count]=r.xyz("CB"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index("CB"),ir),r.atom_type(r.atom_index("CB")).lj_radius());*/ }
      } else if(BB==m) {
        if(r.has("CA")){ atoms[++count]=r.xyz("CA"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index("CA"),ir),r.atom_type(r.atom_index("CA")).lj_radius());*/ }
		if(r.has( "C")){ atoms[++count]=r.xyz( "C"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index( "C"),ir),r.atom_type(r.atom_index( "C")).lj_radius());*/ }
        if(r.has("CB")){ atoms[++count]=r.xyz("CB"); /*meta[count]=aidr_as_float(core::id::AtomID(r.atom_index("CB"),ir),r.atom_type(r.atom_index("CB")).lj_radius());*/ }
      } else {
        int natom = (ALL==m) ? r.natoms() : r.nheavyatoms();;
        for(int ia = 1; ia <= natom; ++ia) {
          core::id::AtomID const aid(ia,ir);
          atoms[++count] = p.xyz(aid);
          // meta [  count] = aidr_as_float(aid,r.atom_type(ia).lj_radius() );
        }
      }
    }
    init(atoms);
  }

};

#endif
