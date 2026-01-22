#include "test_helpers_ut.hpp"

#include <gmock/gmock.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

void TBalancingTestBase::SetUp() {
    Predictor_ = std::make_shared<TMockLoadFactorPredictor>();
}

TEpoch TBalancingTestBase::EP(TEpoch::UnderlyingType epoch) const
{
    return TEpoch(epoch);
}

TPartitionId TBalancingTestBase::PID(TPartitionId::UnderlyingType partitionId) const
{
    return TPartitionId{partitionId};
}

TPartitionWeight TBalancingTestBase::PW(TPartitionWeight::UnderlyingType partitionWeight) const
{
    return TPartitionWeight(partitionWeight);
}

THubEndpoint TBalancingTestBase::HUB(THubEndpoint::UnderlyingType hub) const
{
    return THubEndpoint{std::move(hub)};
}

THubDC TBalancingTestBase::DC(THubDC::UnderlyingType dc) const
{
    return THubDC{std::move(dc)};
}

TLoadFactor TBalancingTestBase::LF(TLoadFactor::UnderlyingType lf) const
{
    return TLoadFactor{lf};
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
