// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/etable/etrie/TrieCountPairAll.table_etable.cc
/// @brief  File which actually instantiates the templated trie-vs-trie and trie-vs-path
///         algorithms for the table-lookup etable energy
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com)

// Unit Headers
#include <core/scoring/etable/etrie/TrieCountPairAll.hh>

// Package Headers
#include <core/scoring/trie/trie_vs_trie.hh>
#include <core/scoring/trie/trie_vs_path.hh>
#include <core/scoring/etable/etrie/CountPairData_1_1.hh>
#include <core/scoring/etable/etrie/CountPairData_1_2.hh>
#include <core/scoring/etable/etrie/CountPairData_1_3.hh>
#include <core/scoring/etable/etrie/CountPairDataGeneric.hh>

// Project Headers
#include <core/scoring/etable/EtableEnergy.hh>

// STL Headers

#include <core/scoring/etable/etrie/EtableAtom.hh>
#include <core/scoring/trie/RotamerTrie.hh>
#include <utility/vector1.hh>


namespace core {
namespace scoring {
namespace etable {
namespace etrie {

using namespace trie;

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_trie(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	ObjexxFCL::FArray2D< core::PackerEnergy > & pair_energy_table,
	ObjexxFCL::FArray2D< core::PackerEnergy > & temp_table,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_trie( trie1, trie2, *this, sfxn, pair_energy_table, temp_table, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_1 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_2 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairData_1_3 > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}

void TrieCountPairAll::resolve_trie_vs_path(
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie1,
	trie::RotamerTrie< EtableAtom, CountPairDataGeneric > const & trie2,
	etable::TableLookupEvaluator const & sfxn,
	utility::vector1< core::PackerEnergy > & pair_energy_vector,
	utility::vector1< core::PackerEnergy > & temp_vector,
	TrieVsTrieCachedDataContainerBase const * const cached_data
)
{
	trie_vs_path( trie1, trie2, *this, sfxn, pair_energy_vector, temp_vector, cached_data );
}


} // namespace etrie
} // namespace etable
} // namespace scoring
} // namespace core

