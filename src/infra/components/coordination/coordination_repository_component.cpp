#include "coordination_repository_component.hpp"

#include <infra/coordination_repository/ydb_coordination_repository.hpp>

#include <userver/components/component.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/ydb/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TCoordinationRepositoryComponent::TCoordinationRepositoryComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto dbname = config["dbname"].As<std::string>();

    auto tableClient = context.FindComponent<userver::ydb::YdbComponent>().GetTableClient(dbname);
    auto configSource = context.FindComponent<userver::components::DynamicConfig>().GetSource();

    Repository_ = std::make_unique<NInfra::NRepository::TYdbCoordinationRepository>(
        std::move(tableClient),
        std::move(configSource));
}

NCore::NDomain::ICoordinationRepository& TCoordinationRepositoryComponent::GetRepository()
{
    return *Repository_;
}

userver::yaml_config::Schema TCoordinationRepositoryComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(
      R"(
type: object
description: Component for coordination repository
additionalProperties: false
properties:
    dbname:
        type: string
        description: the key of the database within ydb component (NOT the
            actual database path)
)");
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
