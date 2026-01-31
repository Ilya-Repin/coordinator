#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDetail;
using namespace NDomain;

class AssignOrphanedPartitionsTest
    : public TBalancingTestBase
{
protected:
    TCoordinationState MakeStateWithHubs(const std::vector<std::pair<std::string, NDomain::TLoadFactor>>& hubsInfo)
    {
        TCoordinationState::TClusterSnapshot snapshot;
        for (const auto& [name, load] : hubsInfo) {
            snapshot.emplace_back(EP(42), HUB(name), DC("myt"), load, PWS({}));
        }

        TPartitionMap map{
            .Partitions = {},
            .Epoch = EP(42)
        };
        TCoordinationContext context;
        TStateBuildingSettings settings;

        return TCoordinationState(map, snapshot, context, settings);
    }
};

TEST_F(AssignOrphanedPartitionsTest, DistributesLoadBetweenHubs) {
    auto state = MakeStateWithHubs({
        {"hub-light", LF(10)},
        {"hub-heavy", LF(50)}
    });

    TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(10), HUB("hub-light"));
    sortedHubs.emplace(LF(50), HUB("hub-heavy"));

    TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(50),  PID(2)}
    };

    TAssignedPartitions assignedPartitions;
    
    EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(10)),
            Field(&TPredictionParams::PartitionWeight, PW(100)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 0),
            Field(&TPredictionParams::PartitionsWeight, PW(0)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(10)))))
        .WillOnce(Return(LF(30)));

    EXPECT_CALL(*Predictor_, 
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(30)),
            Field(&TPredictionParams::PartitionWeight, PW(50)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 1),
            Field(&TPredictionParams::PartitionsWeight, PW(100)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(10)))))
        .WillOnce(Return(LF(35)));

    auto migratingWeight = AssignOrphanedPartitions(
        state, orphaned, *Predictor_, sortedHubs, assignedPartitions
    );

    ASSERT_EQ(assignedPartitions.size(), 2);
    
    EXPECT_EQ(assignedPartitions[0].first, PID(1));
    EXPECT_EQ(assignedPartitions[0].second, HUB("hub-light"));
    
    EXPECT_EQ(assignedPartitions[1].first, PID(2));
    EXPECT_EQ(assignedPartitions[1].second, HUB("hub-light"));

    ASSERT_FALSE(sortedHubs.empty());

    EXPECT_EQ(sortedHubs.begin()->first, LF(35));
    EXPECT_EQ(sortedHubs.begin()->second, HUB("hub-light"));
}

TEST_F(AssignOrphanedPartitionsTest, CorrectlyUpdatesPredictorParams) {
    auto state = MakeStateWithHubs({{"hub-one", LF(0)}});
    
    TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(0), HUB("hub-one"));

    TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(200), PID(2)},
    };

    TAssignedPartitions assignedPartitions;

    {
        testing::InSequence s;

        // Params: Total=0, Weight=0 
        EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(0)),
            Field(&TPredictionParams::PartitionWeight, PW(100)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 0),
            Field(&TPredictionParams::PartitionsWeight, PW(0)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(0)))))
        .WillOnce(Return(LF(10)));

        // Params: Total=1, Weight=100
        EXPECT_CALL(*Predictor_, 
            PredictLoadFactor(AllOf(
                Field(&TPredictionParams::LoadFactor, LF(10)),
                Field(&TPredictionParams::PartitionWeight, PW(200)),
                Field(&TPredictionParams::Increasing, true),
                Field(&TPredictionParams::TotalPartitions, 1),
                Field(&TPredictionParams::PartitionsWeight, PW(100)),
                Field(&TPredictionParams::OriginalLoadFactor, LF(0)))))
            .WillOnce(Return(LF(30)));
    }

    auto migratingWeight = AssignOrphanedPartitions(
        state, orphaned, *Predictor_, sortedHubs, assignedPartitions
    );

    ASSERT_EQ(migratingWeight.size(), 1);
    EXPECT_EQ(migratingWeight[HUB("hub-one")].first, 2); // added 2 partitions
    EXPECT_EQ(migratingWeight[HUB("hub-one")].second, PW(300)); // 100 + 200
}

TEST_F(AssignOrphanedPartitionsTest, StopsWhenNoHubsAvailable) {
    auto state = MakeStateWithHubs({});
    TSortedHubs sortedHubs;

    TWeightedPartitions orphaned = {{PW(100), PID(1)}};
    TAssignedPartitions assignedPartitions;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_)).Times(0);

    auto migratingWeight = AssignOrphanedPartitions(
        state, orphaned, *Predictor_, sortedHubs, assignedPartitions
    );

    EXPECT_TRUE(assignedPartitions.empty());
    EXPECT_TRUE(migratingWeight.empty());
}

TEST_F(AssignOrphanedPartitionsTest, ConsidersSortOrderAfterUpdate) {
    auto state = MakeStateWithHubs({
        {"hub-a", LF(10)},
        {"hub-b", LF(15)}
    });

    TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(10), HUB("hub-a"));
    sortedHubs.emplace(LF(15), HUB("hub-b"));

    TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(100), PID(2)}
    };
    TAssignedPartitions assignedPartitions;

    // 1. Hub-A (10) < Hub-B (15).
    EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(10)),
            Field(&TPredictionParams::PartitionWeight, PW(100)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 0),
            Field(&TPredictionParams::PartitionsWeight, PW(0)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(10)))))
        .WillOnce(Return(LF(20)));

    // 2. sortedHubs: Hub-B (15), Hub-A (20)
    EXPECT_CALL(*Predictor_,
        PredictLoadFactor(AllOf(
            Field(&TPredictionParams::LoadFactor, LF(15)),
            Field(&TPredictionParams::PartitionWeight, PW(100)),
            Field(&TPredictionParams::Increasing, true),
            Field(&TPredictionParams::TotalPartitions, 0),
            Field(&TPredictionParams::PartitionsWeight, PW(0)),
            Field(&TPredictionParams::OriginalLoadFactor, LF(15)))))
        .WillOnce(Return(LF(25)));

    AssignOrphanedPartitions(
        state, orphaned, *Predictor_, sortedHubs, assignedPartitions
    );

    ASSERT_EQ(assignedPartitions.size(), 2);
    EXPECT_EQ(assignedPartitions[0].second, HUB("hub-a"));
    EXPECT_EQ(assignedPartitions[1].second, HUB("hub-b"));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
