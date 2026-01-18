#include "partition_balancer.hpp"

#include <utils/math.hpp>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

TPartitionBalancer::TPartitionBalancer(TLoadFactorPredictorPtr predictor)
    : LoadFactorPredictor_(std::move(predictor))
{ }

NDomain::TPartitionMap TPartitionBalancer::BalancePartitions(
    const NDomain::TCoordinationState& state,
    const NDomain::TBalancingSettings& settings) const
{
    auto [activeHubs, sortedHubs] = CollectActiveHubs(state);

    auto [assignedPartitions, orphanedPartitions] = SeparatePartitions(state, activeHubs);

    auto migratingLoad = AssignOrphanedPartitions(
        state,
        orphanedPartitions,
        sortedHubs,
        assignedPartitions);

    auto totalMigratingLoad = AccumulateMigratingLoad(migratingLoad);

    if (totalMigratingLoad < settings.MigratingLoadLimit) {
        std::vector<std::uint64_t> loadFactors;
        loadFactors.reserve(sortedHubs.size());

        for (const auto& hub : sortedHubs) {
            loadFactors.push_back(hub.first.GetUnderlying());
        }

        const auto cv = NUtils::NMath::CalculateCV(loadFactors);
        if (cv >= settings.BalancingThresholdCV) {
            RebalancePartitions(sortedHubs, assignedPartitions, cv, state, settings);
        }
        // rebalance partitions if needed

        // calculate cv from sortedHubs
        // if cv is > threshold
        //     rebalance
    }

    // TODO расставлять ласт мигрейшн эпоху передвинутым - но это потом просто сравнивая партишн мапы - либо тут но ставить эпоху с которой можно будет перемещать (при бесхозной срок назначается новый)

    std::sort(assignedPartitions.begin(), assignedPartitions.end());

    return {
        .Partitions = std::move(assignedPartitions),
        .Epoch = NDomain::TEpoch{state.GetEpoch().GetUnderlying() + 1},
    };
}

std::pair<TPartitionBalancer::THubEndpoints, TPartitionBalancer::TSortedHubs> TPartitionBalancer::CollectActiveHubs(
    const NDomain::TCoordinationState& state) const
{
    TSortedHubs sortedHubs;
    THubEndpoints activeHubs;

    for (const auto& [endpoint, hubState] : state.GetHubStates()) {
        if (hubState.GetStatus() == NDomain::EHubStatus::DRAINING ||
            hubState.GetStatus() == NDomain::EHubStatus::LAGGED)
        {
            continue;
        }

        activeHubs.emplace(endpoint);

        TPredictionParams predictionParams {
            .TotalPartitions = hubState.GetTotalPartitions(),
            .PartitionsLoad = hubState.GetPartitionsLoad(),
            .OriginalLoadFactor = hubState.GetLoadFactor(),
        };
        
        NDomain::TLoadFactor forecastedLoad = LoadFactorPredictor_->PredictLoadFactor(
            hubState.GetLoadFactor(), 
            hubState.GetExpectedLoadGrowth(),
            predictionParams);
        
        sortedHubs.emplace(forecastedLoad, endpoint);
    }

    return {activeHubs, sortedHubs};
}

TPartitionBalancer::TSeparatedPartitions TPartitionBalancer::SeparatePartitions(
    const NDomain::TCoordinationState& state,
    const THubEndpoints& activeHubs) const
{
    TWeightedPartitions orphanedPartitions;
    TAssignedPartitions assignedPartitions;

    for (const auto& [id, partitionState] : state.GetPartitionStates()) {
        auto hub = partitionState.GetAssignedHub();
        
        if (!activeHubs.contains(hub)) {
            orphanedPartitions.emplace_back(partitionState.GetObservedLoad(), id);
        } else {
            assignedPartitions.emplace_back(id, hub);
        }
    }

    std::sort(orphanedPartitions.begin(), orphanedPartitions.end(), std::greater{});

    return {assignedPartitions, orphanedPartitions};
}

TPartitionBalancer::TMigratingLoad TPartitionBalancer::AssignOrphanedPartitions(
    const NDomain::TCoordinationState& state,
    const TWeightedPartitions& orphanedPartitions,
    TSortedHubs& sortedHubs,
    TAssignedPartitions& assignedPartitions) const
{
    TMigratingLoad addedLoad;

    for (const auto& [partitionLoad, partitionId] : orphanedPartitions) {
        if (sortedHubs.empty()) {
            break;
        }
        auto it = sortedHubs.begin(); 
        auto [loadFactor, hub] = *it; // скопировалось?
        sortedHubs.erase(it);

        auto& hubState = state.GetHubState(hub).value().get();
        
        assignedPartitions.emplace_back(partitionId, hub);

        TPredictionParams params {
            .TotalPartitions = hubState.GetTotalPartitions() + addedLoad[hub].first,
            .PartitionsLoad = NDomain::TPartitionLoad{
                hubState.GetPartitionsLoad().GetUnderlying() + addedLoad[hub].second
            },
            .OriginalLoadFactor = hubState.GetLoadFactor(),
        };
        loadFactor = LoadFactorPredictor_->PredictLoadFactor(loadFactor, partitionLoad, params);
        
        addedLoad[hub].first++;
        addedLoad[hub].second += partitionLoad.GetUnderlying();

        sortedHubs.emplace(loadFactor, hub);
    }

    return addedLoad;
}

 NDomain::TPartitionLoad TPartitionBalancer::AccumulateMigratingLoad(
    const TMigratingLoad& migratingLoad) const
{
    NDomain::TPartitionLoad::UnderlyingType totalMigratingLoad = 0;

    for (const auto& [hub, hubLoad] : migratingLoad) {
        totalMigratingLoad += hubLoad.second;
    }

    return NDomain::TPartitionLoad{totalMigratingLoad};
}

void RebalancePartitions();

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore:
