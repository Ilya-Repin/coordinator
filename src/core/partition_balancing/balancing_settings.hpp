#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <cstdint>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

struct TBalancingSettings {
    std::size_t MaxRebalancePhases{};
    NDomain::TPartitionWeight MigratingWeightLimit;
    NDomain::TLoadFactor MinLoadFactorDelta;
    NDomain::TPartitionWeight MigrationBudgetThreshold;
    std::uint64_t BalancingThresholdCV{};
    std::uint64_t BalancingTargetCV{};
    NDomain::TEpoch MinMigrationCooldown;
    double MigrationWeightPenaltyCoeff{};
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
