#include "balancing_impl.hpp"

#include "load_factor_predictor.hpp"

#include <utils/math.hpp>

namespace NCoordinator::NCore::NDetail {

////////////////////////////////////////////////////////////////////////////////

std::pair<THubEndpoints, TSortedHubs> CollectActiveHubs(
    const NDomain::TCoordinationState& state,
    const TLoadFactorPredictorPtr& loadFactorPredictor)
{
    TSortedHubs sortedHubs;
    THubEndpoints activeHubs;

    for (const auto& [endpoint, hubState] : state.GetHubStates()) {
        if (hubState.Status == NDomain::EHubStatus::DRAINING ||
            hubState.Status == NDomain::EHubStatus::LAGGED)
        {
            continue;
        }

        activeHubs.emplace(endpoint);

        TPredictionParams predictionParams {
            .Increasing = true,
            .TotalPartitions = hubState.TotalPartitions,
            .PartitionsWeight = hubState.PartitionsWeight,
            .OriginalLoadFactor = hubState.LoadFactor,
        };
        
        NDomain::TLoadFactor forecastedLoad = loadFactorPredictor->PredictLoadFactor(
            hubState.LoadFactor, 
            hubState.ExpectedWeightGrowth,
            predictionParams);
        
        sortedHubs.emplace(forecastedLoad, endpoint);
    }

    return {activeHubs, sortedHubs};
}

TSeparatedPartitions SeparatePartitions(
    const NDomain::TCoordinationState& state,
    const THubEndpoints& activeHubs)
{
    TWeightedPartitions orphanedPartitions;
    TAssignedPartitions assignedPartitions;

    for (const auto& [id, partitionState] : state.GetPartitionStates()) {
        auto hub = partitionState.AssignedHub;
        
        if (!activeHubs.contains(hub)) {
            auto weight = partitionState.ObservedWeight.value_or(state.GetAveragePartitionWeight()) +
                partitionState.ExpectedWeightGrowth;

            orphanedPartitions.emplace_back(weight, id);
        } else {
            assignedPartitions.emplace_back(id, hub);
        }
    }

    std::sort(orphanedPartitions.begin(), orphanedPartitions.end(), std::greater{});

    return {assignedPartitions, orphanedPartitions};
}

TMigratingWeight AssignOrphanedPartitions(
    const NDomain::TCoordinationState& state,
    const TWeightedPartitions& orphanedPartitions,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    TSortedHubs& sortedHubs,
    TAssignedPartitions& assignedPartitions)
{
    TMigratingWeight migratingWeight;

    for (const auto& [partitionWeight, partitionId] : orphanedPartitions) {
        if (sortedHubs.empty()) {
            break;
        }

        auto it = sortedHubs.begin(); 
        auto [loadFactor, hub] = *it;
        sortedHubs.erase(it);

        const auto& hubState = state.GetHubState(hub);
        
        assignedPartitions.emplace_back(partitionId, hub);

        TPredictionParams params {
            .Increasing = true,
            .TotalPartitions = hubState.TotalPartitions + migratingWeight[hub].first,
            .PartitionsWeight = hubState.PartitionsWeight + migratingWeight[hub].second,
            .OriginalLoadFactor = hubState.LoadFactor,
        };
        loadFactor = loadFactorPredictor->PredictLoadFactor(loadFactor, partitionWeight, params);
        
        migratingWeight[hub].first++;
        migratingWeight[hub].second += partitionWeight;

        sortedHubs.emplace(loadFactor, hub);
    }

    return migratingWeight;
}

NDomain::TPartitionWeight AccumulateMigratingWeight(
    const TMigratingWeight& migratingWeight)
{
    auto totalMigratingWeight = NDomain::TPartitionWeight{0};

    for (const auto& [hub, hubWeight] : migratingWeight) {
        totalMigratingWeight += hubWeight.second;
    }

    return totalMigratingWeight;
}

void RebalancePartitions(
    TSortedHubs& sortedHubs,
    TAssignedPartitions& assignedPartitions,
    TMigrationContext& migrationContext,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings)
{
    const auto sortedHubsProj = [](const auto& hub) { return hub.first.GetUnderlying(); };

    auto cv = NUtils::NMath::CalculateCV(sortedHubs, sortedHubsProj);
    if (cv < settings.BalancingThresholdCV) {
        return;
    }

    THubPartitions hubPartitions;
    for (const auto& [id, hub] : assignedPartitions) {
        auto weight = state.GetAveragePartitionWeight();
        auto partition = state.GetPartitionState(id);

        weight = partition.ObservedWeight.value_or(weight) + partition.ExpectedWeightGrowth;
        
        hubPartitions[hub].emplace(weight, id);
    }

    for (std::size_t i = 0; i < settings.MaxRebalancePhases; ++i) {
        auto migrationBudget = settings.MigratingWeightLimit - migrationContext.TotalMigratingWeight;
        if (migrationBudget <= settings.MigrationBudgetThreshold) {
            break;
        }

        auto cv = NUtils::NMath::CalculateCV(sortedHubs, sortedHubsProj);
        if (cv <= settings.BalancingTargetCV) {
            break;
        }

        ExecuteRebalancingPhase(sortedHubs, hubPartitions, migrationContext, loadFactorPredictor, state, settings);
    }

    for (auto it = hubPartitions.begin(); it != hubPartitions.end(); ++it) {
        auto& weightedPartitions = it->second;
        const auto& hub = it->first;

        while (!weightedPartitions.empty()) {
            auto node = weightedPartitions.extract(weightedPartitions.begin());

            assignedPartitions.emplace_back(std::move(node.value().second), hub);
        }
    }
}

void ExecuteRebalancingPhase(
    TSortedHubs& sortedHubs, 
    THubPartitions& hubPartitions, 
    TMigrationContext& migrationContext,
    const TLoadFactorPredictorPtr& loadFactorPredictor,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings)
{
    TSortedHubs processedHubs;
    while (sortedHubs.size() >= 2) {
        auto maxLoadedHub = sortedHubs.extract(std::prev(sortedHubs.end()));
        auto minLoadedHub = sortedHubs.extract(sortedHubs.begin());

        auto& [maxLoadFactor, maxHub] = maxLoadedHub.value();
        auto& [minLoadFactor, minHub] = minLoadedHub.value();

        if (maxLoadFactor - minLoadFactor < settings.MinLoadFactorDelta) {
            processedHubs.insert(std::move(maxLoadedHub));
            processedHubs.insert(std::move(minLoadedHub));
            break;
        }

        auto& candidates = hubPartitions[maxHub];

        for (auto it = candidates.begin(); it != candidates.end();) {
            const auto& [partitionWeight, partitionId] = *it;

            auto migrationCooldown = state.GetPartitionState(partitionId).MigrationCooldown;

            if (migrationCooldown.has_value() && migrationCooldown.value() > state.GetEpoch()) {
                ++it;
                continue;
            }

            if (!migrationContext.MigratingPartitions.contains(it->second)) {
                auto weight = migrationContext.TotalMigratingWeight + partitionWeight;
                if (weight > settings.MigratingWeightLimit) {
                    ++it;
                    continue;
                }
            }

            auto maxParams = BuildPredictionParams(false, hubPartitions[maxHub], state.GetHubState(maxHub));
            auto nextMaxLoadFactor = loadFactorPredictor->PredictLoadFactor(maxLoadFactor, partitionWeight, maxParams);
            
            auto minParams = BuildPredictionParams(true, hubPartitions[minHub], state.GetHubState(minHub));
            auto nextMinLoadFactor = loadFactorPredictor->PredictLoadFactor(minLoadFactor, partitionWeight, minParams);

            auto currentDelta = maxLoadFactor - minLoadFactor;
            auto nextDelta = nextMaxLoadFactor - nextMinLoadFactor;

            if (nextDelta < currentDelta && nextMaxLoadFactor >= nextMinLoadFactor) {
                maxLoadFactor = nextMaxLoadFactor; 
                minLoadFactor = nextMinLoadFactor;
                hubPartitions[minHub].emplace(partitionWeight, partitionId);

                auto migrationIt = migrationContext.MigratingPartitions.find(partitionId);

                if (migrationIt == migrationContext.MigratingPartitions.end()) {
                    migrationContext.TotalMigratingWeight += partitionWeight;    
                    migrationContext.MigratingPartitions.emplace(partitionId, maxHub);
                } else if (migrationIt->second.has_value() && migrationIt->second.value() == minHub) {
                    migrationContext.TotalMigratingWeight -= partitionWeight;    
                    migrationContext.MigratingPartitions.erase(migrationIt);
                }

                it = candidates.erase(it);
            } else {
                ++it;
            }
        }
        
        processedHubs.insert(std::move(maxLoadedHub));
        processedHubs.insert(std::move(minLoadedHub));
    }

    sortedHubs.merge(processedHubs);
}

TPredictionParams BuildPredictionParams(
    const bool increasing,
    const std::set<TWeightedPartition>& partitions,
    const NDomain::THubState& state)
{
    return TPredictionParams{
        .Increasing = increasing,
        .TotalPartitions = partitions.size(),
        .PartitionsWeight = 
            std::accumulate(partitions.begin(), partitions.end(), NDomain::TPartitionWeight{0},
                [](auto sum, const auto& p) { return sum + p.first; }),
        .OriginalLoadFactor = state.LoadFactor,
    };
}

NDomain::TCoordinationContext BuildCoordinationContext(
    TMigratingPartitions& migratingPartitions,
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings)
{
    NDomain::TCoordinationContext context;

    for (const auto& [id, partitionState] : state.GetPartitionStates()) {
        if (partitionState.ObservedWeight.has_value()) {
            context.PartitionWeights[id] = partitionState.ObservedWeight.value();
        }
        if (partitionState.MigrationCooldown.has_value() && partitionState.MigrationCooldown.value() > state.GetEpoch()) {
            context.PartitionCooldowns[id] = partitionState.MigrationCooldown.value();
        }
    }

    for (const auto& [id, _] : migratingPartitions) {
        auto partition = state.GetPartitionState(id);
        auto weight = partition.ObservedWeight.value_or(state.GetAveragePartitionWeight()) +
            partition.ExpectedWeightGrowth;

        auto weightPenalty = static_cast<NDomain::TEpoch::UnderlyingType>(
            weight.GetUnderlying() * settings.MigrationWeightPenaltyCoeff);

        context.PartitionCooldowns[id] = 
            state.GetEpoch() + settings.MinMigrationCooldown + NDomain::TEpoch(weightPenalty + 1);
    }

    return context;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDetail
