#include "coordination_dist_lock_component.hpp"

#include <infra/components/coordination/coordination_gateway_component.hpp>

#include <userver/components/component_context.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/logging/log.hpp>

#include <chrono>

namespace {

////////////////////////////////////////////////////////////////////////////////

const auto DEFAULT_COORDINATION_PAUSE_SECONDS = std::chrono::seconds(10); // TODO replace with dynamic config

////////////////////////////////////////////////////////////////////////////////

}  // anonymous namespace

namespace NCoordinator::NInfra::NComponents {

////////////////////////////////////////////////////////////////////////////////

TCoordinationDistLockComponent::TCoordinationDistLockComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : DistLockComponentBase(config, context)
    , Gateway_(context.FindComponent<TCoordinationGatewayComponent>().GetGateway())
{
    Start();
}

TCoordinationDistLockComponent::~TCoordinationDistLockComponent()
{
    Stop();
}

void TCoordinationDistLockComponent::DoWork() {
    while (!userver::engine::current_task::ShouldCancel()) {
        auto partitionMap = Gateway_.GetPartitionMap().value_or(NCore::NDomain::TPartitionMap{});
        
        ++(partitionMap.Epoch.GetUnderlying());
        Gateway_.BroadcastPartitionMap(partitionMap);

        userver::engine::InterruptibleSleepFor(DEFAULT_COORDINATION_PAUSE_SECONDS); // TODO replace with dynamic config
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
