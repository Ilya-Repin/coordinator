#pragma once

#include <core/partition_balancing/load_factor_predictor.hpp>

#include <gmock/gmock.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;

class TMockLoadFactorPredictor
    : public ILoadFactorPredictor
{
public:
    MOCK_METHOD(NDomain::TLoadFactor, PredictLoadFactor,
        (const TPredictionParams& params),
        (const, override));
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
