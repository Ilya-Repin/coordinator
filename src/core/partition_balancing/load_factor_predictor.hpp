#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_state.hpp>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

struct TPredictionParams {
    bool Increasing{};
    std::size_t TotalPartitions{};
    NDomain::TPartitionWeight PartitionsWeight;
    NDomain::TLoadFactor OriginalLoadFactor;
};

class ILoadFactorPredictor {
public:
    virtual NDomain::TLoadFactor PredictLoadFactor(
        const NDomain::TLoadFactor loadFactor,
        const NDomain::TPartitionWeight partitionWeight,
        const TPredictionParams& params) const = 0;

    virtual ~ILoadFactorPredictor() = default;
};

using TLoadFactorPredictorPtr = std::shared_ptr<ILoadFactorPredictor>;

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
