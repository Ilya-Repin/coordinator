#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <unordered_set>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TStateBuildingSettings {
    std::unordered_set<NDomain::THubDC> BlockedDCs;
    std::unordered_set<NDomain::THubEndpoint> BlockedHubs;
    TLoadFactor OverloadThreshold;
};

struct TBalancingSettings { // move to partition balancer
    std::size_t MaxRebalancePhases;
    NDomain::TPartitionWeight MigratingWeightLimit;
    NDomain::TLoadFactor MinLoadFactorDelta;
    NDomain::TPartitionWeight MigrationBudgetThreshold;
    std::uint64_t BalancingThresholdCV;
    std::uint64_t BalancingTargetCV;
    NDomain::TEpoch MinMigrationCooldown;
    double MigrationWeightPenaltyCoeff;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
