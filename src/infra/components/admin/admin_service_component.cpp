#include "admin_service_component.hpp"

#include <infra/components/coordination/coordination_gateway_component.hpp>
#include <infra/components/coordination/coordination_repository_component.hpp>

#include <userver/components/component.hpp>
#include <userver/components/component_context.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TAdminServiceComponent::TAdminServiceComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context)
{
    auto& coordinationRepository =
        context.FindComponent<NComponents::TCoordinationRepositoryComponent>().GetRepository();
    auto& coordinationGateway =
        context.FindComponent<NComponents::TCoordinationGatewayComponent>().GetGateway();
 
 
    Service_ = std::make_unique<NApp::NService::TAdminService>(coordinationRepository, coordinationGateway);
}

NApp::NService::TAdminService& TAdminServiceComponent::GetService()
{
    return *Service_;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
