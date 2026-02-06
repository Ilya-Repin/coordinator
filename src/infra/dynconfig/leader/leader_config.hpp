#pragma once

#include <core/coordination/coordination_settings.hpp>
#include <core/partition_balancing/balancing_settings.hpp>

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/formats/json/value.hpp>

#include <chrono>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

struct TLeaderSettings {
    std::chrono::seconds CoordinationPeriod{10};
    std::size_t DefaultPartitionsAmount{100};
    NCore::NDomain::TStateBuildingSettings StateBuildingSettings;
    NCore::TBalancingSettings BalancingSettings;
};

inline const userver::dynamic_config::Key<TLeaderSettings> LEADER_CONFIG{
    "LEADER_CONFIG",
    userver::dynamic_config::DefaultAsJsonString{R"(
{
  "coordination_period_seconds": 10,
  "default_partitions_amount": 100,
  "state_building_settings": {
    "blocked_dcs": [],
    "blocked_hubs": [],
    "overload_threshold": 90
  },
  "balancing_settings": {
    "max_rebalance_phases": 5,
    "migrating_weight_limit": 1500,
    "min_load_factor_delta": 10,
    "migration_budget_threshold": 100,
    "balancing_threshold_cv": 25,
    "balancing_target_cv": 5,
    "min_migration_cooldown": 5,
    "migration_weight_penalty_coeff": 0.5
  }
}
)"}};

NCore::NDomain::THubEndpoint Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::THubEndpoint>);

NCore::NDomain::THubDC Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::THubDC>);

NCore::NDomain::TEpoch Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TEpoch>);

NCore::NDomain::TPartitionWeight Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TPartitionWeight>);

NCore::NDomain::TLoadFactor Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<NCore::NDomain::TLoadFactor>);

TLeaderSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TLeaderSettings>);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
