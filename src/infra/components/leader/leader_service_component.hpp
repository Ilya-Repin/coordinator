#pragma once

#include <app/services/leader/leader_service.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TLeaderServiceComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "leader-service";

    TLeaderServiceComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NApp::NService::TLeaderService& GetService();

private:
    std::unique_ptr<NApp::NService::TLeaderService> Service_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
