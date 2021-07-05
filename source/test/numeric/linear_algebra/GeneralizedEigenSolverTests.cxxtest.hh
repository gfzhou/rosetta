// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   GeneralizedEigenSolverTests.cxxtest.hh
/// @brief  Unit test for GeneralizedEigenSolver.
/// @author Kale Kundert (kale.kundert@ucsf.edu)

// Test headers // {{{1
#include <cxxtest/TestSuite.h>

// Unit headers // {{{1
#include <numeric/linear_algebra/GeneralizedEigenSolver.hh>

// External headers // {{{1

#include <basic/Tracer.hh>

// C++ headers // {{{1
#include <iostream>
#include <complex>
#include <string>

static basic::Tracer TR("numeric.linear_algebra.GeneralizedEigenSolverTests.cxxtest");

// Namespaces // {{{1
using namespace std;
using numeric::linear_algebra::GeneralizedEigenSolver;
// }}}1

// Note that the test matrices used here were generated by LAPACK (via scipy),
// while the generalized eigenvalue problem solver itself is based on EISPACK.
// This is nice, because it means that the results are effectively being
// cross-validated by different algorithms.  However, this is also somewhat
// problematic because the two algorithms return very different eigenvectors.
// It was therefore necessary to validate all the eigenvectors used in this
// test by hand.  It's possible that I made mistakes while doing this.

class GeneralizedEigenSolverTests : public CxxTest::TestSuite {
public:

	void test_public_interface() { // {{{1
		Eigen::Matrix2d A, B;

		A << 1, 5, 6, 9;
		B << 9, 6, 7, 9;

		Eigen::Matrix2d A_clone(A), B_clone(B);

		GeneralizedEigenSolver<Eigen::Matrix2d> solver_1 =
			GeneralizedEigenSolver<Eigen::Matrix2d>();

		GeneralizedEigenSolver<Eigen::Matrix2d> solver_2 =
			GeneralizedEigenSolver<Eigen::Matrix2d>(2);

		GeneralizedEigenSolver<Eigen::Matrix2d> solver_3 =
			GeneralizedEigenSolver<Eigen::Matrix2d>();

		GeneralizedEigenSolver<Eigen::Matrix2d> solver_4 =
			GeneralizedEigenSolver<Eigen::Matrix2d>();

		GeneralizedEigenSolver<Eigen::Matrix2d> solver_5 =
			GeneralizedEigenSolver<Eigen::Matrix2d>(A, B);

		solver_1.compute(A, B);
		solver_2.compute(A, B);
		solver_3.compute(A, B, false);
		solver_4.compute_in_place(A_clone, B_clone);

		TS_ASSERT(solver_1.eigenvalues() == solver_2.eigenvalues())
			TS_ASSERT(solver_1.eigenvalues() == solver_3.eigenvalues())
			TS_ASSERT(solver_1.eigenvalues() == solver_4.eigenvalues())
			TS_ASSERT(solver_1.eigenvalues() == solver_5.eigenvalues())

			TS_ASSERT(!A.isApprox(A_clone))
			TS_ASSERT(!B.isApprox(B_clone))
	}

	void test_error_conditions() {
		Eigen::Matrix2d A, B;
		GeneralizedEigenSolver<Eigen::Matrix2d> solver;

		// I could not find any examples that should produce these error cases, so
		// as of yet these error remain untested.

		// TS_ASSERT_EQUALS(solver.info(), Eigen::NumericalIssue);
		// TS_ASSERT_EQUALS(solver.info(), Eigen::InvalidInput);
	}

	void test_2x2_real_problem() { // {{{1
		Eigen::Matrix2d A, B;
		GeneralizedEigenSolver<Eigen::Matrix2d> solver;

		A << 1, 5, 6, 9;
		B << 9, 6, 7, 9;

		solver.compute(A, B);

		TS_ASSERT_EQUALS(solver.num_real_solutions(), 2);

		check_real_eigenvalue(solver, 0, 0, 1e-8, -0.52958391);
		check_real_eigenvalue(solver, 1, 1, 1e-8,  1.01676340);

		// The complex eigenvalues compred against here are not exactly those
		// returned by scipy, because scipy does not normalize eigenvectors before
		// returning them (even though the documentation says it should).  The
		// expected values given here have been manually checked.

		check_real_eigenvector(solver, 0, 0, 1e-8,
			0.81725499,
			-0.57627622);

		check_real_eigenvector(solver, 1, 1, 1e-8,
			0.13381179,
			-0.99100676);
	}

	void test_2x2_complex_problem() { // {{{1
		Eigen::Matrix2d A, B;
		GeneralizedEigenSolver<Eigen::Matrix2d> solver;

		A << 4, 6, 2, 9;
		B << 3, 9, 0, 6;

		solver.compute(A, B);

		TS_ASSERT_EQUALS(solver.num_real_solutions(), 0);

		check_eigenvalue(solver, 0, 1e-8,  0.91666667,  0.70217915);
		check_eigenvalue(solver, 1, 1e-8,  0.91666667, -0.70217915);

		// The complex eigenvectors calculated by RGG are different from those
		// calculated by scipy, but they are still eigenvectors.  The expected
		// values given below have been manually checked.

		check_eigenvector(solver, 0, 1e-8,
			-0.72253638, -0.60024505,
			0.00000000,  0.34299717);

		check_eigenvector(solver, 1, 1e-8,
			-0.72253638,  0.60024505,
			0.00000000, -0.34299717);
	}

	void test_4x4_real_problem() { // {{{1
		Eigen::Matrix4d A, B;
		GeneralizedEigenSolver<Eigen::Matrix4d> solver;

		A << 7,  0,  3,  4,
			0,  6,  0,  3,
			2,  5,  7,  9,
			6,  5,  5,  8;

		B << 8,  3,  8,  3,
			4,  5,  8,  2,
			4,  4,  1,  4,
			2,  7,  9,  9;

		solver.compute(A, B);

		TS_ASSERT_EQUALS(solver.num_real_solutions(), 4);

		check_real_eigenvalue(solver, 0, 0, 1e-8, -0.52276009);
		check_real_eigenvalue(solver, 1, 1, 1e-8, -0.00718568);
		check_real_eigenvalue(solver, 2, 2, 1e-8,  1.95977768);
		check_real_eigenvalue(solver, 3, 3, 1e-8,  1.36065223);

		// These eigenvectors have been manually checked.
		// See the comments in the 2x2 tests.

		check_real_eigenvector(solver, 0, 0, 1e-8,
			0.35536422,
			0.16847057,
			-0.85405004,
			0.34048858);

		check_real_eigenvector(solver, 1, 1, 1e-8,
			0.12963370,
			0.34194366,
			0.61603810,
			-0.69768667);

		check_real_eigenvector(solver, 2, 2, 1e-8,
			-0.36128099,
			0.59102217,
			0.07947736,
			-0.71683484);

		check_real_eigenvector(solver, 3, 3, 1e-8,
			-0.50094817,
			0.05939368,
			0.22474655,
			-0.83367398);
	}

	void test_4x4_complex_problem() { // {{{1
		Eigen::Matrix4d A, B;
		GeneralizedEigenSolver<Eigen::Matrix4d> solver;

		A << 6,  1,  0,  3,
			2,  0,  7,  5,
			3,  0,  3,  2,
			1,  8,  6,  4;

		B << 2,  9,  1,  2,
			9,  7,  0,  5,
			5,  6,  7,  7,
			1,  1,  0,  8;

		solver.compute(A, B);

		TS_ASSERT_EQUALS(solver.num_real_solutions(), 0);

		check_eigenvalue(solver, 0, 1e-8, -0.47086912,  0.41549550);
		check_eigenvalue(solver, 1, 1e-8, -0.47086912, -0.41549550);
		check_eigenvalue(solver, 2, 1e-8,  0.30589602,  0.46842584);
		check_eigenvalue(solver, 3, 1e-8,  0.30589602, -0.46842584);

		// These eigenvectors have been manually checked.
		// See the comments in the 2x2 tests.
		//
		// The epsilon used here (1e-6) is less stringent because of release-mode tests

		check_eigenvector(solver, 0, 1e-6,
			-0.08639801,  0.36148133,
			0.43085111,  0.10570383,
			0.35560457,  0.09872071,
			-0.54925363, -0.47663478);

		check_eigenvector(solver, 1, 1e-6,
			-0.08639801, -0.36148133,
			0.43085111, -0.10570383,
			0.35560457, -0.09872071,
			-0.54925363,  0.47663478);

		check_eigenvector(solver, 2, 1e-6,
			0.41530214, -0.22931348,
			-0.25794209, -0.16537696,
			0.51372334, -0.12748241,
			-0.63037770, -0.05929858);

		check_eigenvector(solver, 3, 1e-6,
			0.41530214,  0.22931348,
			-0.25794209,  0.16537696,
			0.51372334,  0.12748241,
			-0.63037770,  0.05929858);
	}

	void test_4x4_mixed_problem() { // {{{1
		Eigen::Matrix4d A, B;
		GeneralizedEigenSolver<Eigen::Matrix4d> solver;

		A << 7,  2,  1,  4,
			2,  8,  4,  0,
			1,  4,  0,  5,
			2,  4,  9,  6;

		B << 8,  2,  4,  7,
			5,  3,  3,  4,
			1,  7,  8,  0,
			6,  8,  8,  7;

		solver.compute(A, B);

		TS_ASSERT_EQUALS(solver.num_real_solutions(), 2);

		check_eigenvalue(solver, 0, 1e-8,  2.29818042,  1.95081854);
		check_eigenvalue(solver, 1, 1e-8,  2.29818042, -1.95081854);
		check_real_eigenvalue(solver, 2, 0, 1e-8, -1.70728508);
		check_real_eigenvalue(solver, 3, 1, 1e-8,  0.69728787);

		// These eigenvectors have been manually checked.
		// See the comments in the 2x2 tests.

		check_eigenvector(solver, 0, 1e-8,
			0.27103127, -0.63244745,
			0.29561706,  0.051458117,
			-0.23076585,  0.10505648,
			-0.28523215,  0.53932157);

		check_eigenvector(solver, 1, 1e-8,
			0.27103127,  0.63244745,
			0.29561706, -0.051458117,
			-0.23076585, -0.10505648,
			-0.28523215, -0.53932157);

		check_real_eigenvector(solver, 2, 0, 1e-8,
			-0.35386993,
			0.37116447,
			-0.59120531,
			0.62248638);

		check_real_eigenvector(solver, 3, 1, 1e-8,
			0.71844648,
			0.26942740,
			0.42604584,
			0.47929999);

	}
	// }}}1

	// void show_matrices(     // NxN {{{1
	template <typename SolverType>
	void show_matrices(
		typename SolverType::MatrixType const & A,
		typename SolverType::MatrixType const & B,
		SolverType const & solver) {

		Eigen::IOFormat scipy(8, 0, ", ", ",\n", "[", "]", "[", "]");
		string skipl = "\n\n";

		TR << "A = \\" << endl << A.format(scipy) << skipl;
		TR << "B = \\" << endl << B.format(scipy) << skipl;
		TR << "l = \\" << endl << solver.eigenvalues().format(scipy) << skipl;
		TR << "x = \\" << endl << solver.eigenvectors().format(scipy) << skipl;
	}

	// void check_eigenvalue(  // NxN {{{1
	template <typename SolverType>
	void check_eigenvalue(
		SolverType const & solver,
		int index, double epsilon,
		double expected_real, double expected_imag) {

		// Use check_real_eigenvalue() for real eigenvalues.
		TS_ASSERT(expected_imag);

		// Make sure the solver reported success.
		TS_ASSERT_EQUALS(solver.info(), Eigen::Success);

		complex<double> actual = solver.eigenvalues()(index);
		TS_ASSERT_DELTA(actual.real(), expected_real, epsilon);
		TS_ASSERT_DELTA(actual.imag(), expected_imag, epsilon);


	}

	void check_eigenvector( // 2x2 {{{1
		GeneralizedEigenSolver<Eigen::Matrix2d> const & solver,
		int index, double epsilon,
		double expected_1_real, double expected_1_imag,
		double expected_2_real, double expected_2_imag) {

		// Use check_real_eigenvector() for real eigenvectors.
		TS_ASSERT(expected_1_imag);
		TS_ASSERT(expected_2_imag);

		complex<double> actual_1 = solver.eigenvectors()(0, index);
		complex<double> actual_2 = solver.eigenvectors()(1, index);

		TS_ASSERT_DELTA(actual_1.real(), expected_1_real, epsilon);
		TS_ASSERT_DELTA(actual_1.imag(), expected_1_imag, epsilon);
		TS_ASSERT_DELTA(actual_2.real(), expected_2_real, epsilon);
		TS_ASSERT_DELTA(actual_2.imag(), expected_2_imag, epsilon);
	}

	void check_eigenvector( // 4x4 {{{1
		GeneralizedEigenSolver<Eigen::Matrix4d> const & solver,
		int index, double epsilon,
		double expected_1_real, double expected_1_imag,
		double expected_2_real, double expected_2_imag,
		double expected_3_real, double expected_3_imag,
		double expected_4_real, double expected_4_imag) {

		// Use check_real_eigenvector() for real eigenvectors.
		TS_ASSERT(expected_1_imag);
		TS_ASSERT(expected_2_imag);
		TS_ASSERT(expected_3_imag);
		TS_ASSERT(expected_4_imag);

		complex<double> actual_1 = solver.eigenvectors()(0, index);
		complex<double> actual_2 = solver.eigenvectors()(1, index);
		complex<double> actual_3 = solver.eigenvectors()(2, index);
		complex<double> actual_4 = solver.eigenvectors()(3, index);

		TS_ASSERT_DELTA(actual_1.real(), expected_1_real, epsilon);
		TS_ASSERT_DELTA(actual_1.imag(), expected_1_imag, epsilon);
		TS_ASSERT_DELTA(actual_2.real(), expected_2_real, epsilon);
		TS_ASSERT_DELTA(actual_2.imag(), expected_2_imag, epsilon);
		TS_ASSERT_DELTA(actual_3.real(), expected_3_real, epsilon);
		TS_ASSERT_DELTA(actual_3.imag(), expected_3_imag, epsilon);
		TS_ASSERT_DELTA(actual_4.real(), expected_4_real, epsilon);
		TS_ASSERT_DELTA(actual_4.imag(), expected_4_imag, epsilon);
	}

	/// }}}1
	// void check_real_eigenvalue(  // NxN {{{1
	template <typename SolverType>
	void check_real_eigenvalue(
		SolverType const & solver,
		int index, int real_index, double epsilon, double expected) {

		// Make sure the solver reported success.
		TS_ASSERT_EQUALS(solver.info(), Eigen::Success);

		double actual = solver.eigenvalues()(index).real();
		double actual_from_reals = solver.real_eigenvalues()(real_index);

		TS_ASSERT_DELTA(actual, expected, epsilon);
		TS_ASSERT_DELTA(actual_from_reals, expected, epsilon);
	}

	void check_real_eigenvector( // 2x2 {{{1
		GeneralizedEigenSolver<Eigen::Matrix2d> const & solver,
		int index, int real_index, double epsilon,
		double expected_1, double expected_2) {

		double actual_1 = solver.eigenvectors()(0, index).real();
		double actual_2 = solver.eigenvectors()(1, index).real();
		double actual_from_reals_1 = solver.real_eigenvectors()(0, real_index);
		double actual_from_reals_2 = solver.real_eigenvectors()(1, real_index);

		TS_ASSERT_DELTA(actual_1, expected_1, epsilon);
		TS_ASSERT_DELTA(actual_2, expected_2, epsilon);
		TS_ASSERT_DELTA(actual_from_reals_1, expected_1, epsilon);
		TS_ASSERT_DELTA(actual_from_reals_2, expected_2, epsilon);
	}

	void check_real_eigenvector( // 4x4 {{{1
		GeneralizedEigenSolver<Eigen::Matrix4d> const & solver,
		int index, int real_index, double epsilon,
		double expected_1, double expected_2,
		double expected_3, double expected_4) {

		double actual_1 = solver.eigenvectors()(0, index).real();
		double actual_2 = solver.eigenvectors()(1, index).real();
		double actual_3 = solver.eigenvectors()(2, index).real();
		double actual_4 = solver.eigenvectors()(3, index).real();

		double actual_from_reals_1 = solver.real_eigenvectors()(0, real_index);
		double actual_from_reals_2 = solver.real_eigenvectors()(1, real_index);
		double actual_from_reals_3 = solver.real_eigenvectors()(2, real_index);
		double actual_from_reals_4 = solver.real_eigenvectors()(3, real_index);

		TS_ASSERT_DELTA(actual_1, expected_1, epsilon);
		TS_ASSERT_DELTA(actual_2, expected_2, epsilon);
		TS_ASSERT_DELTA(actual_3, expected_3, epsilon);
		TS_ASSERT_DELTA(actual_4, expected_4, epsilon);

		TS_ASSERT_DELTA(actual_from_reals_1, expected_1, epsilon);
		TS_ASSERT_DELTA(actual_from_reals_2, expected_2, epsilon);
		TS_ASSERT_DELTA(actual_from_reals_3, expected_3, epsilon);
		TS_ASSERT_DELTA(actual_from_reals_4, expected_4, epsilon);
	}

	/// }}}1

};
