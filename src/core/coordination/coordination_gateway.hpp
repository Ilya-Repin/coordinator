#pragma once

#include <core/common/hub_params.hpp>
#include <core/partition/partition_map.hpp>

#include <stdexcept>
#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class TInvalidPartitionMapException
    : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class ICoordinationGateway {
public:
    virtual TPartitionMap GetPartitionMap() const = 0;
    virtual void BroadcastPartitionMap(const TPartitionMap& partitionMap) const = 0;

    virtual std::vector<THubEndpoint> GetHubDiscovery() const = 0;

    virtual ~ICoordinationGateway() = default;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
