#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class BuildCoordinationContextTest
    : public TBalancingTestBase
{ };

TEST_F(BuildCoordinationContextTest, BuildsCoordinationContext) {
    TCoordinationState::TClusterSnapshot snapshot;
    {
        snapshot.emplace_back(
            EP(42), HUB("hub-active"), DC("myt"), LF(10), PWS({
                {PID(1), PW(50)},
            }));

        snapshot.emplace_back(
            EP(42), HUB("hub-draining"), DC("myt"), LF(10), PWS({
                {PID(2), PW(220)},
            }));
    }

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-active")}, {PID(2), HUB("hub-draining")}, {PID(3), HUB("hub-offline")},
        },
        .Epoch{EP(42)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(1), EP(50)},
        },
        .PartitionWeights{
            {PID(1), PW(100)},
            {PID(2), PW(200)},
        },
    };

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{HUB("hub-draining")},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    TMigratingPartitions migratingPartitions{
        {PID(2), std::nullopt},
        {PID(3), std::nullopt},
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(5);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.5;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    EXPECT_EQ(newContext.PartitionCooldowns.size(), 3);
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(50));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(2)), EP(42 + 1 + 5 + 220 * 0.5));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(3)), EP(42 + 1 + 5 + 150 * 0.5));

    ASSERT_EQ(newContext.PartitionWeights.size(), 2);
    EXPECT_EQ(newContext.PartitionWeights.at(PID(1)), PW(50));
    EXPECT_EQ(newContext.PartitionWeights.at(PID(2)), PW(220));
}

TEST_F(BuildCoordinationContextTest, UsesAverageWeightIfObservedMissing) {
    TCoordinationState::TClusterSnapshot snapshot;
    {
        snapshot.emplace_back(
            EP(10), HUB("hub-1"), DC("myt"), LF(50), PWS({
                {PID(1), PW(100)},
                {PID(2), PW(200)},
            }));
    }

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-1")},
            {PID(2), HUB("hub-1")},
            {PID(3), HUB("hub-1")},
        },
        .Epoch{EP(10)},
    };

    TCoordinationContext context;

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{HUB("hub-draining")},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    TMigratingPartitions migratingPartitions{
        {PID(3), std::nullopt},
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(10);
    balancingSettings.MigrationWeightPenaltyCoeff = 1.0;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    ASSERT_TRUE(newContext.PartitionCooldowns.contains(PID(3)));
    EXPECT_GT(newContext.PartitionCooldowns.at(PID(3)), EP(10 + 10 + 1)); 

    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(3)), EP(171));
}

TEST_F(BuildCoordinationContextTest, FiltersOutdatedCooldowns) {
    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}, {PID(2), HUB("h")}, {PID(3), HUB("h")}},
        .Epoch{EP(100)},
    };

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(100), HUB("h"), DC("dc"), LF(0), PWS({}));

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(1), EP(150)},
            {PID(2), EP(90)},
            {PID(3), EP(100)},
        },
        .PartitionWeights{},
    };

    TCoordinationState state(map, snapshot, context, {});
    TMigratingPartitions migratingPartitions;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, {});

    EXPECT_TRUE(newContext.PartitionCooldowns.contains(PID(1)));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(150));

    EXPECT_FALSE(newContext.PartitionCooldowns.contains(PID(2)));
    EXPECT_FALSE(newContext.PartitionCooldowns.contains(PID(3)));
}

TEST_F(BuildCoordinationContextTest, MigrationOverwritesExistingCooldown) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(10), HUB("h"), DC("dc"), LF(0), PWS({{PID(1), PW(100)}}));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(1), EP(9999)},
        },
        .PartitionWeights{},
    };

    TCoordinationState state(map, snapshot, context, {});

    TMigratingPartitions migratingPartitions{
        {PID(1), std::nullopt}
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(5);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.0;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(16));
}

TEST_F(BuildCoordinationContextTest, UpdatesObservedWeightsInContext) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(
        EP(10), HUB("h"), DC("dc"), LF(0), PWS({
            {PID(1), PW(500)},
        }));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(100)},
        },
    };

    TCoordinationState state(map, snapshot, context, {});
    TMigratingPartitions migratingPartitions;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, {});

    ASSERT_TRUE(newContext.PartitionWeights.contains(PID(1)));
    EXPECT_EQ(newContext.PartitionWeights.at(PID(1)), PW(500));
}

TEST_F(BuildCoordinationContextTest, HandlesExpectedWeightGrowth) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(10), HUB("h"), DC("dc"), LF(0), PWS({{PID(1), PW(100)}}));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationState state(map, snapshot, {}, {});
    
    TMigratingPartitions migratingPartitions{{PID(1), std::nullopt}};

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(0);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.5;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(61));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
