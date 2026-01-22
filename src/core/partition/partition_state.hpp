#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TPartitionState {
    TPartitionId Id;
    THubEndpoint AssignedHub;
    TPartitionWeight ExpectedWeightGrowth;
    std::optional<TPartitionWeight> ObservedWeight;
    std::optional<TEpoch> MigrationCooldown;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
