#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
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

    NDetail::TMigratingPartitions migratingPartitions{
        {PID(2), std::nullopt},
        {PID(3), std::nullopt},
    };

    TBalancingSettings balancingSettings;
    balancingSettings.MinMigrationCooldown = EP(5);
    balancingSettings.MigrationWeightPenaltyCoeff = 0.5;

    auto newContext = NDetail::BuildCoordinationContext(migratingPartitions, state, balancingSettings);

    EXPECT_EQ(newContext.PartitionCooldowns.size(), 3);
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(1)), EP(50));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(2)), EP(42 + 1 + 5 + 220 * 0.5));
    EXPECT_EQ(newContext.PartitionCooldowns.at(PID(3)), EP(42 + 1 + 5 + 150 * 0.5));

    ASSERT_EQ(newContext.PartitionWeights.size(), 2);
    EXPECT_EQ(newContext.PartitionWeights.at(PID(1)), PW(50));
    EXPECT_EQ(newContext.PartitionWeights.at(PID(2)), PW(220));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
