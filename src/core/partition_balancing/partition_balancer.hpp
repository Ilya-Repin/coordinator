#pragma once

#include "balancing_settings.hpp"
#include "load_factor_predictor.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_context.hpp>
#include <core/coordination/coordination_state.hpp>
#include <core/partition/partition_map.hpp>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

struct TBalancingResult {
    NDomain::TPartitionMap PartitionMap;
    NDomain::TCoordinationContext Context;
};

class TPartitionBalancer {
public:
    explicit TPartitionBalancer(ILoadFactorPredictor& predictor);

    TBalancingResult BalancePartitions(
        const NDomain::TCoordinationState& state,
        const TBalancingSettings& settings) const;

private:
    ILoadFactorPredictor& LoadFactorPredictor_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
