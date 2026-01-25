#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

using TAssignedPartition = std::pair<TPartitionId, THubEndpoint>;

struct TPartitionMap {
    std::vector<TAssignedPartition> Partitions; // sorted by partition id
    TEpoch Epoch;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
