#include "coordination_state.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace {

////////////////////////////////////////////////////////////////////////////////

using namespace NCoordinator::NCore::NDomain;

TPartitionMap MakePartitionMap()
{
    TPartitionMap map;
    map.Epoch = TEpoch{42};
    map.Partitions = {
        {TPartitionId(1), THubEndpoint("hub-a")},
        {TPartitionId(2), THubEndpoint("hub-b")},
        {TPartitionId(3), THubEndpoint("hub-c")},
        {TPartitionId(4), THubEndpoint("hub-a")},
    };
    return map;
}

TCoordinationContext MakeContext()
{
    TCoordinationContext ctx;
    ctx.PartitionWeights = {
        {TPartitionId(1), TPartitionWeight(100)},
        {TPartitionId(2), TPartitionWeight(200)},
        {TPartitionId(3), TPartitionWeight(400)},
        {TPartitionId(4), TPartitionWeight(20)},
    };
    ctx.PartitionCooldowns = {
        {TPartitionId(2), TEpoch{10}},
    };
    return ctx;
}

TStateBuildingSettings MakeSettings()
{
    TStateBuildingSettings s;
    s.OverloadThreshold = TLoadFactor{90};

    return s;
}

THubReport MakeHubReport(
    const std::string& hub,
    const std::string& dc,
    TEpoch::UnderlyingType epoch,
    TLoadFactor::UnderlyingType load,
    std::unordered_map<TPartitionId, TPartitionWeight> weights)
{
    THubReport r;
    r.Endpoint = THubEndpoint(hub);
    r.DC = THubDC{dc};
    r.Epoch = TEpoch{epoch};
    r.LoadFactor = TLoadFactor{load};
    r.PartitionWeights = std::move(weights);
    return r;
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TEST(TCoordinationState, EpochIsTakenFromPartitionMap)
{
    TCoordinationState state(
        MakePartitionMap(),
        {},
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(state.GetEpoch(), TEpoch{42});
}

TEST(TCoordinationState, InitializesPartitionStates)
{
    TCoordinationState state(
        MakePartitionMap(),
        {},
        MakeContext(),
        MakeSettings());

    const auto& p1 = state.GetPartitionState(TPartitionId(1));
    EXPECT_EQ(p1.Id, TPartitionId(1));
    EXPECT_EQ(p1.AssignedHub, THubEndpoint("hub-a"));
    ASSERT_TRUE(p1.ObservedWeight.has_value());
    EXPECT_EQ(p1.ObservedWeight.value(), TPartitionWeight(100));
    EXPECT_EQ(p1.ExpectedWeightGrowth, TPartitionWeight(0));
    EXPECT_FALSE(p1.MigrationCooldown.has_value());

    const auto& p2 = state.GetPartitionState(TPartitionId(2));
    ASSERT_TRUE(p2.MigrationCooldown.has_value());
    EXPECT_EQ(p2.MigrationCooldown.value(), TEpoch{10});

    EXPECT_THROW(state.GetPartitionState(TPartitionId(5)), std::out_of_range);
}

TEST(TCoordinationState, InitializesAveragePartitionWeight)
{
    TCoordinationState state({}, {}, {}, {});

    EXPECT_EQ(state.GetAveragePartitionWeight(), TPartitionWeight(0));
}

TEST(TCoordinationState, CalculateAveragePartitionWeightFromContext)
{
    TCoordinationState state(
        MakePartitionMap(),
        {},
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(state.GetAveragePartitionWeight(), TPartitionWeight(180));
}

TEST(TCoordinationState, CalculatesAveragePartitionWeightFromSnapshot)
{
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport(
            "hub-a",
            "myt",
            42,
            50,
            {
                {TPartitionId(1), TPartitionWeight(300)},
                {TPartitionId(4), TPartitionWeight(500)},
            }));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(state.GetAveragePartitionWeight(), TPartitionWeight(400));
}

TEST(TCoordinationState, HandlesEmptySnapshot) {
    TCoordinationState state(MakePartitionMap(), {}, MakeContext(), MakeSettings());

    EXPECT_TRUE(state.GetHubStates().empty());
}

TEST(TCoordinationState, BuildsHubStatesFromSnapshot)
{
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport(
            "hub-a",
            "myt",
            42,
            50,
            {
                {TPartitionId(1), TPartitionWeight(90)},
                {TPartitionId(4), TPartitionWeight(30)},
            }));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(state.GetHubStates().size(), 1);
    const auto& hub = state.GetHubState(THubEndpoint("hub-a"));
    EXPECT_EQ(hub.Endpoint, THubEndpoint("hub-a"));
    EXPECT_EQ(hub.DC, THubDC("myt"));
    EXPECT_EQ(hub.Status, EHubStatus::HEALTHY);
    EXPECT_EQ(hub.TotalPartitions, 2);
    EXPECT_EQ(hub.PartitionsWeight, TPartitionWeight(120));
    EXPECT_EQ(hub.ExpectedWeightGrowth, TPartitionWeight(10));
}

TEST(TCoordinationState, ComputesExpectedWeightGrowth)
{
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport(
            "hub-a",
            "myt",
            42,
            30,
            {
                // cached was 100
                {TPartitionId(1), TPartitionWeight(60)},
            }));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        MakeSettings());

    const auto& p1 = state.GetPartitionState(TPartitionId(1));
    EXPECT_EQ(p1.ExpectedWeightGrowth, TPartitionWeight(40));

    const auto& hub = state.GetHubState(THubEndpoint("hub-a"));
    EXPECT_EQ(hub.ExpectedWeightGrowth, TPartitionWeight(40));
}

TEST(TCoordinationState, HubStatusDrainingBySettings)
{
    auto settings = MakeSettings();
    settings.BlockedHubs.insert(THubEndpoint("hub-a"));

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport("hub-a", "myt", 42, 10, {}));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        settings);

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-a")).Status,
        EHubStatus::DRAINING);
}

TEST(TCoordinationState, HubStatusLagged)
{
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport("hub-a", "myt", 41, 10, {}));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-a")).Status,
        EHubStatus::LAGGED);
}

TEST(TCoordinationState, HubStatusOverloaded)
{
    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport("hub-a", "myt", 42, 95, {}));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        MakeSettings());

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-a")).Status,
        EHubStatus::OVERLOADED);
}

TEST(TCoordinationState, HubStatusDrainingPriority)
{
    auto settings = MakeSettings();
    settings.BlockedHubs.insert(THubEndpoint("hub-a"));

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport("hub-a", "myt", 42, 95, {}));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        settings);

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-a")).Status,
        EHubStatus::DRAINING);
}

TEST(TCoordinationState, DrainingHubsByBlockedDC)
{
    auto settings = MakeSettings();
    settings.BlockedDCs.insert(THubDC("myt"));

    TCoordinationState::TClusterSnapshot snapshot;
    snapshot.emplace(
        THubEndpoint("hub-a"),
        MakeHubReport("hub-a", "myt", 42, 10, {}));
    snapshot.emplace(
        THubEndpoint("hub-b"),
        MakeHubReport("hub-b", "myt", 42, 35, {}));
    snapshot.emplace(
        THubEndpoint("hub-c"),
        MakeHubReport("hub-c", "sas", 42, 24, {}));

    TCoordinationState state(
        MakePartitionMap(),
        snapshot,
        MakeContext(),
        settings);

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-a")).Status,
        EHubStatus::DRAINING);

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-b")).Status,
        EHubStatus::DRAINING);

    EXPECT_EQ(
        state.GetHubState(THubEndpoint("hub-c")).Status,
        EHubStatus::HEALTHY);
}

///////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
