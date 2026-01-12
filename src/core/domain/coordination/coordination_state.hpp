
#pragma once

#include <core/domain/common/coordination_params.hpp>
#include <core/domain/coordination/coordination_context.hpp>
#include <core/domain/coordination/coordination_settings.hpp>
#include <core/domain/hub/hub_report.hpp>
#include <core/domain/hub/hub_state.hpp>
#include <core/domain/partition/partition_map.hpp>
#include <core/domain/partition/partition_state.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationState {
public:
    using TClusterSnapshot = std::unordered_map<THubEndpoint, THubReport>;
    using TPartitionStates = std::unordered_map<TPartitionId, TPartitionState>;
    using THubStates = std::unordered_map<THubEndpoint, THubState>;

public:
    TCoordinationState(
        const TPartitionMap& partitionMap,
        const TClusterSnapshot& snapshot,
        const TCoordinationContext& context,
        const TCoordinationSettings& settings);

    const TPartitionState& GetPartitionState(const TPartitionId partition) const;
    const THubState& GetHubState(const THubEndpoint& hub) const; 

    const TPartitionStates& GetPartitionStates() const;
    const THubStates& GetHubStates() const;

private:
    void InitializePartitionStates(
        const TPartitionMap& partitionMap,
        const TCoordinationContext& context);

    void ApplyClusterSnapshot(
        const TClusterSnapshot& snapshot,
        const TCoordinationContext& context,
        const TCoordinationSettings& settings);

    NDomain::EHubStatus DetermineHubStatus(
        const NDomain::THubReport& hubReport,
        const TCoordinationContext& context,
        const TCoordinationSettings& settings) const;

private:
    TEpoch Epoch_;
    TPartitionStates PartitionStates_;
    THubStates HubStates_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
