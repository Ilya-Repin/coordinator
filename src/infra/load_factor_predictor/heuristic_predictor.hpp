#pragma once

#include <core/common/hub_params.hpp>
#include <core/partition_balancing/load_factor_predictor.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

class THeuristicPredictor
    : public NCore::ILoadFactorPredictor 
{
public:
    NCore::NDomain::TLoadFactor PredictLoadFactor(const NCore::TPredictionParams& params) const override;

private:
    NCore::NDomain::TLoadFactor DefaultFirstLoadFactor{5}; // TODO replace with dynconfig
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
