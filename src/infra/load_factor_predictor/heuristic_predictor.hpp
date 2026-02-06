#pragma once

#include <core/common/hub_params.hpp>
#include <core/partition_balancing/load_factor_predictor.hpp>

#include <userver/dynamic_config/source.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

class THeuristicPredictor
    : public NCore::ILoadFactorPredictor 
{
public:
    explicit THeuristicPredictor(userver::dynamic_config::Source configSource);
            
    NCore::NDomain::TLoadFactor PredictLoadFactor(const NCore::TPredictionParams& params) const override;

private:
    userver::dynamic_config::Source ConfigSource_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
