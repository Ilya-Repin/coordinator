#include "hub_gateway_component.hpp"

#include <infra/hub_gateway/hub_gateway.hpp>

#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

THubGatewayComponent::THubGatewayComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto& httpClient = context.FindComponent<userver::components::HttpClient>().GetHttpClient();

    Gateway_ = std::make_unique<NInfra::NGateway::THubGateway>(httpClient);
}

NCore::NDomain::IHubGateway& THubGatewayComponent::GetGateway()
{
    return *Gateway_;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
