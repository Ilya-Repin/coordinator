#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

#include <set>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class BuildPredictionParamsTest
    : public TBalancingTestBase
{ };

TEST_F(BuildPredictionParamsTest, CorrectlyBuildsParams) {
    std::set<TWeightedPartition> partitions{
        {PW(100), PID(1)},
        {PW(237), PID(2)},
        {PW(341), PID(3)},
        {PW(400), PID(4)},
        {PW(401), PID(5)},
        {PW(499), PID(6)},
        {PW(504), PID(7)},
        {PW(600), PID(8)},
    };
    THubState state;
    state.LoadFactor = LF(35);

    auto params = BuildPredictionParams(LF(15), PW(101), true, partitions, state);
    
    EXPECT_EQ(params.LoadFactor, LF(15));
    EXPECT_EQ(params.PartitionWeight, PW(101));
    EXPECT_EQ(params.Increasing, true);
    EXPECT_EQ(params.TotalPartitions, 8);
    EXPECT_EQ(params.PartitionsWeight, PW(3082));
    EXPECT_EQ(params.OriginalLoadFactor, LF(35));
}

TEST_F(BuildPredictionParamsTest, HandlesEmptyPartitions) {
    std::set<TWeightedPartition> partitions;
    THubState state;
    state.LoadFactor = LF(5);

    auto params = BuildPredictionParams(LF(5), PW(3), false, partitions, state);

    EXPECT_EQ(params.LoadFactor, LF(5));
    EXPECT_EQ(params.PartitionWeight, PW(3));
    EXPECT_EQ(params.Increasing, false);
    EXPECT_EQ(params.TotalPartitions, 0);
    EXPECT_EQ(params.PartitionsWeight, PW(0));
    EXPECT_EQ(params.OriginalLoadFactor, LF(5));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
