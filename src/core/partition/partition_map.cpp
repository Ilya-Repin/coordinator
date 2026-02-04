#include "partition_map.hpp"

#include <core/hash_ring/hash_ring.hpp>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TPartitionMap BuildStartingPartitionMap(std::size_t partitionsAmount)
{
    NCore::NDomain::THashRing hashRing(partitionsAmount);
    auto partitions = hashRing.GetAllPartitions();

    TPartitionMap result;
    result.Partitions.reserve(partitions.size());

    for (const auto& partition : partitions) {
        result.Partitions.emplace_back(partition, NCore::NDomain::THubEndpoint{});
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
