#include "coordination_state.hpp"

#include <optional>
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
    AveragePartitionWeight_ = TPartitionWeight{0};

    InitializePartitionStates(partitionMap, context);
    ApplyClusterSnapshot(snapshot, settings);
}

TEpoch TCoordinationState::GetEpoch() const
{
    return Epoch_;
}

TPartitionWeight TCoordinationState::GetAveragePartitionWeight() const
{
    return AveragePartitionWeight_;
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
    std::size_t partitionsWithObservedWeight = 0;
    auto sumPartitionWeight = TPartitionWeight{0};

    for (const auto& [partition, hub] : partitionMap.Partitions) {
        auto& state = PartitionStates_[partition];
        state.Id = partition;
        state.AssignedHub = hub;

        auto itWeight = context.PartitionWeights.find(partition);
        state.ObservedWeight = itWeight != context.PartitionWeights.end()
            ? std::make_optional(itWeight->second)
            : std::nullopt;

        auto itMigration = context.PartitionCooldowns.find(partition);
        state.MigrationCooldown = itMigration != context.PartitionCooldowns.end()
            ? std::make_optional(itMigration->second)
            : std::nullopt;

        if (const auto& weight = state.ObservedWeight) {
            sumPartitionWeight += weight.value();
            ++partitionsWithObservedWeight;
        }
    }

    if (partitionsWithObservedWeight > 0) {
        AveragePartitionWeight_ = TPartitionWeight{
            sumPartitionWeight.GetUnderlying() / partitionsWithObservedWeight
        }; // integer division
    }
}

void TCoordinationState::ApplyClusterSnapshot(
    const TClusterSnapshot& snapshot,
    const TStateBuildingSettings& settings)
{
    std::unordered_map<THubEndpoint, TPartitionWeight> expectedWeightGrowths;

    for (const auto& [hub, report] : snapshot) {
        THubState& state = HubStates_[hub];

        state.Endpoint = report.Endpoint;
        state.DC = report.DC;
        state.Status = DetermineHubStatus(report, settings);
        state.LoadFactor = report.LoadFactor;

        auto sumPartitionWeight = TPartitionWeight{0};

        for (const auto& [id, weight] : report.PartitionWeights) {
            sumPartitionWeight += weight;

            if (auto it = PartitionStates_.find(id); it != PartitionStates_.end()) {
                auto& [_, partitionState] = *it;
                auto cachedWeight = partitionState.ObservedWeight;

                if (cachedWeight.has_value() && cachedWeight.value() > weight) {
                    auto expectedWeightGrowth = cachedWeight.value() - weight;
                    
                    expectedWeightGrowths[partitionState.AssignedHub] += expectedWeightGrowth;
                    partitionState.ExpectedWeightGrowth = expectedWeightGrowth;
                }

                partitionState.ObservedWeight = weight;
            }
        }

        state.PartitionsWeight = sumPartitionWeight;
        state.TotalPartitions = report.PartitionWeights.size();
    }

    auto sumPartitionWeight = TPartitionWeight{0};
    std::size_t totalPartitions = 0;

    for (auto& [hub, state] : HubStates_) {
        if (auto it = expectedWeightGrowths.find(hub); it != expectedWeightGrowths.end()) {
            state.ExpectedWeightGrowth += it->second;
        }
        sumPartitionWeight += state.PartitionsWeight;
        totalPartitions += state.TotalPartitions;
    }

    if (totalPartitions > 0) {
        auto averageWeight = TPartitionWeight{
            sumPartitionWeight.GetUnderlying() / totalPartitions
        }; // integer division

        AveragePartitionWeight_ = std::max(averageWeight, AveragePartitionWeight_);
    }
}

NDomain::EHubStatus TCoordinationState::DetermineHubStatus(
    const NDomain::THubReport& hubReport,
    const TStateBuildingSettings& settings) const
{
    if (settings.BlockedDCs.contains(hubReport.DC) || settings.BlockedHubs.contains(hubReport.Endpoint)) {
        return NDomain::EHubStatus::DRAINING;
    }
    if (hubReport.Epoch != Epoch_) {
        return NDomain::EHubStatus::LAGGED;
    }
    if (hubReport.LoadFactor >= settings.OverloadThreshold) {
        return NDomain::EHubStatus::OVERLOADED;
    }

    return NDomain::EHubStatus::HEALTHY;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
