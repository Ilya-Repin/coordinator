#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>

#include <cstdint>
#include <vector>
// TODO to delete
namespace NCoordinator::NCore::NDomain {

struct TPartitionMap {
    TEpoch Epoch;
    std::vector<std::pair<TPartitionId, THubEndpoint>> Partitions;
}; // TODO Delete

} 