#include "coordination_gateway_component.hpp"

#include <infra/coordination_gateway/kesus_coordination_gateway.hpp>

#include <userver/components/component.hpp>
#include <userver/ydb/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TCoordinationGatewayComponent::TCoordinationGatewayComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto dbname = config["dbname"].As<std::string>();
    auto coordinationNode = config["coordination-node"].As<std::string>();
    auto partitionMapSemaphore = config["partition-map-semaphore"].As<std::string>();
    auto discoverySemaphore = config["discovery-semaphore"].As<std::string>();
    auto initialSetup = config["initial-setup"].As<bool>(true);

    auto coordinationClient = context.FindComponent<userver::ydb::YdbComponent>().GetCoordinationClient(dbname);

    Gateway_ = std::make_unique<NInfra::NGateway::TKesusCoordinationGateway>(
        std::move(coordinationClient),
        coordinationNode,
        partitionMapSemaphore,
        discoverySemaphore,
        initialSetup);
}

NCore::NDomain::ICoordinationGateway& TCoordinationGatewayComponent::GetGateway()
{
    return *Gateway_;
}

userver::yaml_config::Schema TCoordinationGatewayComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for coordination gateway
additionalProperties: false
properties:
    dbname:
        type: string
        description: the key of the database within ydb component (NOT the
            actual database path)
    coordination-node:
        type: string
        description: name of the coordination node within the database
    partition-map-semaphore:
        type: string
        description: name of the partition map semaphore within the coordination node
    discovery-semaphore:
        type: string
        description: name of the discovery semaphore within the coordination node
    initial-setup:
        type: boolean
        description: if true, then create the coordination node and the semaphore
            unless they already exist
        default: true
)");
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
