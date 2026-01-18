
#pragma once

#include "load_factor_predictor.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_state.hpp>

#include <set>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

class TPartitionBalancer {
public:
    struct TBalancingResult {
        NDomain::TPartitionMap PartitionMap;
        NDomain::TCoordinationContext Context;
    };

public:
    explicit TPartitionBalancer(TLoadFactorPredictorPtr predictor);

    NDomain::TPartitionMap BalancePartitions(
        const NDomain::TCoordinationState& state,
        const NDomain::TBalancingSettings& settings) const;

private:
    using TWeightedHub = std::pair<NDomain::TLoadFactor, NDomain::THubEndpoint>;
    using TSortedHubs = std::set<TWeightedHub>;
    using THubEndpoints = std::unordered_set<NDomain::THubEndpoint>;
    using TWeightedPartitions = std::vector<std::pair<NDomain::TPartitionLoad, NDomain::TPartitionId>>;
    using TAssignedPartitions = std::vector<std::pair<NDomain::TPartitionId, NDomain::THubEndpoint>>;
    using TSeparatedPartitions = std::pair<TAssignedPartitions, TWeightedPartitions>;
    using TMigratingLoad = std::unordered_map<NDomain::THubEndpoint, std::pair<std::size_t, NDomain::TPartitionLoad::UnderlyingType>>;

private:
    std::pair<THubEndpoints, TSortedHubs> CollectActiveHubs(const NDomain::TCoordinationState& state) const;

    TSeparatedPartitions SeparatePartitions(
        const NDomain::TCoordinationState& state,
        const THubEndpoints& activeHubs) const;

    TMigratingLoad AssignOrphanedPartitions(
        const NDomain::TCoordinationState& state,
        const TWeightedPartitions& partitions,
        TSortedHubs& hubSortedSet,
        TAssignedPartitions& assignedPartitions) const;

    NDomain::TPartitionLoad AccumulateMigratingLoad(
        const TMigratingLoad& migratingLoad) const;

    void RebalancePartitions() const;

private:
    TLoadFactorPredictorPtr LoadFactorPredictor_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
