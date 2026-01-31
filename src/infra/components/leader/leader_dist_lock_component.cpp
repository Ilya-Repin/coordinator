#include "leader_dist_lock_component.hpp"

#include "leader_service_component.hpp"

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

TLeaderDistLockComponent::TLeaderDistLockComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : DistLockComponentBase(config, context)
    , Service_(context.FindComponent<TLeaderServiceComponent>().GetService())
{
    Start();
}

TLeaderDistLockComponent::~TLeaderDistLockComponent()
{
    Stop();
}

void TLeaderDistLockComponent::DoWork() {
    while (!userver::engine::current_task::ShouldCancel()) {

        // TODO replace to settings from dynconfig
        NCore::NDomain::TStateBuildingSettings stateBuildingSettings{
            .BlockedDCs = {},
            .BlockedHubs = {},
            .OverloadThreshold = NCore::NDomain::TLoadFactor{90},
        };

        NCore::TBalancingSettings balancingSettings{
            .MaxRebalancePhases = 5,
            .MigratingWeightLimit = NCore::NDomain::TPartitionWeight{1500},
            .MinLoadFactorDelta = NCore::NDomain::TLoadFactor{10},
            .MigrationBudgetThreshold = NCore::NDomain::TPartitionWeight{100},
            .BalancingThresholdCV = 25,
            .BalancingTargetCV = 5,
            .MinMigrationCooldown = NCore::NDomain::TEpoch{5},
            .MigrationWeightPenaltyCoeff = 0.5,
        };

        NApp::NDto::TCoordinationRequest request{
            .StateBuildingSettings = std::move(stateBuildingSettings),
            .BalancingSettings = std::move(balancingSettings),
        };
        Service_.Coordinate(std::move(request));

        userver::engine::InterruptibleSleepFor(DEFAULT_COORDINATION_PAUSE_SECONDS); // TODO replace with dynamic config
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NComponents
