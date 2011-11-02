// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   core/pack/packer_neighbors.cc
/// @brief  creates a graph that describes the possible connectivity induced by designing-in larger side chains
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com)

// Unit Headers
#include <core/pack/packer_neighbors.hh>

// Package Headers
#include <core/pack/rotamer_set/RotamerSetOperation.hh>
#include <core/pack/task/PackerTask.hh>

// Project Headers
#include <core/conformation/Residue.hh>
// AUTO-REMOVED #include <core/chemical/ResidueTypeSet.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/Energies.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionInfo.hh>
#include <core/graph/Graph.hh>
#include <core/conformation/PointGraph.hh>
#include <core/conformation/find_neighbors.hh>

// Utility Headers
#include <utility/vector1.functions.hh>

#include <core/conformation/PointGraphData.hh>
#include <core/graph/UpperEdgeGraph.hh>
#include <utility/vector1.hh>


namespace core {
namespace pack {



/// @brief Constructs a graph where edges represent the possibility of interactions between
/// residue pairs
graph::GraphOP
create_packer_graph(
	pose::Pose const & pose,
	scoring::ScoreFunction const & scfxn,
	task::PackerTaskCOP task
)
{
	utility::vector1< Distance > residue_radii = find_residue_max_radii( pose, task );

	return create_packer_graph( pose, scfxn, task, pose.total_residue(), residue_radii );
}

/// @brief Constructs a graph where edges represent the possibility of interactions between
/// residue pairs
graph::GraphOP
create_packer_graph(
	pose::Pose const & pose,
	scoring::ScoreFunction const & scfxn,
	task::PackerTaskCOP task,
	core::Size total_nodes,
	utility::vector1< Distance > const & residue_radii
)
{
	using namespace graph;

	//GraphOP g = new Graph( pose.total_residue() );
	GraphOP g = new Graph( total_nodes );

	if ( ! task->design_any() ) {
		g->copy_connectivity( pose.energies().energy_graph() );
	} else {

		// find radii for residues...   NOTE: flo oct 08, not anymore, doing this in above function now
		//utility::vector1< Distance > residue_radii = find_residue_max_radii( pose, task );

		// find max radius
		Distance const max_radius = utility::max( residue_radii );
		Distance const atomic_itxn_dist = scfxn.info()->max_atomic_interaction_distance();

		// create point graph and detect neighbors
		core::conformation::PointGraphOP point_graph( new core::conformation::PointGraph );
		core::conformation::residue_point_graph_from_conformation( pose.conformation(), *point_graph );
		core::conformation::find_neighbors<core::conformation::PointGraphVertexData,core::conformation::PointGraphEdgeData>( point_graph, atomic_itxn_dist + 2 * max_radius );

		// add edges
		//for ( Size ii = 1; ii <= pose.total_residue(); ++ii ) {
		for ( Size ii = 1; ii <= total_nodes; ++ii ) {
			Distance const ii_itxn_rad = residue_radii[ ii ] + atomic_itxn_dist;
			for ( core::conformation::PointGraph::UpperEdgeListConstIter
				iter = point_graph->get_vertex( ii ).const_upper_edge_list_begin(),
				iter_end = point_graph->get_vertex( ii ).const_upper_edge_list_end();
				iter != iter_end; ++iter ) {

				Size jj = iter->upper_vertex();
				Distance const jj_rad = residue_radii[ jj ];

				if ( jj_rad + ii_itxn_rad > 0 &&
						iter->data().dsq() < ( jj_rad + ii_itxn_rad )*( jj_rad + ii_itxn_rad ) ) {
					g->add_edge( ii, jj );
				}
			}
		}
	}
	return g;
}


/// @brief for each residue in the protein, finds the largest bounding sphere
/// over all allowable rotameric/chemical modifications possible given the input task.
///
utility::vector1< Distance >
find_residue_max_radii(
	pose::Pose const & pose,
	task::PackerTaskCOP the_task
)
{
	using namespace chemical;

	utility::vector1< Distance > residue_max_radii( pose.total_residue(), 0.0 );

	for ( Size ii = 1, ii_end = pose.total_residue(); ii <= ii_end; ++ii ) {
		Distance max_radius_for_res( 0.0 );
		/*
		if ( task->design_residue( ii ) ) {

			for ( ResidueTypeSet::AAsIter iter = residue_set.aas_defined_begin(),
				eiter = residue_set.aas_defined_end(); iter != eiter; ++iter ) {

				if ( task->allow_aa( ii, *iter )) {
					ResidueTypeCAPs const & concrete_residues( residue_set.aa_map( *iter ) );
					for ( ResidueTypeCAPs::const_iterator resiter = concrete_residues.begin(),
							eresiter = concrete_residues.end(); resiter != eresiter; ++resiter ) {
						if ( task->allow_concrete( pose.residue(ii), **resiter )) {
							if ( (*resiter)->nbr_radius() > max_radius_for_res ) {
								max_radius_for_res = (*resiter)->nbr_radius();
							}
						}
					}
				}
			}
		} else if ( task->pack_residue( ii ) ) {

			chemical::AA const resaa( pose.residue( ii ).aa());
			ResidueTypeCAPs const & concrete_residues( residue_set.aa_map( resaa ) );
			for ( ResidueTypeCAPs::const_iterator resiter = concrete_residues.begin(),
					eresiter = concrete_residues.end(); resiter != eresiter; ++resiter ) {
				if ( task->repacking_allow_concrete( pose.residue(ii), **resiter)) {
					if ( (*resiter)->nbr_radius() > max_radius_for_res ) {
						max_radius_for_res = (*resiter)->nbr_radius();
					}
				}
			}
		} */
		if ( the_task->pack_residue( ii ) ) {
			for ( task::ResidueLevelTask::ResidueTypeCAPListConstIter
					allowed_iter = the_task->residue_task( ii ).allowed_residue_types_begin(),
					allowed_end = the_task->residue_task( ii ).allowed_residue_types_end();
					allowed_iter != allowed_end; ++allowed_iter ) {
				if ((*allowed_iter)->nbr_radius() > max_radius_for_res ) {
					max_radius_for_res = (*allowed_iter)->nbr_radius();
				}
			}
			//check whether the radius at any position needs to be increased
			Distance max_rad_change(0.0);
			for ( rotamer_set::RotSetOperationListIterator
							rotsetop_iter = the_task->residue_task( ii ).rotamer_set_operation_begin(),
							rotsetop_end = the_task->residue_task( ii ).rotamer_set_operation_end();
						rotsetop_iter != rotsetop_end; ++rotsetop_iter ) {
				if( (*rotsetop_iter)->increase_packer_residue_radius( pose, the_task ) > max_rad_change ) {
					max_rad_change = (*rotsetop_iter)->increase_packer_residue_radius( pose, the_task );
				}
			}
			if( max_rad_change != 0.0 ) max_radius_for_res = max_radius_for_res + max_rad_change;
		} else {
			max_radius_for_res = pose.residue( ii ).nbr_radius();
		}
		residue_max_radii[ ii ] = max_radius_for_res;
	}
	return residue_max_radii;
}

/// @details pose and score function must have met before packing
/// may begin; this function will force a score evaluation if the
/// energie's scorefunction-info object does not match that of the
/// given score function.
void
pack_scorefxn_pose_handshake(
	pose::Pose & pose,
	scoring::ScoreFunction const & scfxn
)
{
	if ( pose.energies().get_scorefxn_info() != *(scfxn.info() ) ) {
		scfxn( pose );
	}
}

} // namespace core
} // namespace pack
