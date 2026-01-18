
#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TCoordinationContext {
    std::unordered_map<TPartitionId, TEpoch> PartitionLastMigrations;
    std::unordered_map<TPartitionId, TPartitionLoad> PartitionLoads; 
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
