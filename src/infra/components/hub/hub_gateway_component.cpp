#include "hub_gateway_component.hpp"

#include <infra/hub_gateway/hub_gateway.hpp>

#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/storage/component.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

THubGatewayComponent::THubGatewayComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto& httpClient = context.FindComponent<userver::components::HttpClient>().GetHttpClient();
    auto configSource = context.FindComponent<userver::components::DynamicConfig>().GetSource();

    Gateway_ = std::make_unique<NInfra::NGateway::THubGateway>(httpClient, std::move(configSource));
}

NCore::NDomain::IHubGateway& THubGatewayComponent::GetGateway()
{
    return *Gateway_;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
