#include "get_partition.hpp"

#include <core/hash_ring/hash_ring.hpp>
#include <core/partition/partition_map.hpp>

#include <userver/logging/log.hpp>

#include <fmt/format.h>

#include <vector>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

TGetPartitionUseCase::TGetPartitionUseCase(NCore::NDomain::ICoordinationGateway& coordinationGateway)
    : CoordinationGateway_(coordinationGateway)
{ }

NDto::TGetPartitionResponse TGetPartitionUseCase::Execute(const NDto::TGetPartitionRequest& request) const
{
    NCore::NDomain::TPartitionMap partitionMap;
    
    try {
        partitionMap = CoordinationGateway_.GetPartitionMap();
    } catch (std::exception& ex) {
        throw TGetPartitionTemporaryUnavailable(fmt::format("Failed to get partition map: {}", ex.what()));
    }

    std::vector<NCore::NDomain::TPartitionId> partitions;
    partitions.reserve(partitionMap.Partitions.size());
    
    for (const auto& [partition, hub] : partitionMap.Partitions) {
        partitions.push_back(partition);
    }

    NCore::NDomain::THashRing hashRing(partitions);
    auto partition = hashRing.GetPartition(request.ChannelId);

    NDto::TGetPartitionResponse response{
        .Partition = partition,
    };

    return response;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
