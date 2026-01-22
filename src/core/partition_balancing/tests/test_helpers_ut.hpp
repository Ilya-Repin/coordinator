#pragma once

#include "load_factor_predictor_mock.hpp"

#include <core/partition_balancing/load_factor_predictor.hpp>
#include <core/partition/partition_map.hpp>
#include <core/hub/hub_report.hpp>
#include <core/coordination/coordination_context.hpp>
#include <core/coordination/coordination_settings.hpp>

#include <memory>

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDomain;

class TBalancingTestBase
    : public Test
{
protected:
    void SetUp() override;

    TEpoch EP(TEpoch::UnderlyingType epoch) const;
    TPartitionId PID(TPartitionId::UnderlyingType partitionId) const;
    TPartitionWeight PW(TPartitionWeight::UnderlyingType partitionWeight) const;

    THubEndpoint HUB(THubEndpoint::UnderlyingType hub) const;
    THubDC DC(THubDC::UnderlyingType dc) const;
    TLoadFactor LF(TLoadFactor::UnderlyingType lf) const;

protected:
    std::shared_ptr<TMockLoadFactorPredictor> Predictor_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
