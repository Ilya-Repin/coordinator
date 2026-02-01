#pragma once

#include <app/services/admin/admin_service.hpp>

#include <userver/components/loggable_component_base.hpp>

#include <memory>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TAdminServiceComponent
    : public userver::components::LoggableComponentBase
{
public:
    static constexpr std::string_view kName = "admin-service";

    TAdminServiceComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    NApp::NService::TAdminService& GetService();

private:
    std::unique_ptr<NApp::NService::TAdminService> Service_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
