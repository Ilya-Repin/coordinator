#pragma once

#include "balancing_settings.hpp"
#include "load_factor_predictor.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_state.hpp>

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <optional>

namespace NCoordinator::NCore::NDetail {

////////////////////////////////////////////////////////////////////////////////

using TWeightedHub = std::pair<NDomain::TLoadFactor, NDomain::THubEndpoint>;
using TSortedHubs = std::set<TWeightedHub>;
using THubEndpoints = std::unordered_set<NDomain::THubEndpoint>;
using TWeightedPartition = std::pair<NDomain::TPartitionWeight, NDomain::TPartitionId>;
using TWeightedPartitions = std::vector<TWeightedPartition>;
using TAssignedPartitions = std::vector<NDomain::TAssignedPartition>;
using TSeparatedPartitions = std::pair<TAssignedPartitions, TWeightedPartitions>;
using TMigratingWeight = std::unordered_map<NDomain::THubEndpoint, std::pair<std::size_t, NDomain::TPartitionWeight>>;
using TMigratingPartitions = std::unordered_map<NDomain::TPartitionId, std::optional<NDomain::THubEndpoint>>; // partition and its origin
using THubPartitions = std::unordered_map<NDomain::THubEndpoint, std::set<TWeightedPartition>>;

struct TMigrationContext {
    TMigratingPartitions MigratingPartitions;
    NDomain::TPartitionWeight TotalMigratingWeight;
};

////////////////////////////////////////////////////////////////////////////////

std::pair<THubEndpoints, TSortedHubs> CollectActiveHubs(
    const NDomain::TCoordinationState& state,
    const TLoadFactorPredictorPtr& loadFactorPredictor);

TSeparatedPartitions SeparatePartitions(const NDomain::TCoordinationState& state, const THubEndpoints& activeHubs);

TMigratingWeight AssignOrphanedPartitions(
    const NDomain::TCoordinationState& state,
    const TWeightedPartitions& orphanedPartitions,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    TSortedHubs& sortedHubs,
    TAssignedPartitions& assignedPartitions);

NDomain::TPartitionWeight AccumulateMigratingWeight(
    const TMigratingWeight& migratingWeight);

void RebalancePartitions(
    TSortedHubs& sortedHubs,
    TAssignedPartitions& assignedPartitions,
    TMigrationContext& migrationContext,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings);

void ExecuteRebalancingPhase(
    TSortedHubs& sortedHubs, 
    THubPartitions& hubPartitions, 
    TMigrationContext& migrationContext,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings);

TPredictionParams BuildPredictionParams(
    const bool increasing,
    const std::set<TWeightedPartition>& partitions,
    const NDomain::THubState& state);

NDomain::TCoordinationContext BuildCoordinationContext(
    TMigratingPartitions& migratingPartitions,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDetail
