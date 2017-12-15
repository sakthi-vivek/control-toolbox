/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#include <ct/models/HyA/HyAInverseKinematics.h>
#include <ct/models/Irb4600/Irb4600InverseKinematics.h>
#include <ct/rbd/rbd.h>
#include <ct/rbd/state/JointState.h>
#include <ct/models/HyA/HyA.h>

#include <gtest/gtest.h>


// TODO: add proper tests.
TEST(HyAIKTest, IKFastTest)
{
    ct::rbd::HyAInverseKinematics<double> hya_ik_solver;
    ct::rbd::HyA::Kinematics kin;
    typename ct::rbd::tpl::JointState<6, double>::Position pos;
    pos << 0, 1, 0, -1, 1, -1;

    auto ret = kin.getEEPoseInBase(0, pos);

    std::cout << ret.position() << std::endl;

    std::vector<double> joints{0, 1, 0, -1, 1, -1}, eetrans(3), eerot(9);
    hya_ik::ComputeFk(joints.data(), eetrans.data(), eerot.data());

    for (auto i : eetrans)
    {
        std::cout << i << std::endl;
    }

    for (const auto& i : hya_ik_solver.computeInverseKinematics(ret))
    {
        std::cout << i << std::endl << std::endl;
    }
}

TEST(Irb4600IKTest, IKFastTest)
{
    ct::rbd::Irb4600InverseKinematics<double> irb4600_ik_solver;
    ct::rbd::HyA::Kinematics kin;
    typename ct::rbd::tpl::JointState<6, double>::Position pos;
    pos << 0, 1, 0, -1, 1, -1;

    auto ret = kin.getEEPoseInBase(0, pos);

    std::cout << ret.position() << std::endl;

    std::vector<double> joints{0, 1, 0, -1, 1, -1}, eetrans(3), eerot(9);
    irb4600_ik::ComputeFk(joints.data(), eetrans.data(), eerot.data());

    for (auto i : eetrans)
    {
        std::cout << i << std::endl;
    }

    for (const auto& i : irb4600_ik_solver.computeInverseKinematics(ret))
    {
        std::cout << i << std::endl << std::endl;
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
