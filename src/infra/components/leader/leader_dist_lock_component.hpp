#pragma once

#include <app/services/leader/leader_service.hpp>

#include <userver/ydb/dist_lock/component_base.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TLeaderDistLockComponent
    : public userver::ydb::DistLockComponentBase
{
public:
    static constexpr std::string_view kName = "leader-dist-lock";

    TLeaderDistLockComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);
    
    ~TLeaderDistLockComponent();

    void DoWork() final;

private:
    NApp::NService::TLeaderService& Service_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
