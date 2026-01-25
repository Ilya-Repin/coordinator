#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class SeparatePartitionsTest
    : public TBalancingTestBase
{ };

TEST_F(SeparatePartitionsTest, CorrectLabelsAndSortsOrphanedPartitions) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-active"), THubReport{
        EP(42), HUB("hub-active"), DC("myt"), LF(25), {
            {PID(1), PW(100)},
            {PID(3), PW(150)},
    }});

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-active")},
            {PID(2), HUB("hub-dead")},
            {PID(3), HUB("hub-active")},
            {PID(4), HUB("hub-offline")},
        },
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(100)}, {PID(2), PW(500)}, {PID(3), PW(150)}, {PID(4), PW(300)},
        },
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);
    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _))
        .WillOnce(Return(TLoadFactor{25}));

    auto [activeHubs, sortedHubs] = CollectActiveHubs(state, Predictor_);
    ASSERT_EQ(activeHubs.size(), 1);

    auto result = SeparatePartitions(state, activeHubs);
    EXPECT_EQ(result.first.size(), 2);
    EXPECT_EQ(result.first[0].second, HUB("hub-active"));
    EXPECT_EQ(result.first[1].second, HUB("hub-active"));

    ASSERT_EQ(result.second.size(), 2);

    EXPECT_EQ(result.second[0].second, PID(2));
    EXPECT_EQ(result.second[0].first, PW(500));

    EXPECT_EQ(result.second[1].second, PID(4));
    EXPECT_EQ(result.second[1].first, PW(300));
}

TEST_F(SeparatePartitionsTest, FallbackToAverageWeightWhenObservedMissing) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-active"), THubReport{
        EP(42), HUB("hub-active"), DC("myt"), LF(25), {
            {PID(1), PW(200)},
    }});

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-active")},
            {PID(2), HUB("hub-dead")},
        },
        .Epoch{EP(42)},
    };

    TCoordinationContext context; 

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);
    ASSERT_EQ(state.GetAveragePartitionWeight(), PW(200));
    
    THubEndpoints activeHubs;
    activeHubs.insert(HUB("hub-active"));

    auto result = SeparatePartitions(state, activeHubs);

    // 1. Assigned (PID 1)
    ASSERT_EQ(result.first.size(), 1);
    EXPECT_EQ(result.first[0].first, PID(1));

    // 2. Orphaned (PID 2)
    ASSERT_EQ(result.second.size(), 1);
    const auto& orphanedPartition = result.second[0];
    
    EXPECT_EQ(orphanedPartition.second, PID(2));
    EXPECT_EQ(orphanedPartition.first, PW(200)); // Average weight
}

TEST_F(SeparatePartitionsTest, SortsOrphansByCalculatedWeightDescending) {
    TCoordinationState::TClusterSnapshot snapshot; 

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-dead")},
            {PID(2), HUB("hub-dead")},
            {PID(3), HUB("hub-dead")},
        },
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(100)}, 
            {PID(2), PW(300)}, 
            {PID(3), PW(50)},
        },
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };
    TCoordinationState state(map, snapshot, context, settings);
    
    THubEndpoints activeHubs;

    auto result = SeparatePartitions(state, activeHubs);

    ASSERT_EQ(result.first.size(), 0);
    ASSERT_EQ(result.second.size(), 3);

    EXPECT_EQ(result.second[0].second, PID(2));
    EXPECT_EQ(result.second[0].first, PW(300));

    EXPECT_EQ(result.second[1].second, PID(1));
    EXPECT_EQ(result.second[1].first, PW(100));

    EXPECT_EQ(result.second[2].second, PID(3));
    EXPECT_EQ(result.second[2].first, PW(50));
}

TEST_F(SeparatePartitionsTest, AllHubsActiveNoOrphans) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-1"), THubReport{
        EP(10), HUB("hub-1"), DC("dc1"), LF(10), {{PID(1), PW(100)}}
    });

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-1")}},
        .Epoch{EP(42)},
    };

    TCoordinationContext context;
    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };
    TCoordinationState state(map, snapshot, context, settings);

    THubEndpoints activeHubs;
    activeHubs.insert(HUB("hub-1"));

    auto result = SeparatePartitions(state, activeHubs);

    EXPECT_EQ(result.first.size(), 1); // Assigned
    EXPECT_EQ(result.second.size(), 0); // Orphans empty
}

TEST_F(SeparatePartitionsTest, AddsExpectedGrowthToWeight) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-1"), THubReport{
        EP(10), HUB("hub-1"), DC("dc1"), LF(10), {{PID(1), PW(100)}}
    });

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-1")}},
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(150)}
        }
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{},
        .OverloadThreshold{LF(90)},
    };
    TCoordinationState state(map, snapshot, context, settings);

    THubEndpoints activeHubs;
    
    auto result = SeparatePartitions(state, activeHubs);

    ASSERT_EQ(result.second.size(), 1);
    EXPECT_EQ(result.second[0].first, PW(150));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
