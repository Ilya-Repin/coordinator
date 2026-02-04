#include "leader_dist_lock_component.hpp"

#include "leader_service_component.hpp"

#include <infra/dynconfig/leader/leader_config.hpp>

#include <userver/components/component_context.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/logging/log.hpp>

#include <chrono>

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TLeaderDistLockComponent::TLeaderDistLockComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : DistLockComponentBase(config, context)
    , Service_(context.FindComponent<TLeaderServiceComponent>().GetService())
    , ConfigSource_(context.FindComponent<userver::components::DynamicConfig>().GetSource())
{
    Start();
}

TLeaderDistLockComponent::~TLeaderDistLockComponent()
{
    Stop();
}

void TLeaderDistLockComponent::DoWork() {
    while (!userver::engine::current_task::ShouldCancel()) {
        const auto snapshot = ConfigSource_.GetSnapshot();
        const auto config = snapshot[LEADER_CONFIG];

        NApp::NDto::TCoordinationRequest request{
            .DefaultPartitionsAmount = config.DefaultPartitionsAmount,
            .StateBuildingSettings = config.StateBuildingSettings,
            .BalancingSettings = config.BalancingSettings,
        };
        Service_.Coordinate(std::move(request));

        userver::engine::InterruptibleSleepFor(config.CoordinationPeriod);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
