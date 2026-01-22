
#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TCoordinationContext {
    std::unordered_map<TPartitionId, TEpoch> PartitionCooldowns;
    std::unordered_map<TPartitionId, TPartitionWeight> PartitionWeights; 
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
