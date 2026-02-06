#include "leader_config.hpp"

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

NCore::NDomain::THubEndpoint Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::THubEndpoint>)
{
    return NCore::NDomain::THubEndpoint{
        value.As<NCore::NDomain::THubEndpoint::UnderlyingType>()
    };
}

NCore::NDomain::THubDC Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::THubDC>)
{
    return NCore::NDomain::THubDC{
        value.As<NCore::NDomain::THubDC::UnderlyingType>()
    };
}

NCore::NDomain::TEpoch Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TEpoch>)
{
    return NCore::NDomain::TEpoch{
        value.As<NCore::NDomain::TEpoch::UnderlyingType>()
    };
}

NCore::NDomain::TPartitionWeight Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TPartitionWeight>)
{
    return NCore::NDomain::TPartitionWeight{
        value.As<NCore::NDomain::TPartitionWeight::UnderlyingType>()
    };
}

NCore::NDomain::TLoadFactor Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TLoadFactor>)
{
    return NCore::NDomain::TLoadFactor{
        value.As<NCore::NDomain::TLoadFactor::UnderlyingType>()
    };
}

TLeaderSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TLeaderSettings>)
{
    const auto& state = value["state_building_settings"];
    const auto& balancing = value["balancing_settings"];

    NCore::NDomain::TStateBuildingSettings stateSettings{
        .BlockedDCs = state["blocked_dcs"]
            .As<std::unordered_set<NCore::NDomain::THubDC>>(),
        .BlockedHubs = state["blocked_hubs"]
            .As<std::unordered_set<NCore::NDomain::THubEndpoint>>(),
        .OverloadThreshold = NCore::NDomain::TLoadFactor{state["overload_threshold"]
            .As<NCore::NDomain::TLoadFactor::UnderlyingType>()},
    };

    NCore::TBalancingSettings balancingSettings{
        .MaxRebalancePhases = balancing["max_rebalance_phases"]
            .As<std::size_t>(),
        .MigratingWeightLimit = balancing["migrating_weight_limit"]
            .As<NCore::NDomain::TPartitionWeight>(),
        .MinLoadFactorDelta = balancing["min_load_factor_delta"]
            .As<NCore::NDomain::TLoadFactor>(),
        .MigrationBudgetThreshold = balancing["migration_budget_threshold"]
            .As<NCore::NDomain::TPartitionWeight>(),
        .BalancingThresholdCV = balancing["balancing_threshold_cv"]
            .As<std::uint64_t>(),
        .BalancingTargetCV = balancing["balancing_target_cv"]
            .As<std::uint64_t>(),
        .MinMigrationCooldown = balancing["min_migration_cooldown"]
            .As<NCore::NDomain::TEpoch>(),
        .MigrationWeightPenaltyCoeff = balancing["migration_weight_penalty_coeff"]
            .As<double>(),
    };

    return TLeaderSettings{
        std::chrono::seconds{value["coordination_period_seconds"].As<std::uint32_t>()},
        value["default_partitions_amount"].As<std::size_t>(),
        std::move(stateSettings),
        std::move(balancingSettings),
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
