#include "leader_service_component.hpp"

#include <infra/components/hub/hub_gateway_component.hpp>
#include <infra/components/coordination/coordination_gateway_component.hpp>
#include <infra/components/coordination/coordination_repository_component.hpp>
#include <infra/components/partition_balancing/load_factor_predictor_component.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TLeaderServiceComponent::TLeaderServiceComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto& coordinationGateway = context.FindComponent<NComponents::TCoordinationGatewayComponent>().GetGateway();
    auto& coordinationRepository =
        context.FindComponent<NComponents::TCoordinationRepositoryComponent>().GetRepository();
    auto& hubGateway = context.FindComponent<NComponents::THubGatewayComponent>().GetGateway();
    auto& predictor = context.FindComponent<NComponents::TLoadFactorPredictorComponent>().GetPredictor();

    Service_ = std::make_unique<NApp::NService::TLeaderService>(
        coordinationGateway,
        coordinationRepository,
        hubGateway,
        predictor);
}

NApp::NService::TLeaderService& TLeaderServiceComponent::GetService()
{
    return *Service_;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
