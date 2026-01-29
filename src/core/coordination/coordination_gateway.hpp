#pragma once

#include <core/common/hub_params.hpp>
#include <core/partition/partition_map.hpp>

#include <optional>
#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class ICoordinationGateway {
public:
    virtual std::optional<TPartitionMap> GetPartitionMap() const = 0;
    virtual void BroadcastPartitionMap(const TPartitionMap& partitionMap) const = 0;

    virtual std::vector<THubEndpoint> GetHubDiscovery() const = 0;

    virtual ~ICoordinationGateway() = default;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
