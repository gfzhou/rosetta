// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file SequenceAlignment.hh
/// @brief class definition for a multiple sequence alignment
/// @author James Thompson

#ifndef INCLUDED_core_sequence_SequenceAlignment_hh
#define INCLUDED_core_sequence_SequenceAlignment_hh

// C/C++ headers
// AUTO-REMOVED #include <functional>
// AUTO-REMOVED #include <iostream>
#include <map>
#include <string>

// External headers
#include <boost/functional/hash.hpp>

// Utility headers
#include <utility/vector1_bool.hh>
#include <utility/pointer/ReferenceCount.hh>

// Project headers
#include <core/types.hh>
#include <core/id/SequenceMapping.hh>

// Package headers
#include <core/sequence/Sequence.hh>
#include <core/sequence/ScoringScheme.fwd.hh>
#include <core/sequence/SequenceAlignment.fwd.hh>

#include <utility/vector1.hh>


namespace core {
namespace sequence {

class SequenceAlignment : public utility::pointer::ReferenceCount {
public:
	/// @brief ctor
	SequenceAlignment() {}

	/// @brief dtor
	//@brief Autogenerated destructor definition
	virtual ~SequenceAlignment();

//	~SequenceAlignment() {
// 		clear();
// 	}

	SequenceAlignmentOP clone() const {
		return new SequenceAlignment( *this );
	}

	void clear() {
		comments_.clear();
		sequences_.clear();
	}

	void add_sequence( SequenceOP myseq );

	/// @brief Returns the number of sequences.
	Size size() const;

	/// @brief Returns the length of all sequences in this SequenceAlignment.
	Size length() const;

	SequenceOP sequence( Size idx ) const;

	std::string to_string() const;
	void read_data( std::istream & in );

	/// @brief Accessors for the score of this SequenceAlignment.
	void score( Real const & sc );
	Real score() const;
	Real score( std::string const & name ) const;
	void score( std::string const & name, Real const value );
	std::map< std::string, core::Real > scores() const;
	void scores( std::map< std::string, core::Real > new_scores );

	/// @brief initialize this SequenceAlignment object from the
	/// given file.
	void read_from_file( std::string const & filename );

	/// @brief returns a SequenceMapping of the sequence at index
	/// idx1 mapped to the sequence at idx2.
	core::id::SequenceMapping sequence_mapping(
		core::Size const idx1,
		core::Size const idx2
	) const;

	/// @brief Removes positions that are gapped in all
	/// columns of the alignment.
	void remove_gapped_positions();

	/// @brief calculates the score of this alignment under
	/// the given ScoringScheme by summing up all of pairwise
	/// alignment scores between all pairs of sequence.
	Real calculate_score_sum_of_pairs( ScoringSchemeOP ss ) const;

	/// @brief calculates pairwise scores similar to
	/// calculate_score_sum_of_pairs.
	utility::vector1< Real >
	calculate_per_position_scores( ScoringSchemeOP ss ) const;

	/// @brief Returns true if all of the sequences are
	/// "in-frame," which means that residue x in the first
	/// sequence maps to residue x in all other sequences.
	/// This method intentionally ignores any sequence at the
	/// position where that sequence has a gap.
	bool sequences_are_in_frame() const;

	/// @brief Returns a vector1 of the sequence indices in
	/// the given column of the SequenceAlignment that are
	/// calculated using the Sequence resnum() method.
	utility::vector1< core::Size > sequence_indices(
		core::Size const column
	) const;

	/// @brief getter/setters for comments
	void comment( std::string const & comment );
	utility::vector1< std::string > comments() const;

	void data_integrity_check() const;

	/// @brief Returns the number of positions at which all
	// of the characters are the same, ignoring gaps.
	Size identities() const;

	/// @brief Returns the number of positions in the
	/// SequenceAlignment with at least one gap.
	Size gapped_positions() const;

	/// @brief Returns the number of gaps contained by
	/// the sequence in the alignment with the most gaps.
	Real max_gap_percentage() const;

	/// @brief Returns true if the given alignment column index is gapped,
	/// false otherwise.
	bool is_gapped( Size const col_idx ) const;

	/// @brief this gives the id of the second sequence
	/// --- in a typical alignment it is 1) query, 2) template
	std::string alignment_id() const;

	/// @brief copy constructor
	SequenceAlignment( SequenceAlignment const & src );

	/// @brief assignment operator
	SequenceAlignment &
	operator=( SequenceAlignment const & src );

  /// @brief Equality operator
  bool operator==(const SequenceAlignment& other) const {
    if ( alignment_id() != other.alignment_id() ) return false;
		if ( size() != other.size() ) return false;

		using core::Size;
		for ( Size ii = 1, end = size(); ii <= end; ++ii ) {
			if ( ! (*sequence(ii) == *other.sequence(ii)) ) return false;
		}

    return sequence_mapping(1,2) == other.sequence_mapping(1,2);
  }

	/// @brief Less-than operator. Usd for storing SequenceAlignment objects in a
	/// std::set.
	friend bool
	operator<(
		SequenceAlignment const & lhs,
		SequenceAlignment const & rhs
	);

public: // output operators

	/// @brief Print SequenceAlignment object to the given
	/// std::ostream.
	friend std::ostream & operator << (
		std::ostream & out,
		const SequenceAlignment & sa
	);

	friend std::istream & operator>>(
		std::istream & in,
		SequenceAlignment & aln
	);

	void printGrishinFormat (
		std::ostream & out
	) const;

private:
	utility::vector1< SequenceOP > sequences_;
	utility::vector1< std::string > comments_;
	std::map< std::string, core::Real > scores_;
}; // class SequenceAlignment


/// @class Computes a hash value for a SequenceAlignment based on its sequences.
/// Allows SequenceAlignment's to be used as keys in unordered_{set|map}.
struct SequenceAlignmentHasher : std::unary_function<SequenceAlignment, size_t> {
  std::size_t operator()(const SequenceAlignment& alignment) const {
    std::size_t h = 0;
    for (core::Size i = 1; i <= alignment.size(); ++i)
      boost::hash_combine(h, alignment.sequence(i)->sequence());

    return h;
  }
};

} // sequence
} // core

#endif
