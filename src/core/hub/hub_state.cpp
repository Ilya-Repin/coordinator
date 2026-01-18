#include "hub_state.hpp"

#include <utility>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

THubState::THubState(
    THubEndpoint endpoint,
    THubDC dc,
    EHubStatus status,
    TLoadFactor loadFactor,
    TPartitionLoad expectedLoadGrowth,
    TPartitionLoad partitionsLoad,
    std::size_t totalPartitions)
    : Endpoint_(std::move(endpoint))
    , DC_(std::move(dc))
    , Status_(status)
    , LoadFactor_(loadFactor)
    , ExpectedLoadGrowth_(expectedLoadGrowth)
    , PartitionsLoad_(partitionsLoad)
    , TotalPartitions_(totalPartitions)
{ }

const THubEndpoint& THubState::GetEndpoint() const
{
    return Endpoint_;
}

const THubDC& THubState::GetDC() const
{
    return DC_;
}

EHubStatus THubState::GetStatus() const
{
    return Status_;
}

TLoadFactor THubState::GetLoadFactor() const
{
    return LoadFactor_;
}

TPartitionLoad THubState::GetExpectedLoadGrowth() const
{
    return ExpectedLoadGrowth_;
}

TPartitionLoad THubState::GetPartitionsLoad() const
{
    return PartitionsLoad_;
}

std::size_t THubState::GetTotalPartitions() const
{
    return TotalPartitions_;
}

void THubState::SetEndpoint(THubEndpoint endpoint)
{
    Endpoint_ = std::move(endpoint);
}

void THubState::SetDC(THubDC dc)
{
    DC_ = std::move(dc);
}

void THubState::SetStatus(EHubStatus status)
{
    Status_ = status;
}

void THubState::SetLoadFactor(TLoadFactor loadFactor)
{
    LoadFactor_ = loadFactor;
}

void THubState::SetExpectedLoadGrowth(TPartitionLoad growth)
{
    ExpectedLoadGrowth_ = growth;
}

void THubState::SetPartitionsLoad(TPartitionLoad load)
{
    PartitionsLoad_ = load;
}

void THubState::SetTotalPartitions(std::size_t totalPartitions)
{
    TotalPartitions_ = totalPartitions;
}

void THubState::AddToExpectedLoadGrowth(TPartitionLoad growth)
{
    ExpectedLoadGrowth_ = TPartitionLoad{ExpectedLoadGrowth_.GetUnderlying() + growth.GetUnderlying()};
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
