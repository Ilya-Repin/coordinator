#include "kesus_gateway.hpp"

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>
#include <infra/serializer/serializer.hpp>

#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>
#include <userver/ydb/exceptions.hpp>

#include <cstdint>

namespace {

////////////////////////////////////////////////////////////////////////////////

const std::uint64_t SEMAPHORE_LIMIT = 1;

////////////////////////////////////////////////////////////////////////////////

}  // anonymous namespace

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

TKesusGateway::TKesusGateway(
    std::shared_ptr<userver::ydb::CoordinationClient> coordinationClient,
    const std::string& coordinationNode,
    const std::string& semaphoreName,
    const bool initialSetup)
    : SemaphoreName_(semaphoreName)
    , CoordinationSession_(nullptr)
{
    if (initialSetup) {
        InitialSetup(*coordinationClient, coordinationNode);
    }

    auto session = coordinationClient->StartSession(coordinationNode, {});
    CoordinationSession_ = std::make_unique<userver::ydb::CoordinationSession>(std::move(session));
}

NCore::NDomain::TPartitionMap TKesusGateway::GetPartitionMap() const
{
    NYdb::NCoordination::TDescribeSemaphoreSettings describeSettings;
    auto description = CoordinationSession_->DescribeSemaphore(SemaphoreName_, describeSettings);

    userver::formats::json::Value json;
    try {
        json = userver::formats::json::FromString(description.GetData());
    } catch(std::exception& ex) {
        LOG_ERROR() << "Can't parse json: " << ex;
        return {};
    }

    return DeserializePartitionMap(json);
}

void TKesusGateway::BroadcastPartitionMap(const NCore::NDomain::TPartitionMap& partitionMap) const
{
    auto json = SerializePartitionMap(partitionMap);
    CoordinationSession_->UpdateSemaphore(
        SemaphoreName_,
        userver::formats::json::ToStableString(json));
}

void TKesusGateway::InitialSetup(
    userver::ydb::CoordinationClient& coordinationClient,
    const std::string& coordinationNode) const
{
    NYdb::NCoordination::TCreateNodeSettings createNodeSettings;
    try {
        coordinationClient.CreateNode(coordinationNode, createNodeSettings);
    } catch (const userver::ydb::YdbResponseError& ex) {
        LOG_WARNING() << "Could not create coordination node: " << ex;
    }

    auto session = coordinationClient.StartSession(coordinationNode, {});
    try {
        session.CreateSemaphore(SemaphoreName_, SEMAPHORE_LIMIT);
    } catch (const userver::ydb::YdbResponseError& ex) {
        LOG_WARNING() << "Could not create semaphore: " << ex;
    }
    session.Close();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
