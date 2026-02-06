#include "get_partition_map.hpp"

#include <userver/logging/log.hpp>

#include <fmt/format.h>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

TGetPartitionMapUseCase::TGetPartitionMapUseCase(NCore::NDomain::ICoordinationGateway& coordinationGateway)
    : CoordinationGateway_(coordinationGateway)
{ }

NDto::TGetPartitionMapResponse TGetPartitionMapUseCase::Execute() const
{
    NCore::NDomain::TPartitionMap partitionMap;
    
    try {
        partitionMap = CoordinationGateway_.GetPartitionMap();
    } catch (std::exception& ex) {
        throw TGetPartitionMapTemporaryUnavailable(fmt::format("Failed to get partition map: {}", ex.what()));
    }

    NDto::TGetPartitionMapResponse response{
        .PartitionMap = std::move(partitionMap),
    };

    return response;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
