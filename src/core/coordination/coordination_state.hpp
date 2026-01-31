#pragma once

#include "coordination_context.hpp"
#include "coordination_settings.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/hub/hub_report.hpp>
#include <core/hub/hub_state.hpp>
#include <core/partition/partition_map.hpp>
#include <core/partition/partition_state.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationState {
public:
    using TClusterSnapshot = std::vector<THubReport>;
    using TPartitionStates = std::unordered_map<TPartitionId, TPartitionState>;
    using THubStates = std::unordered_map<THubEndpoint, THubState>;

public:
    TCoordinationState(
        const TPartitionMap& partitionMap,
        const TClusterSnapshot& snapshot,
        const TCoordinationContext& context,
        const TStateBuildingSettings& settings);

    TEpoch GetEpoch() const;

    TPartitionWeight GetAveragePartitionWeight() const;

    const TPartitionState& GetPartitionState(
        const TPartitionId partition) const;

    const THubState& GetHubState(
        const THubEndpoint& hub) const; 

    const TPartitionStates& GetPartitionStates() const;

    const THubStates& GetHubStates() const;

private:
    void InitializePartitionStates(
        const TPartitionMap& partitionMap,
        const TCoordinationContext& context);

    void ApplyClusterSnapshot(
        const TClusterSnapshot& snapshot,
        const TStateBuildingSettings& settings);

    NDomain::EHubStatus DetermineHubStatus(
        const NDomain::THubReport& hubReport,
        const TStateBuildingSettings& settings) const;

private:
    TEpoch Epoch_;
    TPartitionWeight AveragePartitionWeight_;
    TPartitionStates PartitionStates_;
    THubStates HubStates_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
