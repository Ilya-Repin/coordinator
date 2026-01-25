#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class CollectActiveHubsTest
    : public TBalancingTestBase
{ };

TEST_F(CollectActiveHubsTest, FiltersDrainingLaggedOfflineHubs) {
    TCoordinationState::TClusterSnapshot snapshot;
    {
        snapshot.emplace(HUB("hub-active"), THubReport{
            EP(42), HUB("hub-active"), DC("myt"), LF(10), {
                {PID(1), PW(50)},
                {PID(7), PW(320)},
        }});

        snapshot.emplace(HUB("hub-draining"), THubReport{
            EP(42), HUB("hub-draining"), DC("myt"), LF(10), {
                {PID(2), PW(200)},
                {PID(3), PW(410)},
            }});

        snapshot.emplace(HUB("hub-lagged"), THubReport{
            EP(41), HUB("hub-lagged"), DC("myt"), LF(10), {
                {PID(5), PW(309)},
                {PID(6), PW(200)},
            }});
    }

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-active")}, {PID(2), HUB("hub-draining")}, {PID(3), HUB("hub-draining")},
            {PID(4), HUB("hub-offline")}, {PID(5), HUB("hub-lagged")}, {PID(6), HUB("hub-lagged")},
            {PID(7), HUB("hub-active")}, {PID(8), HUB("hub-offline")},
        },
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(2), EP(50)},
        },
        .PartitionWeights{
            {PID(1), PW(100)}, {PID(2), PW(200)}, {PID(3), PW(400)},
            {PID(4), PW(150)}, {PID(5), PW(300)}, {PID(6), PW(170)},
            {PID(7), PW(290)},
        },
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{HUB("hub-draining")},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _))
        .WillOnce(Return(TLoadFactor{15}));

    auto [activeHubs, sortedHubs] = CollectActiveHubs(state, Predictor_);

    EXPECT_EQ(activeHubs.size(), 1);
    EXPECT_TRUE(activeHubs.contains(HUB("hub-active")));
    EXPECT_EQ(sortedHubs.size(), 1);
}

TEST_F(CollectActiveHubsTest, SortsHubsByForecastedLoad) {
    TCoordinationState::TClusterSnapshot snapshot;
    {
        snapshot.emplace(HUB("hub-heavy"), THubReport{
            EP(42), HUB("hub-heavy"), DC("myt"), LF(80), {
                {PID(1), PW(150)},
                {PID(3), PW(220)},
        }});

        snapshot.emplace(HUB("hub-light"), THubReport{
            EP(42), HUB("hub-light"), DC("myt"), LF(20), {
                {PID(2), PW(75)},
            }});
    }

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-heavy")}, {PID(2), HUB("hub-light")}, {PID(3), HUB("hub-heavy")},
        },
        .Epoch{EP(42)},
    };

     TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(160)}, {PID(2), PW(450)}, {PID(3), PW(220)},
        },
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    // increasing load of light hub
    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _))
        .WillRepeatedly(Invoke([](TLoadFactor current, auto, auto) {
            return current == TLoadFactor{80} ? TLoadFactor{85} : TLoadFactor{95};
        }));

    auto [activeHubs, sortedHubs] = CollectActiveHubs(state, Predictor_);

    ASSERT_EQ(sortedHubs.size(), 2);
    auto it = sortedHubs.begin();
    EXPECT_EQ(it->second, HUB("hub-heavy")); // 85
    EXPECT_EQ((++it)->second, HUB("hub-light")); // 95
}

TEST_F(CollectActiveHubsTest, PassesCorrectParamsToPredictor) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-a"), THubReport{EP(42), HUB("hub-a"), DC("myt"), LF(50), {
        {PID(1), PW(50)} 
    }});

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-a")}},
        .Epoch{EP(42)},
    };

    // weight in context - 100, in report - 50 -> ExpectedWeightGrowth = 50
    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{{PID(1), PW(100)}},
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };
    
    TCoordinationState state(map, snapshot, context, settings);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(
        Eq(LF(50)), 
        Eq(PW(50)),
        Field(&TPredictionParams::TotalPartitions, 1)
    )).WillOnce(Return(LF(60)));

    auto [activeHubs, sortedHubs] = CollectActiveHubs(state, Predictor_);

    ASSERT_EQ(sortedHubs.size(), 1);
    ASSERT_EQ(activeHubs.size(), 1);
    EXPECT_EQ(sortedHubs.begin()->second, HUB("hub-a"));
}

TEST_F(CollectActiveHubsTest, IncludesOverloadedHubs) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-overloaded"), THubReport{EP(42), HUB("hub-overloaded"), DC("myt"), LF(95), {
        {PID(1), PW(970)} 
    }});

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-overloaded")}},
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{{PID(1), PW(1200)}},
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _)).WillOnce(Return(TLoadFactor{99}));

    auto [activeHubs, sortedHubs] = CollectActiveHubs(state, Predictor_);

    ASSERT_EQ(activeHubs.size(), 1);
    EXPECT_TRUE(activeHubs.contains(HUB("hub-overloaded")));
    ASSERT_EQ(sortedHubs.size(), 1);
    EXPECT_EQ(sortedHubs.begin()->first, LF(99));
    EXPECT_EQ(sortedHubs.begin()->second, HUB("hub-overloaded"));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
