
#pragma once

#include <core/domain/common/coordination_params.hpp>
#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>

#include <unordered_map>
#include <unordered_set>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TCoordinationContext {
    std::unordered_set<NDomain::THubDC> BlockedDCs;
    std::unordered_set<NDomain::THubEndpoint> BlockedHubs;
    std::unordered_map<TPartitionId, TEpoch> PartitionsLastMigrations;
    std::unordered_map<TPartitionId, TPartitionLoad> CachedPartitionLoads; 
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
