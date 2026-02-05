#include "heuristic_predictor.hpp"

#include <core/common/partition_params.hpp>
#include <infra/dynconfig/predictor/predictor_config.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

THeuristicPredictor::THeuristicPredictor(userver::dynamic_config::Source configSource)
    : ConfigSource_(configSource)
{ }

NCore::NDomain::TLoadFactor THeuristicPredictor::PredictLoadFactor(
    const NCore::TPredictionParams& params) const
{
    const double currentTotalWeight = static_cast<double>(params.PartitionsWeight.GetUnderlying());

    if (currentTotalWeight <= std::numeric_limits<double>::epsilon() || params.TotalPartitions == 0) {
        if (params.Increasing) {
            const auto snapshot = ConfigSource_.GetSnapshot();
            auto loadFactor = snapshot[PREDICTOR_CONFIG].DefaultFirstLoadFactor;
            return loadFactor;
        } else {
            return NCore::NDomain::TLoadFactor{0};
        }
    }

    const double currentLoadFactor = static_cast<double>(params.LoadFactor.GetUnderlying());
    const double deltaWeight = static_cast<double>(params.PartitionWeight.GetUnderlying());

    double newTotalWeight = currentTotalWeight;
    if (params.Increasing) {
        newTotalWeight += deltaWeight;
    } else {
        newTotalWeight -= deltaWeight;
    }

    if (newTotalWeight < 0.0) {
        newTotalWeight = 0.0;
    }

    double predictedVal = currentLoadFactor * (newTotalWeight / currentTotalWeight);

    predictedVal = std::ceil(predictedVal);

    const auto result = std::clamp(
        static_cast<std::uint64_t>(predictedVal), 
        std::uint64_t{0}, 
        std::uint64_t{100});

    return NCore::NDomain::TLoadFactor{result};
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
