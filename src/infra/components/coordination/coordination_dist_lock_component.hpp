#pragma once

#include <core/coordination/coordination_gateway.hpp>

#include <userver/ydb/dist_lock/component_base.hpp>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationDistLockComponent
    : public userver::ydb::DistLockComponentBase
{
public:
    static constexpr std::string_view kName = "coordination-dist-lock";

    TCoordinationDistLockComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);
    
    ~TCoordinationDistLockComponent();

    void DoWork() final;

private:
    NCore::NDomain::ICoordinationGateway& Gateway_; // Replace with service with core
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
