#include "partition_balancer.hpp"

#include "balancing_impl.hpp"

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

TPartitionBalancer::TPartitionBalancer(TLoadFactorPredictorPtr predictor)
    : LoadFactorPredictor_(std::move(predictor))
{ }

TPartitionBalancer::TBalancingResult TPartitionBalancer::BalancePartitions(
    const NDomain::TCoordinationState& state,
    const TBalancingSettings& settings) const
{
    auto [activeHubs, sortedHubs] = NDetail::CollectActiveHubs(state, LoadFactorPredictor_);

    auto [assignedPartitions, orphanedPartitions] = NDetail::SeparatePartitions(state, activeHubs);

    NDetail::TMigratingPartitions migratingPartitions;
    migratingPartitions.reserve(orphanedPartitions.size());
    for (const auto& [_, id] : orphanedPartitions) {
        migratingPartitions.emplace(id, std::nullopt);
    }

    auto migratingWeight = NDetail::AssignOrphanedPartitions(
        state,
        orphanedPartitions,
        LoadFactorPredictor_,
        sortedHubs,
        assignedPartitions);

    auto totalMigratingWeight = NDetail::AccumulateMigratingWeight(migratingWeight);
    if (totalMigratingWeight < settings.MigratingWeightLimit) {
        NDetail::TMigrationContext migrationContext {
            .MigratingPartitions = std::move(migratingPartitions),
            .TotalMigratingWeight = std::move(totalMigratingWeight),
        };
        NDetail::RebalancePartitions(
            sortedHubs,
            assignedPartitions,
            migrationContext,
            LoadFactorPredictor_,
            state,
            settings);
    }

    std::sort(assignedPartitions.begin(), assignedPartitions.end());

    auto context = NDetail::BuildCoordinationContext(migratingPartitions, state, settings);

    auto partitionMap = NDomain::TPartitionMap{
        .Partitions = std::move(assignedPartitions),
        .Epoch = state.GetEpoch() + NDomain::TEpoch{1},
    };

    return TBalancingResult{
        .PartitionMap = std::move(partitionMap),
        .Context = std::move(context),
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
