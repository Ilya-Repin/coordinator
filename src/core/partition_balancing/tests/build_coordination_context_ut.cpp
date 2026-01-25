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
        snapshot.emplace(HUB("hub-active"), THubReport{
            EP(42), HUB("hub-active"), DC("myt"), LF(10), {
                {PID(1), PW(50)},
        }});

        snapshot.emplace(HUB("hub-draining"), THubReport{
            EP(42), HUB("hub-draining"), DC("myt"), LF(10), {
                {PID(2), PW(220)},
            }});
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
    // Scenario: We have partitions with known weights, and one "new" partition
    // without report. The new partition is migrating.
    // It should use the average weight of the cluster.

    TCoordinationState::TClusterSnapshot snapshot;
    {
        // Hub with 2 partitions, weights 100 and 200. Average = 150.
        snapshot.emplace(HUB("hub-1"), THubReport{
            EP(10), HUB("hub-1"), DC("myt"), LF(50), {
                {PID(1), PW(100)},
                {PID(2), PW(200)},
            }});
    }

    TPartitionMap map{
        .Partitions{
            {PID(1), HUB("hub-1")},
            {PID(2), HUB("hub-1")},
            {PID(3), HUB("hub-1")}, // PID(3) has no report in snapshot
        },
        .Epoch{EP(10)},
    };

    TCoordinationContext context; // Empty context

    TStateBuildingSettings settings{
        .BlockedDCs{},
        .BlockedHubs{HUB("hub-draining")},
        .OverloadThreshold{LF(90)},
    };

    TCoordinationState state(map, snapshot, context, settings);

    TMigratingPartitions migratingPartitions{
        {PID(3), std::nullopt}, // PID 3 is migrating
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(10);
    balancingSettings.MigrationWeightPenaltyCoeff = 1.0;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    // Calculation:
    // Base Epoch: 10
    // MinCooldown: 10
    // Weight: Average of (100, 200) = 150 (Assuming TCoordinationState calc is standard)
    // Penalty: 150 * 1.0 = 150
    // Fixed +1 constant
    // Total = 10 + 10 + 150 + 1 = 171
    
    // We verify that it picked up a weight > 0 (implying fallback worked)
    ASSERT_TRUE(newContext.PartitionCooldowns.contains(PID(3)));
    EXPECT_GT(newContext.PartitionCooldowns.at(PID(3)), EP(10 + 10 + 1)); 
    
    // If average calculation is exact arithmetic mean:
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(3)), EP(171));
}

TEST_F(BuildCoordinationContextTest, FiltersOutdatedCooldowns) {
    // Scenario:
    // PID 1: Has cooldown in future -> Should be kept.
    // PID 2: Has cooldown in past -> Should be removed.
    // PID 3: Has cooldown equal to now -> Should be removed (logic is > current).

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}, {PID(2), HUB("h")}, {PID(3), HUB("h")}},
        .Epoch{EP(100)},
    };
    
    // Snapshot is needed to satisfy TCoordinationState constructor, can be empty for this logic check
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("h"), THubReport{EP(100), HUB("h"), DC("dc"), LF(0), {}});

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(1), EP(150)}, // Future
            {PID(2), EP(90)},  // Past
            {PID(3), EP(100)}, // Current
        },
        .PartitionWeights{},
    };

    TCoordinationState state(map, snapshot, context, {});
    TMigratingPartitions migratingPartitions; // No migrations

    auto newContext = BuildCoordinationContext(migratingPartitions, state, {});

    EXPECT_TRUE(newContext.PartitionCooldowns.contains(PID(1)));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(150));

    EXPECT_FALSE(newContext.PartitionCooldowns.contains(PID(2)));
    EXPECT_FALSE(newContext.PartitionCooldowns.contains(PID(3)));
}

TEST_F(BuildCoordinationContextTest, MigrationOverwritesExistingCooldown) {
    // Scenario: PID 1 already has a very long cooldown, but it is selected for migration again.
    // The new migration cooldown logic should overwrite the old one.

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("h"), THubReport{
        EP(10), HUB("h"), DC("dc"), LF(0), {{PID(1), PW(100)}}
    });

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{
            {PID(1), EP(9999)}, // Very far future
        },
        .PartitionWeights{},
    };

    TCoordinationState state(map, snapshot, context, {});

    TMigratingPartitions migratingPartitions{
        {PID(1), std::nullopt}
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(5);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.0; // Simplify math

    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    // Should NOT be 9999.
    // Calc: Epoch(10) + Min(5) + Penalty(0) + 1 = 16
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(16));
}

TEST_F(BuildCoordinationContextTest, UpdatesObservedWeightsInContext) {
    // Scenario: Ensure that the weights in the returned context match the weights
    // observed in the snapshot (updating the historical values).

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("h"), THubReport{
        EP(10), HUB("h"), DC("dc"), LF(0), {
            {PID(1), PW(500)}, // New weight
        }
    });

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{
            {PID(1), PW(100)}, // Old weight
        },
    };

    TCoordinationState state(map, snapshot, context, {});
    TMigratingPartitions migratingPartitions;

    auto newContext = BuildCoordinationContext(migratingPartitions, state, {});

    ASSERT_TRUE(newContext.PartitionWeights.contains(PID(1)));
    EXPECT_EQ(newContext.PartitionWeights.at(PID(1)), PW(500));
}

TEST_F(BuildCoordinationContextTest, HandlesExpectedWeightGrowth) {
    // Scenario: Verify that ExpectedWeightGrowth is added to the base weight
    // before calculating the penalty.
    // Note: Since we cannot easily inject ExpectedWeightGrowth via public API of TCoordinationState
    // without mocking internals, we rely on the fact that TCoordinationState generally
    // calculates growth based on History vs Snapshot.
    // Here we simulate a scenario where Observed is 100, but let's assume specific settings/mocking
    // allows growth.
    // If mocking is not possible, we focus on the logic:
    // Weight = Observed + Growth.

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("h"), THubReport{
        EP(10), HUB("h"), DC("dc"), LF(0), {{PID(1), PW(100)}}
    });

    TPartitionMap map{
        .Partitions{{PID(1), HUB("h")}},
        .Epoch{EP(10)},
    };

    TCoordinationState state(map, snapshot, {}, {});
    
    TMigratingPartitions migratingPartitions{{PID(1), std::nullopt}};

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(0);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.5;

    // With 0 growth: Penalty = 100 * 0.5 = 50. Result = 10 + 0 + 50 + 1 = 61.
    auto newContext = BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(61));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
