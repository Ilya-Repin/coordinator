#pragma once

#include <core/coordination/coordination_repository.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationRepositoryComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "coordination-repository";

    TCoordinationRepositoryComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NCore::NDomain::ICoordinationRepository& GetRepository();

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    std::unique_ptr<NCore::NDomain::ICoordinationRepository> Repository_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
