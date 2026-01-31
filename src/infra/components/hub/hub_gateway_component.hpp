#pragma once

#include <core/hub/hub_gateway.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class THubGatewayComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "hub-gateway";

    THubGatewayComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NCore::NDomain::IHubGateway& GetGateway();

private:
    std::unique_ptr<NCore::NDomain::IHubGateway> Gateway_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
