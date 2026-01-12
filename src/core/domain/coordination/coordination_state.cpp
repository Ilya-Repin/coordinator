#include "coordination_state.hpp"

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TCoordinationState::TCoordinationState(
    const TPartitionMap& partitionMap,
    const TClusterSnapshot& snapshot,
    const TCoordinationContext& context,
    const TCoordinationSettings& settings)
{
    Epoch_ = partitionMap.Epoch;
    InitializePartitionStates(partitionMap, context);
    ApplyClusterSnapshot(snapshot, context, settings);

}

const TPartitionState& TCoordinationState::GetPartitionState(const TPartitionId partition) const
{
    return PartitionStates_.at(partition);
}

const THubState& TCoordinationState::GetHubState(const THubEndpoint& hub) const
{
    return HubStates_.at(hub);
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
        state.SetHub(hub);
        
        auto itMigration = context.PartitionsLastMigrations.find(partition);
        state.SetLastMigrationEpoch(itMigration != context.PartitionsLastMigrations.end()
            ? itMigration->second
            : TEpoch{0});

        auto itLoad = context.CachedPartitionLoads.find(partition);
        state.SetObservedLoad(itLoad != context.CachedPartitionLoads.end()
            ? itLoad->second
            : TPartitionLoad{0});
    }
}

void TCoordinationState::ApplyClusterSnapshot(
    const TClusterSnapshot& snapshot,
    const TCoordinationContext& context,
    const TCoordinationSettings& settings)
{
    for (const auto& [hub, report] : snapshot) {
        THubState& state = HubStates_[hub];

        state.SetEndpoint(report.GetEndpoint());
        state.SetDC(report.GetDC());

        state.SetStatus(DetermineHubStatus(report, context, settings));
        
        state.SetLoadFactor(report.GetLoadFactor());

        std::uint64_t sumPartitionLoad = 0;
        std::size_t activePartitions = 0;
        for (const auto& [id, partitionData] : report.GetPartitionLoads()) {
            const auto& [load, status] = partitionData;
            if (status == EPartitionStatus::ACTIVE) {
                ++activePartitions;
                sumPartitionLoad += load.GetUnderlying();
                PartitionStates_[id].SetObservedLoad(load);
            } else if (auto it = PartitionStates_.find(id); it != PartitionStates_.end()) {
                if (auto hubId = it->second.GetHub()) {
                    HubStates_[hubId.value()].AddToExpectedLoadGrowth(load);
                }
            }
        }

        TPartitionImpact impact{sumPartitionLoad / activePartitions};
        state.SetPartitionImpact(impact);
    }
}


NDomain::EHubStatus TCoordinationState::DetermineHubStatus(
    const NDomain::THubReport& hubReport,
    const TCoordinationContext& context,
    const TCoordinationSettings& settings) const
{
    if (context.BlockedDCs.contains(hubReport.GetDC()) || context.BlockedHubs.contains(hubReport.GetEndpoint())) {
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
