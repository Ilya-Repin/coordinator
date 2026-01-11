#pragma once

#include <core/domain/common/coordination_params.hpp>
#include <core/domain/partition/partition.hpp>

#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TPartitionMap {
    std::vector<TPartition> Partitions{}; // sorted by partition id
    TEpoch Epoch{};
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
