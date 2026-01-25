#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class ExecuteRebalancingPhaseTest
    : public TBalancingTestBase
{
protected:
    void AddHubToStructures(
        TSortedHubs& sortedHubs,
        THubPartitions& hubPartitions,
        const NDomain::THubEndpoint& hub,
        NDomain::TLoadFactor load,
        const std::vector<std::pair<NDomain::TPartitionId, NDomain::TPartitionWeight>>& parts)
    {
        sortedHubs.insert({load, hub});
        for (const auto& [pid, weight] : parts) {
            hubPartitions[hub].insert({weight, pid});
        }
    }
};

TEST_F(ExecuteRebalancingPhaseTest, MovesPartitionWhenImbalanceIsHigh) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-max"), THubReport{
        EP(100), HUB("hub-max"), DC("myt"), LF(90), { {PID(1), PW(20)} }
    });
    snapshot.emplace(HUB("hub-min"), THubReport{
        EP(100), HUB("hub-min"), DC("myt"), LF(10), {}
    });

    TPartitionMap map{
        .Partitions{ {PID(1), HUB("hub-max")} },
        .Epoch{EP(100)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{}, 
        .PartitionWeights{ {PID(1), PW(20)} },
    };

    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs;
    THubPartitions hubPartitions;
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-max"), LF(90), { {PID(1), PW(20)} });
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-min"), LF(10), {});

    TMigrationContext migrationContext;
    NDomain::TBalancingSettings settings;
    settings.MinLoadFactorDelta = LF(10);
    settings.MigratingWeightLimit = PW(1000);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(90), PW(20), _))
        .WillRepeatedly(Return(LF(70)));

    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(10), PW(20), _))
        .WillRepeatedly(Return(LF(30)));

    ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, Predictor_, state, settings);

    EXPECT_TRUE(hubPartitions[HUB("hub-max")].empty());

    ASSERT_EQ(hubPartitions[HUB("hub-min")].size(), 1u);
    EXPECT_EQ(hubPartitions[HUB("hub-min")].begin()->second, PID(1));

    EXPECT_EQ(migrationContext.TotalMigratingWeight, PW(20));
    ASSERT_TRUE(migrationContext.MigratingPartitions.contains(PID(1)));
    EXPECT_EQ(migrationContext.MigratingPartitions[PID(1)], HUB("hub-max"));
}

TEST_F(ExecuteRebalancingPhaseTest, SkipsMigrationIfCooldownIsActive) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-max"), THubReport{
        EP(100), HUB("hub-max"), DC("myt"), LF(90), { {PID(1), PW(20)} }
    });
    snapshot.emplace(HUB("hub-min"), THubReport{
        EP(100), HUB("hub-min"), DC("myt"), LF(10), {}
    });

    TPartitionMap map{
        .Partitions{ {PID(1), HUB("hub-max")} },
        .Epoch{EP(100)},
    };

    TCoordinationContext context{
        .PartitionCooldowns{ {PID(1), EP(105)} }, 
        .PartitionWeights{ {PID(1), PW(20)} },
    };

    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs;
    THubPartitions hubPartitions;
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-max"), LF(90), { {PID(1), PW(20)} });
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-min"), LF(10), {});

    TMigrationContext migrationContext;
    NDomain::TBalancingSettings settings;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _)).Times(0);

    ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, Predictor_, state, settings);

    EXPECT_EQ(hubPartitions[HUB("hub-max")].size(), 1u);
    EXPECT_TRUE(migrationContext.MigratingPartitions.empty());
}

TEST_F(ExecuteRebalancingPhaseTest, StopsIfDeltaIsTooSmall) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-1"), THubReport{EP(100), HUB("hub-1"), DC("myt"), LF(55), {{PID(1), PW(10)}}});
    snapshot.emplace(HUB("hub-2"), THubReport{EP(100), HUB("hub-2"), DC("myt"), LF(50), {}});

    TPartitionMap map{ .Partitions{{PID(1), HUB("hub-1")}}, .Epoch{EP(100)} };
    TCoordinationContext context; 
    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs;
    THubPartitions hubPartitions;
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-1"), LF(55), {{PID(1), PW(10)}});
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-2"), LF(50), {});

    TMigrationContext migrationContext;
    NDomain::TBalancingSettings settings;
    settings.MinLoadFactorDelta = LF(10);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _)).Times(0);

    ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, Predictor_, state, settings);

    EXPECT_EQ(hubPartitions[HUB("hub-1")].size(), 1u);
    EXPECT_TRUE(migrationContext.MigratingPartitions.empty());
}

TEST_F(ExecuteRebalancingPhaseTest, PreventsOvershoot) {
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-max"), THubReport{EP(100), HUB("hub-max"), DC("myt"), LF(100), {{PID(1), PW(30)}}});
    snapshot.emplace(HUB("hub-min"), THubReport{EP(100), HUB("hub-min"), DC("myt"), LF(90), {}});

    TPartitionMap map{ .Partitions{{PID(1), HUB("hub-max")}}, .Epoch{EP(100)} };
    TCoordinationContext context;
    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs;
    THubPartitions hubPartitions;
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-max"), LF(100), {{PID(1), PW(30)}});
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-min"), LF(90), {});

    TMigrationContext migrationContext;
    NDomain::TBalancingSettings settings;
    settings.MinLoadFactorDelta = LF(5);

    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(100), PW(30), _)).WillRepeatedly(Return(LF(70)));
    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(90), PW(30), _)).WillRepeatedly(Return(LF(120)));

    ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, Predictor_, state, settings);

    EXPECT_EQ(hubPartitions[HUB("hub-max")].size(), 1u);
    EXPECT_TRUE(migrationContext.MigratingPartitions.empty());
}

TEST_F(ExecuteRebalancingPhaseTest, UpdatesContextWhenCancellingMigration) {    
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(HUB("hub-max"), THubReport{EP(100), HUB("hub-max"), DC("myt"), LF(80), {{PID(1), PW(10)}}});
    snapshot.emplace(HUB("hub-min"), THubReport{EP(100), HUB("hub-min"), DC("myt"), LF(20), {}});

    TPartitionMap map{ .Partitions{{PID(1), HUB("hub-max")}}, .Epoch{EP(100)} };
    TCoordinationContext context;
    TStateBuildingSettings stateSettings;
    TCoordinationState state(map, snapshot, context, stateSettings);

    TSortedHubs sortedHubs;
    THubPartitions hubPartitions;
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-max"), LF(80), {{PID(1), PW(10)}});
    AddHubToStructures(sortedHubs, hubPartitions, HUB("hub-min"), LF(20), {});

    TMigrationContext migrationContext;
    migrationContext.MigratingPartitions.emplace(PID(1), HUB("hub-min"));
    migrationContext.TotalMigratingWeight = PW(50);

    NDomain::TBalancingSettings settings;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(80), PW(10), _)).WillRepeatedly(Return(LF(70)));
    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(20), PW(10), _)).WillRepeatedly(Return(LF(30)));

    ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, Predictor_, state, settings);

    EXPECT_EQ(hubPartitions[HUB("hub-min")].size(), 1u);

    EXPECT_FALSE(migrationContext.MigratingPartitions.contains(PID(1)));
    EXPECT_EQ(migrationContext.TotalMigratingWeight, PW(40));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
