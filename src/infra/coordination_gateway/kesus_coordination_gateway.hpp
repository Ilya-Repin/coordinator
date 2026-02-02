#pragma once

#include <core/common/hub_params.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/partition/partition_map.hpp>

#include <userver/ydb/coordination.hpp>

#include <memory>
#include <vector>

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

class TKesusCoordinationGateway
    : public NCore::NDomain::ICoordinationGateway 
{
public:
    TKesusCoordinationGateway(
        std::shared_ptr<userver::ydb::CoordinationClient> coordinationClient,
        const std::string& coordinationNode,
        const std::string& partitionMapSemaphore,
        const std::string& discoverySemaphore,
        const bool initialSetup);

    NCore::NDomain::TPartitionMap GetPartitionMap() const override;
    void BroadcastPartitionMap(const NCore::NDomain::TPartitionMap& partitionMap) const override;

    std::vector<NCore::NDomain::THubEndpoint> GetHubDiscovery() const override;

private:
    void InitialSetup(
        userver::ydb::CoordinationClient& coordinationClient,
        const std::string& coordinationNode) const;

private:
    const std::string PartitionMapSemaphore_;
    const std::string DiscoverySemaphore_;
    std::unique_ptr<userver::ydb::CoordinationSession> CoordinationSession_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
