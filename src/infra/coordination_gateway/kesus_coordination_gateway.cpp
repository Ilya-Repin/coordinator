#include "kesus_coordination_gateway.hpp"

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>
#include <infra/serializer/serializer.hpp>
#include <infra/dynconfig/leader/leader_config.hpp>

#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>
#include <userver/ydb/exceptions.hpp>

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace {

////////////////////////////////////////////////////////////////////////////////

constexpr inline std::uint64_t PARTITION_MAP_SEMAPHORE_LIMIT = 1;
constexpr inline std::uint64_t DISCOVERY_SEMAPHORE_LIMIT = std::numeric_limits<std::uint64_t>::max();

////////////////////////////////////////////////////////////////////////////////

}  // anonymous namespace

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

TKesusCoordinationGateway::TKesusCoordinationGateway(
    std::shared_ptr<userver::ydb::CoordinationClient> coordinationClient,
    userver::dynamic_config::Source configSource,
    const std::string& coordinationNode,
    const std::string& partitionMapSemaphore,
    const std::string& discoverySemaphore,
    const bool initialSetup)
    : CoordinationSession_(nullptr)
    , ConfigSource_(std::move(configSource))
    , PartitionMapSemaphore_(partitionMapSemaphore)
    , DiscoverySemaphore_(discoverySemaphore)
{
    if (initialSetup) {
        InitialSetup(*coordinationClient, coordinationNode);
    }

    auto session = coordinationClient->StartSession(coordinationNode, {});
    CoordinationSession_ = std::make_unique<userver::ydb::CoordinationSession>(std::move(session));
}

NCore::NDomain::TPartitionMap TKesusCoordinationGateway::GetPartitionMap() const
{
    NYdb::NCoordination::TDescribeSemaphoreSettings describeSettings;
    auto description = CoordinationSession_->DescribeSemaphore(PartitionMapSemaphore_, describeSettings);

    userver::formats::json::Value json;
    try {
        json = userver::formats::json::FromString(description.GetData());
    } catch(std::exception& ex) {
        LOG_ERROR() << "Can't parse json: " << ex;
        throw NCore::NDomain::TInvalidPartitionMapException("Can't parse json");
    }

    try {
        return DeserializePartitionMap(json);
    } catch (std::exception& ex) {
        LOG_ERROR() << "Can't deserialize partition map: " << ex;
        throw NCore::NDomain::TInvalidPartitionMapException("Can't deserialize partition map");
    }
}

void TKesusCoordinationGateway::BroadcastPartitionMap(const NCore::NDomain::TPartitionMap& partitionMap) const
{
    auto json = SerializePartitionMap(partitionMap);
    CoordinationSession_->UpdateSemaphore(
        PartitionMapSemaphore_,
        userver::formats::json::ToStableString(json));
}

std::vector<NCore::NDomain::THubEndpoint> TKesusCoordinationGateway::GetHubDiscovery() const
{
    NYdb::NCoordination::TDescribeSemaphoreSettings describeSettings;
    describeSettings.IncludeOwners(true);

    auto description = CoordinationSession_->DescribeSemaphore(DiscoverySemaphore_, describeSettings);

    std::vector<NCore::NDomain::THubEndpoint> hubs;
    hubs.reserve(description.GetCount());

    for (const auto& owner : description.GetOwners()) {
        auto hub = NCore::NDomain::THubEndpoint{owner.GetData()};
        hubs.emplace_back(std::move(hub));
    }

    return hubs;
}

void TKesusCoordinationGateway::InitialSetup(
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
        const auto snapshot = ConfigSource_.GetSnapshot();
        const auto config = snapshot[LEADER_CONFIG];
        auto partitionMap = NCore::NDomain::BuildStartingPartitionMap(config.DefaultPartitionsAmount);
    
        session.CreateSemaphore(
            PartitionMapSemaphore_,
            PARTITION_MAP_SEMAPHORE_LIMIT);
            // userver::formats::json::ToStableString(SerializePartitionMap(partitionMap)));
        session.UpdateSemaphore(
            PartitionMapSemaphore_,
            userver::formats::json::ToStableString(SerializePartitionMap(partitionMap)));
    } catch (const userver::ydb::YdbResponseError& ex) {
        LOG_WARNING() << "Could not create partition map semaphore: " << ex;
    }

    try {
        session.CreateSemaphore(DiscoverySemaphore_, DISCOVERY_SEMAPHORE_LIMIT);
    } catch (const userver::ydb::YdbResponseError& ex) {
        LOG_WARNING() << "Could not create discovery semaphore: " << ex;
    }

    session.Close();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
