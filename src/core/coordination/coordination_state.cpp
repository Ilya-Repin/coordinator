#include "coordination_state.hpp"

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TCoordinationState::TCoordinationState(
    const TPartitionMap& partitionMap,
    const TClusterSnapshot& snapshot,
    const TCoordinationContext& context,
    const TStateBuildingSettings& settings)
{
    Epoch_ = partitionMap.Epoch;
    InitializePartitionStates(partitionMap, context);
    ApplyClusterSnapshot(snapshot, settings);
}

TEpoch TCoordinationState::GetEpoch() const
{
    return Epoch_;
}

std::optional<std::reference_wrapper<const TPartitionState>> TCoordinationState::GetPartitionState(const TPartitionId partition) const
{
    if (auto it = PartitionStates_.find(partition); it != PartitionStates_.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

std::optional<std::reference_wrapper<const THubState>> TCoordinationState::GetHubState(const THubEndpoint& hub) const
{
    if (auto it = HubStates_.find(hub); it != HubStates_.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

const TCoordinationState::TPartitionStates& TCoordinationState::GetPartitionStates() const
{
    return PartitionStates_;
}

const TCoordinationState::THubStates& TCoordinationState::GetHubStates() const
{
    return HubStates_;
}

void TCoordinationState::InitializePartitionStates(
    const TPartitionMap& partitionMap,
    const TCoordinationContext& context)
{
    for (const auto& [partition, hub] : partitionMap.Partitions) {
        TPartitionState& state = PartitionStates_[partition];
        state.SetId(partition);
        state.SetAssignedHub(hub);
        
        auto itMigration = context.PartitionLastMigrations.find(partition);
        state.SetLastMigrationEpoch(itMigration != context.PartitionLastMigrations.end()
            ? itMigration->second
            : TEpoch{0});

        auto itLoad = context.PartitionLoads.find(partition);
        state.SetObservedLoad(itLoad != context.PartitionLoads.end()
            ? itLoad->second
            : TPartitionLoad{0});
    }
}

void TCoordinationState::ApplyClusterSnapshot(
    const TClusterSnapshot& snapshot,
    const TStateBuildingSettings& settings)
{
    std::unordered_map<THubEndpoint, TPartitionLoad::UnderlyingType> expectedLoadGrowths;

    for (const auto& [hub, report] : snapshot) {
        THubState& state = HubStates_[hub];

        state.SetEndpoint(report.GetEndpoint());
        state.SetDC(report.GetDC());
        state.SetStatus(DetermineHubStatus(report, settings));
        state.SetLoadFactor(report.GetLoadFactor());

        TPartitionLoad::UnderlyingType sumPartitionLoad = 0;

        for (const auto& [id, load] : report.GetPartitionLoads()) {
            sumPartitionLoad += load.GetUnderlying();

            if (auto it = PartitionStates_.find(id); it != PartitionStates_.end()) {
                auto cachedLoad = it->second.GetObservedLoad();

                if (cachedLoad > load) {
                    expectedLoadGrowths[it->second.GetAssignedHub()] +=
                        cachedLoad.GetUnderlying() - load.GetUnderlying();
                }

                it->second.SetObservedLoad(load);
            }
        }

        state.SetPartitionsLoad(TPartitionLoad{sumPartitionLoad});
        state.SetTotalPartitions(report.GetPartitionLoads().size());
    }

    for (auto& [hub, state] : HubStates_) {
        if (auto it = expectedLoadGrowths.find(hub); it != expectedLoadGrowths.end()) {
            state.AddToExpectedLoadGrowth(TPartitionLoad{it->second});
        }
    }
}

NDomain::EHubStatus TCoordinationState::DetermineHubStatus(
    const NDomain::THubReport& hubReport,
    const TStateBuildingSettings& settings) const
{
    if (settings.BlockedDCs.contains(hubReport.GetDC()) || settings.BlockedHubs.contains(hubReport.GetEndpoint())) {
        return NDomain::EHubStatus::DRAINING;
    }
    if (hubReport.GetEpoch() != Epoch_) {
        return NDomain::EHubStatus::LAGGED;
    }
    if (hubReport.GetLoadFactor() >= settings.OverloadThreshold) {
        return NDomain::EHubStatus::OVERLOADED;
    }

    return NDomain::EHubStatus::HEALTHY;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
