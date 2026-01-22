#pragma once

#include "load_factor_predictor.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_context.hpp>
#include <core/coordination/coordination_state.hpp>
#include <core/partition/partition_map.hpp>

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

    TBalancingResult BalancePartitions(
        const NDomain::TCoordinationState& state,
        const NDomain::TBalancingSettings& settings) const;

private:
    TLoadFactorPredictorPtr LoadFactorPredictor_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
