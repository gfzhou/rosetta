// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @author Oliver Lange

#ifndef INCLUDED_protocols_abinitio_PairingStatistics_hh
#define INCLUDED_protocols_abinitio_PairingStatistics_hh

// Unit Headers
#include <protocols/abinitio/PairingStatistics.fwd.hh>


// Package Headers
// AUTO-REMOVED #include <protocols/abinitio/Templates.hh>
// AUTO-REMOVED #include <protocols/abinitio/Template.hh>
// AUTO-REMOVED #include <protocols/abinitio/TemplateJumpSetup.fwd.hh>

// Project Headers
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>

// AUTO-REMOVED #include <core/fragment/FragSet.fwd.hh>
// AUTO-REMOVED #include <core/fragment/FragData.fwd.hh>
// AUTO-REMOVED #include <core/fragment/FrameList.fwd.hh>
// AUTO-REMOVED #include <core/fragment/SingleResidueFragData.fwd.hh>

// AUTO-REMOVED #include <core/scoring/constraints/AtomPairConstraint.fwd.hh>
// AUTO-REMOVED #include <core/scoring/constraints/ConstraintSet.fwd.hh>

#include <protocols/jumping/PairingsList.fwd.hh>
// AUTO-REMOVED #include <core/fragment/SecondaryStructure.fwd.hh>
#include <protocols/jumping/StrandPairing.hh>


// ObjexxFCL Headers

// Utility headers
#include <utility/pointer/ReferenceCount.hh>
#include <utility/exit.hh>


//// C++ headers
// AUTO-REMOVED #include <cstdlib>
#include <string>
// AUTO-REMOVED #include <vector>
#include <map>

#include <protocols/abinitio/Templates.fwd.hh>
#include <utility/vector1.hh>


namespace protocols {
namespace abinitio {


class PairingStatEntry  {
public:
  typedef std::string Model;
  typedef utility::vector1< Model > ModelList;

  PairingStatEntry();
  PairingStatEntry( jumping::StrandPairing const& strand, Model const& id );

  core::Size frequency() const { return models_.size(); };
  bool add_pairing( jumping::StrandPairing const&, Model const& );
  bool compatible( jumping::StrandPairing const& ) const;

  void show( std::ostream& ) const;

  void set_weight( core::Real setting ) {
    weight_ = setting;
  }

  core::Real weight() const {
    return weight_;
  }

  core::Size size() const {
    return strand_pairing_.size();
  }

	core::Size contact_order() const {
		return pairing().contact_order();
	}

	jumping::StrandPairing const& pairing() const {
		return strand_pairing_;
	}

	ModelList const& models() const {
		return models_;
	}

	ModelList& models() {
		return models_;
	}

	bool has_model( std::string const& ) const;

	friend std::istream& operator>> ( std::istream& is, PairingStatEntry& ps );

private:
  ModelList models_;
  jumping::StrandPairing strand_pairing_;
  core::Real weight_;
};

inline std::ostream& operator<< ( std::ostream& out, PairingStatEntry const& ps ) {
	ps.show( out );
	return out;
}

typedef utility::vector1< PairingStatEntry > StatEntries;

class PairingStatistics : public utility::pointer::ReferenceCount {
public:
  typedef StatEntries::const_iterator const_iterator;
  typedef PairingStatEntry::Model Model; //String ID !!!
  typedef std::map< Model, jumping::StrandPairingSet > Topologies;
  typedef std::map< Model, core::Size > ModelFreq;
  typedef utility::vector1< std::pair< core::Real, Model > > ModelWeight;
  typedef std::map< Model, core::Real > Weights;

	PairingStatistics() {};

  PairingStatistics( Templates const& templates ) {
    compute( templates );
  };

	PairingStatistics( jumping::StrandPairingSet const& topology );

  void set_native_topology( jumping::StrandPairingSet const& topology ) {
    native_topology_ = topology;
  }

  jumping::StrandPairingSet const& get_native_topology() const {
    return native_topology_;
  }

  const_iterator begin() const {
    return entries_.begin();
  }

  const_iterator end() const {
    return entries_.end();
  }

  Model ranked_model( Size nr ) const {
    if ( nr > model_weight_.size() ) return "BOGUS";
    return model_weight_[ nr ].second;
  }

  core::Size nr_models() const {
    return model_weight_.size();
  }

  core::Real weight( Size nr ) const {
    runtime_assert( nr <= model_weight_.size() );
    return model_weight_[ nr  ].first;
  }

	core::Real weight( Model id ) const;

//		return weights_[ id ];
  //	}

  jumping::StrandPairingSet const& topology( Model id ) const {
    Topologies::const_iterator iter ( topols_.find( id ) );
    if ( iter == topols_.end() ) {
      utility_exit_with_message("Model name not known: " + id );
    }
    return iter->second;
  };

  core::Real strand_weight( jumping::StrandPairing const& pairing ) const;

  jumping::StrandPairingSet const& suggest_topology( std::string& topol_id ) const;

  friend std::ostream& operator<< ( std::ostream&, PairingStatistics const& ps );
	friend std::istream& operator>> ( std::istream& is, PairingStatistics& ps );

  bool is_native_pairing( jumping::StrandPairing const& pairing ) const {
    return native_topology_.has_pairing( pairing );
  }

	static void register_options();

  void add_topology( jumping::StrandPairingSet const& topology, Model const& id );

  void compute_model_weights( ModelFreq& );

	//	void remove_stupid_strands();

private:
  void compute( Templates const& templates );

  StatEntries entries_;
  Topologies topols_;
  ModelWeight model_weight_; // for each model (full length name) a weight
  //	Weights weights_; // same as in ModelWeight, but as a map MODEL --> weight
  jumping::StrandPairingSet native_topology_;
};

extern std::ostream& operator<< ( std::ostream&, PairingStatistics const& ps );

extern std::ostream& operator<< ( std::ostream&, StatEntries const&  );
extern std::istream& operator<< ( std::istream&, StatEntries&  );

}
}

#endif
