// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file src/devel/denovo_design/calculators/ResidueCentralityCalculator.hh
/// @brief header file for ResidueCentralityCalculator class.
/// Roughly, fragment quality is number of fragments which are close to a pose in rmsd
/// @detailed
/// @author Tom Linsky (tlinsky@gmail.com)


#ifndef INCLUDED_devel_denovo_design_calculators_ResidueCentralityCalculator_hh
#define INCLUDED_devel_denovo_design_calculators_ResidueCentralityCalculator_hh

#include <core/pose/metrics/PoseMetricCalculatorBase.hh>
#include <basic/MetricValue.fwd.hh>
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>

// Utility headers
#include <utility/pointer/ReferenceCount.hh>
#include <utility/vector1.hh>

//// C++ headers
#include <list>

// Parser headers
#include <protocols/moves/DataMap.fwd.hh>
#include <protocols/moves/Mover.fwd.hh>
#include <protocols/filters/Filter.fwd.hh>
#include <utility/tag/Tag.fwd.hh>

namespace devel {
namespace denovo_design {
namespace calculators {

class Node;
class Edge;
typedef utility::pointer::owning_ptr< Node > NodeOP;
typedef utility::pointer::owning_ptr< Node const > NodeCOP;
typedef utility::pointer::owning_ptr< Edge > EdgeOP;
typedef utility::pointer::owning_ptr< Edge const > EdgeCOP;

// Nodes and edges for dijkstra's algorithm
class Node : public utility::pointer::ReferenceCount
{
public:
	Node( std::string const id, core::Size const resi)
		: resi(resi), id(id),
			distanceFromStart(9999),
			in_list( false )
	{
		neighbors.clear();
	}

public:
	core::Size resi;
	std::string id;
	std::list< NodeOP > neighbors;
	int distanceFromStart;
	bool in_list;
};

NodeOP
ExtractSmallest( std::list< NodeOP > & nodes);

std::list< NodeOP >
AdjacentRemainingNodes( std::list< NodeOP > const & nodes, NodeOP node);

bool
Contains( std::list< NodeOP > const & nodes, NodeCOP node );

void
find_neighbors( core::pose::Pose const & pose, std::list< NodeOP > const & nodes );


class ResidueCentralityCalculator : public core::pose::metrics::StructureDependentCalculator {
public:// constructor/destructor

	/// @brief default constructor
	ResidueCentralityCalculator();

	/// @brief destructor
	virtual ~ResidueCentralityCalculator();

public:// virtual constructor
	/// @brief make clone
  core::pose::metrics::PoseMetricCalculatorOP
	clone() const { return new ResidueCentralityCalculator( *this ); }

public:// mutators

protected:
  virtual void lookup( std::string const & key, basic::MetricValueBase * valptr ) const;
  virtual std::string print( std::string const & key ) const;
  virtual void recompute( core::pose::Pose const & this_pose );

private:// private functions
	/// @brief determine the connectivity index of a residue
	core::Real
	connectivity_index( std::list< NodeOP > const & nodes,
											core::Size const resi ) const;

private:// member variables
	/// @brief the computed list of centrality indices, by residue
	utility::vector1< core::Real > centralities_;

	/// @brief a list of residues excluded from the calculation
	utility::vector1< core::Size > excluded_residues_;

}; //ResidueCentralityCalculator


} // ns PoseMetricCalculators
} // ns denovo_design
} // ns devel

#endif
