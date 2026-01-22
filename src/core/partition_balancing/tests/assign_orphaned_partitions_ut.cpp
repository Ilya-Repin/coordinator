#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDomain;

class AssignOrphanedPartitionsTest
    : public TBalancingTestBase
{
protected:
    TCoordinationState MakeStateWithHubs(const std::vector<std::pair<std::string, NDomain::TLoadFactor>>& hubsInfo)
    {
        TCoordinationState::TClusterSnapshot snapshot;
        for (const auto& [name, load] : hubsInfo) {
            snapshot.emplace(HUB(name), THubReport{
                EP(42), HUB(name), DC("myt"), load, {}
            });
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

    NDetail::TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(10), HUB("hub-light"));
    sortedHubs.emplace(LF(50), HUB("hub-heavy"));

    NDetail::TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(50),  PID(2)}
    };

    NDetail::TAssignedPartitions assignedPartitions;
    
    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(10), PW(100), _))
        .WillOnce(Return(LF(30)));

    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(30), PW(50), _))
        .WillOnce(Return(LF(35)));

    auto migratingWeight = NDetail::AssignOrphanedPartitions(
        state, orphaned, Predictor_, sortedHubs, assignedPartitions
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
    
    NDetail::TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(0), HUB("hub-one"));

    NDetail::TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(200), PID(2)},
    };

    NDetail::TAssignedPartitions assignedPartitions;

    {
        testing::InSequence s;

        // Params: Total=0, Weight=0 
        EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(0), PW(100), 
            AllOf(
                Field(&TPredictionParams::TotalPartitions, 0),
                Field(&TPredictionParams::PartitionsWeight, PW(0))
            )
        )).WillOnce(Return(LF(10)));


        // Params: Total=1, Weight=100
        EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(10), PW(200), 
            AllOf(
                Field(&TPredictionParams::TotalPartitions, 1),
                Field(&TPredictionParams::PartitionsWeight, PW(100))
            )
        )).WillOnce(Return(LF(30)));
    }

    auto migratingWeight = NDetail::AssignOrphanedPartitions(
        state, orphaned, Predictor_, sortedHubs, assignedPartitions
    );

    ASSERT_EQ(migratingWeight.size(), 1);
    EXPECT_EQ(migratingWeight[HUB("hub-one")].first, 2); // added 2 partitions
    EXPECT_EQ(migratingWeight[HUB("hub-one")].second, PW(300)); // 100 + 200
}

TEST_F(AssignOrphanedPartitionsTest, StopsWhenNoHubsAvailable) {
    auto state = MakeStateWithHubs({});
    NDetail::TSortedHubs sortedHubs;

    NDetail::TWeightedPartitions orphaned = {{PW(100), PID(1)}};
    NDetail::TAssignedPartitions assignedPartitions;

    EXPECT_CALL(*Predictor_, PredictLoadFactor(_, _, _)).Times(0);

    auto migratingWeight = NDetail::AssignOrphanedPartitions(
        state, orphaned, Predictor_, sortedHubs, assignedPartitions
    );

    EXPECT_TRUE(assignedPartitions.empty());
    EXPECT_TRUE(migratingWeight.empty());
}

TEST_F(AssignOrphanedPartitionsTest, ConsidersSortOrderAfterUpdate) {
    auto state = MakeStateWithHubs({
        {"hub-a", LF(10)},
        {"hub-b", LF(15)}
    });

    NDetail::TSortedHubs sortedHubs;
    sortedHubs.emplace(LF(10), HUB("hub-a"));
    sortedHubs.emplace(LF(15), HUB("hub-b"));

    NDetail::TWeightedPartitions orphaned = {
        {PW(100), PID(1)},
        {PW(100), PID(2)}
    };
    NDetail::TAssignedPartitions assignedPartitions;

    // 1. Hub-A (10) < Hub-B (15).
    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(10), PW(100), _))
        .WillOnce(Return(LF(20)));

    // 2. sortedHubs: Hub-B (15), Hub-A (20)
    EXPECT_CALL(*Predictor_, PredictLoadFactor(LF(15), PW(100), _))
        .WillOnce(Return(LF(25)));

    NDetail::AssignOrphanedPartitions(
        state, orphaned, Predictor_, sortedHubs, assignedPartitions
    );

    ASSERT_EQ(assignedPartitions.size(), 2);
    EXPECT_EQ(assignedPartitions[0].second, HUB("hub-a"));
    EXPECT_EQ(assignedPartitions[1].second, HUB("hub-b"));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
