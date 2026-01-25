#pragma once

#include <core/coordination_gateway.hpp>
#include <core/partition/partition_map.hpp>

#include <userver/ydb/coordination.hpp>

#include <memory>

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

class TKesusGateway
    : public NCore::ICoordinationGateway 
{
public:
    TKesusGateway(
        std::shared_ptr<userver::ydb::CoordinationClient> coordinationClient,
        const std::string& coordinationNode,
        const std::string& semaphoreName,
        const bool initialSetup);

    NCore::NDomain::TPartitionMap GetPartitionMap() const override;
    void BroadcastPartitionMap(const NCore::NDomain::TPartitionMap& partitionMap) const override;

private:
    void InitialSetup(
        userver::ydb::CoordinationClient& coordinationClient,
        const std::string& coordinationNode) const;

private:
    const std::string SemaphoreName_;
    std::unique_ptr<userver::ydb::CoordinationSession> CoordinationSession_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
