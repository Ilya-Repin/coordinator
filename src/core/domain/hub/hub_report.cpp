#include "hub_report.hpp"

#include <utility>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

THubReport::THubReport(
    TEpoch epoch,
    THubEndpoint endpoint,
    THubDC dc,
    TLoadFactor loadFactor,
    TPartitionLoads partitionLoads)
    : Epoch_(epoch)
    , Endpoint_(std::move(endpoint))
    , DC_(std::move(dc))
    , LoadFactor_(loadFactor)
    , PartitionLoads_(std::move(partitionLoads))
{ }

TEpoch THubReport::GetEpoch() const
{
    return Epoch_;
}

const THubEndpoint& THubReport::GetEndpoint() const
{
    return Endpoint_;
}

const THubDC& THubReport::GetDC() const
{
    return DC_;
}

TLoadFactor THubReport::GetLoadFactor() const
{
    return LoadFactor_;
}

const THubReport::TPartitionLoads& THubReport::GetPartitionLoads() const
{
    return PartitionLoads_;
}

void THubReport::SetEpoch(TEpoch epoch)
{
    Epoch_ = epoch;
}

void THubReport::SetEndpoint(THubEndpoint endpoint)
{
    Endpoint_ = std::move(endpoint);
}

void THubReport::SetDC(THubDC dc)
{
    DC_ = std::move(dc);
}

void THubReport::SetLoadFactor(TLoadFactor loadFactor)
{
    LoadFactor_ = loadFactor;
}

void THubReport::SetPartitionLoads(TPartitionLoads loads)
{
    PartitionLoads_ = std::move(loads);
}

} // namespace NCoordinator::NCore::NDomain
