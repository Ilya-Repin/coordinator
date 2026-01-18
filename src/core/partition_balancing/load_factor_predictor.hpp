
#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/coordination/coordination_state.hpp>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

struct TPredictionParams {
    std::size_t TotalPartitions;
    NDomain::TPartitionLoad PartitionsLoad;
    NDomain::TLoadFactor OriginalLoadFactor;
};

class ILoadFactorPredictor {
public:
    virtual NDomain::TLoadFactor PredictLoadFactor(
        const NDomain::TLoadFactor loadFactor,
        const NDomain::TPartitionLoad partitionLoad,
        const TPredictionParams& params) const = 0;

    virtual ~ILoadFactorPredictor() = default;
};

using TLoadFactorPredictorPtr = std::shared_ptr<ILoadFactorPredictor>;

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
