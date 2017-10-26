/***********************************************************************************
Copyright (c) 2017, Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo,
Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of ETH ZURICH nor the names of its contributors may be used
      to endorse or promote products derived from this software without specific
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL ETH ZURICH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

#pragma once

#include <gtest/gtest.h>
#include "../../../include/ct/rbd/robot/costfunction/TermTaskspacePosition.hpp"
#include "../../models/testhyq/RobCoGenTestHyQ.h"


using namespace ct::core;
using namespace ct::rbd;

bool verbose = true;


//! some global variables required for the TaskSpaceTerm test
const size_t hyqStateDim = 36;
const size_t hyqControlDim = 12;
using size_type = ct::core::ADCGScalar;
using KinTpl_t = TestHyQ::tpl::Kinematics<size_type>;
KinTpl_t kynTpl;
size_t eeId = 1;
const Eigen::Matrix<double, 3, 3> Q = Eigen::Matrix<double, 3, 3>::Random();
std::shared_ptr<TermTaskspacePosition<KinTpl_t, true, hyqStateDim, hyqControlDim>> termTaskspace(
    new TermTaskspacePosition<KinTpl_t, true, hyqStateDim, hyqControlDim>(eeId, Q));


//! test the TaskSpacePosition term for JIT-compatibility
template <typename SCALAR>
Eigen::Matrix<SCALAR, 1, 1> testFunctionTaskSpacePosition(
    const Eigen::Matrix<SCALAR, hyqStateDim + hyqControlDim, 1> xu)
{
    // evaluate the task-space position term
    Eigen::Matrix<SCALAR, 1, 1> cost;
    SCALAR t(0.0);
    cost(0, 0) = termTaskspace->evaluateCppadCg(xu.template head<hyqStateDim>(), xu.template tail<hyqControlDim>(), t);
    return cost;
}

TEST(RBD_JIT_Tests, TaskspaceCostFunctionTest)
{
    using derivativesCppadJIT = DerivativesCppadJIT<hyqStateDim + hyqControlDim, 1>;

    typename derivativesCppadJIT::FUN_TYPE_CG f = testFunctionTaskSpacePosition<ct::core::ADCGScalar>;

    derivativesCppadJIT jacCG(f);

    DerivativesCppadSettings settings;
    settings.createJacobian_ = true;

    try
    {
        // compile the Jacobian
        jacCG.compileJIT(settings, "taskSpaceCfTestLib", verbose);
        std::cout << "testTaskSpacePositionTerm compiled!" << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "testTaskSpacePositionTerm failed!" << std::endl;
        ASSERT_TRUE(false);
    }
}


//! test the RigidBodyPose class for JIT-compatibility
template <typename SCALAR>
Eigen::Matrix<SCALAR, 1, 1> testFunctionRBDPose(const Eigen::Matrix<SCALAR, 3 + 3, 1>& xu)
{
    // construct a RigidBodyPose from state vector
    kindr::Position<SCALAR, 3> pos(xu.template segment<3>(0));
    kindr::EulerAnglesXyz<SCALAR> euler(xu.template segment<3>(3));

    ct::rbd::tpl::RigidBodyPose<SCALAR> rbdPose;
    rbdPose.position() = pos;
    rbdPose.setFromEulerAnglesXyz(euler);

    // we make up some "artificial", non-physical quantity to be auto-diffed:
    Eigen::Matrix<SCALAR, 1, 1> cost;
    SCALAR t(0.0);

    cost(0, 0) = rbdPose.position().toImplementation().norm() + rbdPose.getEulerAnglesXyz().toImplementation().norm();

    return cost;
}

TEST(RBD_JIT_Tests, RigidBodyPoseTest)
{
    using derivativesCppadJIT = DerivativesCppadJIT<3 + 3, 1>;

    typename derivativesCppadJIT::FUN_TYPE_CG f = testFunctionRBDPose<ct::core::ADCGScalar>;

    derivativesCppadJIT jacCG(f);

    DerivativesCppadSettings settings;
    settings.createJacobian_ = true;

    try
    {
        // compile the Jacobian
        jacCG.compileJIT(settings, "rbdPoseTestLib", verbose);
        std::cout << "testRBDPose compiled!" << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "testRBDPose failed!" << std::endl;
        ASSERT_TRUE(false);
    }
}


//! test the RigidBodyState class for JIT-compatibility
template <typename SCALAR>
Eigen::Matrix<SCALAR, 1, 1> testFunctionRigidBodyState(const Eigen::Matrix<SCALAR, 12, 1>& state)
{
    // construct a RigidBodyState from state vector
    ct::rbd::tpl::RigidBodyState<SCALAR> rigidBodyState(
        ct::rbd::tpl::RigidBodyPose<SCALAR>::EULER);  //<- storage type needs to go here in order to let test pass

    rigidBodyState.velocities().getRotationalVelocity().toImplementation() = state.template segment<3>(6);
    rigidBodyState.velocities().getTranslationalVelocity().toImplementation() = state.template segment<3>(9);

    kindr::Position<SCALAR, 3> pos(state.template segment<3>(0));
    kindr::EulerAnglesXyz<SCALAR> euler(state.template segment<3>(3));

    rigidBodyState.pose().position() = pos;
    rigidBodyState.pose().setFromEulerAnglesXyz(euler);

    // helps to check the copy assignment
    ct::rbd::tpl::RigidBodyState<SCALAR> rigidBodyStateCopy = rigidBodyState;

    // we make up some "artificial", non-physical quantity to be auto-diffed:
    Eigen::Matrix<SCALAR, 1, 1> cost;
    SCALAR t(0.0);

    cost(0, 0) = rigidBodyStateCopy.pose().position().toImplementation().norm() +
                 rigidBodyStateCopy.pose().getEulerAnglesXyz().toImplementation().norm() +
                 rigidBodyStateCopy.velocities().getVector().norm();

    return cost;
}

TEST(RBD_JIT_Tests, RigidBodyStateTest)
{
    using derivativesCppadJIT = DerivativesCppadJIT<12, 1>;

    typename derivativesCppadJIT::FUN_TYPE_CG f = testFunctionRigidBodyState<ct::core::ADCGScalar>;

    derivativesCppadJIT jacCG(f);

    DerivativesCppadSettings settings;
    settings.createJacobian_ = true;

    try
    {
        // compile the Jacobian
        jacCG.compileJIT(settings, "rbdStateTestLib", verbose);
        std::cout << "testRigidBodyState compiled!" << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "testRigidBodyState failed!" << std::endl;
        ASSERT_TRUE(false);
    }
}


//! test the RBDState class for JIT-compatibility
template <typename SCALAR>
Eigen::Matrix<SCALAR, 1, 1> testFunctionRBDState(const Eigen::Matrix<SCALAR, hyqStateDim, 1>& x)
{
    // construct an RBDState from data in the state vector.
    ct::rbd::tpl::RigidBodyState<SCALAR> baseState(
        ct::rbd::tpl::RigidBodyPose<SCALAR>::EULER);  //<- storage type needs to go here in order to let test pass

    baseState.velocities().getRotationalVelocity().toImplementation() = x.template segment<3>(6);
    baseState.velocities().getTranslationalVelocity().toImplementation() = x.template segment<3>(9);

    kindr::Position<SCALAR, 3> pos(x.template segment<3>(0));
    kindr::EulerAnglesXyz<SCALAR> euler(x.template segment<3>(3));
    baseState.pose().position() = pos;
    baseState.pose().setFromEulerAnglesXyz(euler);

    ct::rbd::tpl::JointState<12, SCALAR> jointState(x.template tail<24>());

    ct::rbd::tpl::RBDState<12, SCALAR> rbdState;
    rbdState.base() = baseState;
    rbdState.joints() = jointState;

    // we make up some "artificial", non-physical quantity to be auto-diffed:
    Eigen::Matrix<SCALAR, 1, 1> cost;
    SCALAR t(0.0);

    ct::rbd::tpl::RBDState<12, SCALAR> rbdStateCopy = rbdState;

    cost(0, 0) = rbdStateCopy.toStateVectorEulerXyz().norm();

    return cost;
}

TEST(RBD_JIT_Tests, RBDStateTest)
{
    using derivativesCppadJIT = DerivativesCppadJIT<hyqStateDim, 1>;

    typename derivativesCppadJIT::FUN_TYPE_CG f = testFunctionRBDState<ct::core::ADCGScalar>;

    derivativesCppadJIT jacCG(f);

    DerivativesCppadSettings settings;
    settings.createJacobian_ = true;

    try
    {
        // compile the Jacobian
        jacCG.compileJIT(settings, "rbdStateTestLib", verbose);
        std::cout << "testRBDState compiled!" << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "testRBDState failed!" << std::endl;
        ASSERT_TRUE(false);
    }
}
