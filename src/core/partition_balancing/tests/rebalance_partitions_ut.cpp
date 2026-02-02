#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class RebalancePartitionsTest
    : public TBalancingTestBase
{
};

TEST_F(RebalancePartitionsTest, DoesNothingWhenCVBelowThreshold) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(100), HUB("hub-1"), DC("myt"), LF(50), PWS({}));
    snapshot.emplace_back(EP(100), HUB("hub-2"), DC("myt"), LF(52), PWS({}));

    TPartitionMap map{
        .Partitions{},
        .Epoch{EP(100)},
    };

    TCoordinationContext context;
    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs{
        {LF(50), HUB("hub-1")},
        {LF(52), HUB("hub-2")},
    };

    TAssignedPartitions assignedPartitions;

    TMigrationContext migrationContext;
    TBalancingSettings settings;
    settings.BalancingThresholdCV = 1.0;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_)).Times(0);

    RebalancePartitions(
        sortedHubs,
        assignedPartitions,
        migrationContext,
        *Predictor_,
        state,
        settings);

    EXPECT_TRUE(assignedPartitions.empty());
    EXPECT_TRUE(migrationContext.MigratingPartitions.empty());
}

TEST_F(RebalancePartitionsTest, PerformsSingleRebalancingPhase) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(100), HUB("hub-max"), DC("myt"), LF(90), PWS({{PID(1), PW(20)}}));
    snapshot.emplace_back(EP(100), HUB("hub-min"), DC("myt"), LF(10), PWS({}));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-max")}},
        .Epoch{EP(100)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{{PID(1), PW(20)}},
    };

    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs{
        {LF(90), HUB("hub-max")},
        {LF(10), HUB("hub-min")},
    };

    TAssignedPartitions assignedPartitions{
        {PID(1), HUB("hub-max")},
    };

    TMigrationContext migrationContext;
    TBalancingSettings settings;
    settings.BalancingThresholdCV = 0.1;
    settings.BalancingTargetCV = 0.0;
    settings.MinLoadFactorDelta = LF(5);
    settings.MigratingWeightLimit = PW(1000);
    settings.MaxRebalancePhases = 1;

    EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(90)),
            Field(&TPredictionParams::PartitionWeight, PW(20)),
            Field(&TPredictionParams::Increasing, false),
            Field(&TPredictionParams::TotalPartitions, 1),
            Field(&TPredictionParams::PartitionsWeight, PW(20)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(90)))))
        .WillOnce(Return(LF(70)));

    EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(10)),
            Field(&TPredictionParams::PartitionWeight, PW(20)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 0),
            Field(&TPredictionParams::PartitionsWeight, PW(0)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(10)))))
        .WillOnce(Return(LF(30)));

    RebalancePartitions(
        sortedHubs,
        assignedPartitions,
        migrationContext,
        *Predictor_,
        state,
        settings);

    
    EXPECT_EQ(assignedPartitions.size(), 1u);
    EXPECT_EQ(assignedPartitions.front().first, PID(1));
    EXPECT_EQ(assignedPartitions.front().second, HUB("hub-min"));

    EXPECT_EQ(migrationContext.TotalMigratingWeight, PW(20));
}

TEST_F(RebalancePartitionsTest, StopsWhenMigrationBudgetExceeded) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(100), HUB("hub-max"), DC("myt"), LF(90), PWS({{PID(1), PW(50)}}));
    snapshot.emplace_back(EP(100), HUB("hub-min"), DC("myt"), LF(10), PWS({}));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-max")}},
        .Epoch{EP(100)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{{PID(1), PW(50)}},
    };

    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs{
        {LF(90), HUB("hub-max")},
        {LF(10), HUB("hub-min")},
    };

    TAssignedPartitions assignedPartitions{
        {PID(1), HUB("hub-max")},
    };

    TMigrationContext migrationContext;
    migrationContext.TotalMigratingWeight = PW(45);

    TBalancingSettings settings;
    settings.BalancingThresholdCV = 0.0;
    settings.MigrationBudgetThreshold = PW(10);
    settings.MigratingWeightLimit = PW(50);
    settings.MaxRebalancePhases = 10;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_)).Times(0);

    RebalancePartitions(
        sortedHubs,
        assignedPartitions,
        migrationContext,
        *Predictor_,
        state,
        settings);

    EXPECT_EQ(assignedPartitions.front().second, HUB("hub-max"));
}

TEST_F(RebalancePartitionsTest, RespectsMaxRebalancePhases) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace_back(EP(100), HUB("hub-1"), DC("myt"), LF(80), PWS({{PID(1), PW(10)}}));
    snapshot.emplace_back(EP(100), HUB("hub-2"), DC("myt"), LF(20), PWS({}));

    TPartitionMap map{
        .Partitions{{PID(1), HUB("hub-1")}},
        .Epoch{EP(100)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{},
        .PartitionWeights{{PID(1), PW(10)}},
    };

    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs{
        {LF(80), HUB("hub-1")},
        {LF(20), HUB("hub-2")},
    };

    TAssignedPartitions assignedPartitions{
        {PID(1), HUB("hub-1")},
    };

    TMigrationContext migrationContext;
    TBalancingSettings settings;
    settings.BalancingThresholdCV = 0.0;
    settings.BalancingTargetCV = 0.0;
    settings.MaxRebalancePhases = 0;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_)).Times(0);

    RebalancePartitions(
        sortedHubs,
        assignedPartitions,
        migrationContext,
        *Predictor_,
        state,
        settings);

    EXPECT_EQ(assignedPartitions.front().second, HUB("hub-1"));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
