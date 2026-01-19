#pragma once

#include <core/coordination_gateway.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationGatewayComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "coordination-gateway";

    TCoordinationGatewayComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NCore::ICoordinationGateway& GetGateway();

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    std::unique_ptr<NCore::ICoordinationGateway> Gateway_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
