// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
// :noTabs=false:tabSize=4:indentSize=4:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file   rosetta/benchmark/Design.bench.hh
///
/// @brief Perform a complete redesign of 1A30 (HIV protease/inhibitor).
///  (99 res dimer bound to 3 res. peptide= 201 residue)
/// takes about 1 minute on my machine
/// @author Gordon Lemmon

#include <protocols/moves/PackRotamersMover.hh>

#include <core/pose/Pose.hh>
#include <core/import_pose/import_pose.hh>

//#include <core/pack/task/TaskFactory.hh>
//#include <core/pack/task/operation/TaskOperations.hh>

// AUTO-REMOVED #include <core/scoring/ScoreFunction.hh>

#include <apps/benchmark/benchmark.hh>
// AUTO-REMOVED #include <apps/benchmark/init_util.hh>

//Auto Headers
#include <utility/vector0.hh>
#include <utility/vector1.hh>
//#include <platform/types.hh>
//#include <core/types.hh>
//#include <core/chemical/AA.hh>
//#include <core/chemical/ResidueType.fwd.hh>
//#include <core/chemical/ResidueTypeSet.fwd.hh>
//#include <core/conformation/Atom.fwd.hh>
//#include <core/conformation/Conformation.fwd.hh>
//#include <core/conformation/Residue.fwd.hh>
//#include <core/conformation/signals/XYZEvent.fwd.hh>
//#include <core/id/AtomID.fwd.hh>
//#include <core/id/AtomID.hh>
//#include <core/id/AtomID_Map.fwd.hh>
//#include <core/id/AtomID_Mask.fwd.hh>
//#include <core/id/DOF_ID.fwd.hh>
//#include <core/id/DOF_ID.hh>
//#include <core/id/NamedAtomID.fwd.hh>
//#include <core/id/NamedAtomID.hh>
//#include <core/id/NamedStubID.fwd.hh>
//#include <core/id/NamedStubID.hh>
//#include <core/id/TorsionID.fwd.hh>
//#include <core/id/types.hh>
//#include <core/import_pose/file_data.fwd.hh>
//#include <core/io/pdb/file_data.hh>
//#include <core/kinematics/AtomTree.fwd.hh>
//#include <core/kinematics/DomainMap.fwd.hh>
//#include <core/kinematics/FoldTree.fwd.hh>
//#include <core/kinematics/Jump.fwd.hh>
//#include <core/kinematics/Stub.fwd.hh>
//#include <core/optimization/MinimizerMap.fwd.hh>

//#include <core/pack/interaction_graph/InteractionGraphBase.fwd.hh>
//#include <core/pack/rotamer_set/RotamerSet.fwd.hh>
//#include <core/pack/rotamer_set/RotamerSets.fwd.hh>
//#include <core/pack/task/PackerTask.fwd.hh>
//#include <core/pack/task/TaskFactory.fwd.hh>
//#include <core/pose/PDBInfo.fwd.hh>
//#include <core/pose/Pose.fwd.hh>
//#include <core/pose/datacache/ObserverCache.fwd.hh>
//#include <core/pose/metrics/PoseMetricContainer.fwd.hh>
//#include <core/pose/signals/ConformationEvent.fwd.hh>
//#include <core/pose/signals/ConformationEvent.hh>
//#include <core/pose/signals/DestructionEvent.fwd.hh>
//#include <core/pose/signals/DestructionEvent.hh>
//#include <core/pose/signals/EnergyEvent.fwd.hh>
//#include <core/pose/signals/EnergyEvent.hh>
//#include <core/pose/signals/GeneralEvent.fwd.hh>
//#include <core/pose/signals/GeneralEvent.hh>
//#include <core/scoring/Energies.fwd.hh>
//#include <core/scoring/EnergyMap.fwd.hh>
//#include <core/scoring/EnergyMap.hh>
//#include <core/scoring/ScoreFunction.fwd.hh>
//#include <core/scoring/ScoreFunctionInfo.fwd.hh>
//#include <core/scoring/ScoreType.hh>
//#include <core/scoring/types.hh>
//#include <core/scoring/constraints/Constraint.fwd.hh>
//#include <core/scoring/constraints/ConstraintSet.fwd.hh>
//#include <core/scoring/constraints/Constraints.fwd.hh>
//#include <core/scoring/methods/ContextDependentLRTwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/ContextDependentOneBodyEnergy.fwd.hh>
//#include <core/scoring/methods/ContextDependentTwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/ContextIndependentLRTwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/ContextIndependentOneBodyEnergy.fwd.hh>
//#include <core/scoring/methods/ContextIndependentTwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/EnergyMethod.fwd.hh>
//#include <core/scoring/methods/EnergyMethodOptions.fwd.hh>
//#include <core/scoring/methods/LongRangeTwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/TwoBodyEnergy.fwd.hh>
//#include <core/scoring/methods/WholeStructureEnergy.fwd.hh>
//#include <basic/MetricValue.fwd.hh>
//// AUTO-REMOVED #include <basic/OStream.fwd.hh>
//#include <utility/stream_util.hh>
//#include <basic/Tracer.fwd.hh>
//#include <basic/Tracer.hh>
//#include <basic/datacache/BasicDataCache.fwd.hh>
//#include <protocols/filters/Filter.fwd.hh>
//#include <protocols/moves/DataMap.fwd.hh>
//#include <protocols/moves/MonteCarlo.fwd.hh>
//#include <protocols/moves/Mover.fwd.hh>
//#include <protocols/moves/Mover.hh>
//#include <protocols/moves/MoverStatistics.hh>
//#include <protocols/moves/MoverStatus.hh>
//#include <protocols/moves/PackRotamersMover.fwd.hh>
//#include <utility/down_cast.hh>
//#include <utility/exit.hh>
//#include <utility/vector1.fwd.hh>
//#include <utility/vector1.hh>
//#include <utility/vector1_bool.hh>
//#include <utility/vectorL.fwd.hh>
//#include <utility/vectorL.hh>
//#include <utility/vectorL_Selector.hh>
//#include <utility/vectorL_bool.hh>
//#include <utility/tag/Tag.fwd.hh>
//#include <utility/pointer/ReferenceCount.fwd.hh>
//#include <utility/pointer/ReferenceCount.hh>
//#include <utility/pointer/access_ptr.fwd.hh>
//#include <utility/pointer/access_ptr.hh>
//#include <utility/pointer/owning_ptr.functions.hh>
//#include <utility/pointer/owning_ptr.fwd.hh>
//#include <utility/pointer/owning_ptr.hh>
//#include <utility/signals/BufferedSignalHub.fwd.hh>
//#include <utility/signals/BufferedSignalHub.hh>
//#include <utility/signals/Link.fwd.hh>
//#include <utility/signals/Link.hh>
//#include <utility/signals/LinkUnit.fwd.hh>
//#include <utility/signals/LinkUnit.hh>
//#include <utility/signals/SignalHub.fwd.hh>
//#include <utility/signals/SignalHub.hh>
//#include <numeric/numeric.functions.hh>
//#include <numeric/trig.functions.hh>
//#include <numeric/xyz.functions.fwd.hh>
//#include <numeric/xyzMatrix.fwd.hh>
//#include <numeric/xyzVector.fwd.hh>
//#include <numeric/xyzVector.hh>
//#include <ObjexxFCL/CArray.fwd.hh>
//#include <ObjexxFCL/CArrayP.fwd.hh>
//#include <ObjexxFCL/FArray.all.fwd.hh>
//#include <ObjexxFCL/FArray1D.fwd.hh>
//#include <ObjexxFCL/FArray1.fwd.hh>
//#include <ObjexxFCL/FArray1A.fwd.hh>
//#include <ObjexxFCL/FArray1P.fwd.hh>
//#include <ObjexxFCL/FArray1.all.fwd.hh>
//#include <ObjexxFCL/FArray2D.fwd.hh>
//#include <ObjexxFCL/FArray2.fwd.hh>
//#include <ObjexxFCL/FArray2A.fwd.hh>
//#include <ObjexxFCL/FArray2P.fwd.hh>
//#include <ObjexxFCL/FArray2.all.fwd.hh>
//#include <ObjexxFCL/FArray3D.fwd.hh>
//#include <ObjexxFCL/FArray3.fwd.hh>
//#include <ObjexxFCL/FArray3A.fwd.hh>
//#include <ObjexxFCL/FArray3P.fwd.hh>
//#include <ObjexxFCL/FArray3.all.fwd.hh>
//#include <ObjexxFCL/FArray4D.fwd.hh>
//#include <ObjexxFCL/FArray4.fwd.hh>
//#include <ObjexxFCL/FArray4A.fwd.hh>
//#include <ObjexxFCL/FArray4P.fwd.hh>
//#include <ObjexxFCL/FArray4.all.fwd.hh>
//#include <ObjexxFCL/FArray5D.fwd.hh>
//#include <ObjexxFCL/FArray5.fwd.hh>
//#include <ObjexxFCL/FArray5A.fwd.hh>
//#include <ObjexxFCL/FArray5P.fwd.hh>
//#include <ObjexxFCL/FArray5.all.fwd.hh>
//#include <ObjexxFCL/KeyFArray1D.fwd.hh>
//#include <ObjexxFCL/KeyFArray2D.fwd.hh>
//#include <ObjexxFCL/KeyFArray3D.fwd.hh>
//#include <ObjexxFCL/format.hh>
//#include <algorithm>
//#include <cassert>
//#include <cmath>
//#include <complex>
//#include <cstddef>
//#include <cstdlib>
//#include <iomanip>
//#include <iosfwd>
//#include <iostream>
//#include <limits>
//#include <list>
//#include <map>
//#include <ostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <boost/bind.hpp>
//#include <boost/function.hpp>
//#include <boost/shared_ptr.hpp>


class DesignBenchmark : public Benchmark
{
public:
	DesignBenchmark(std::string name) : Benchmark(name) {};

	protocols::moves::PackRotamersMover pack_mover;
	core::pose::Pose design_pose;

	virtual void setUp() {
		//core_init_with_additional_options( "-ex1" );// I can't get this to work
		core::import_pose::pose_from_pdb(design_pose, "design_in.pdb");
	};

	virtual void run(core::Real scaleFactor) {
		for(int i=0; i<scaleFactor; i++) {
			pack_mover.apply(design_pose);
		}
		//design_pose.dump_scored_pdb("design_out.pdb", *pack_mover.scorefxn());// write out for debug
	};

	virtual void tearDown() {};
};
