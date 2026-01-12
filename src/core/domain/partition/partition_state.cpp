#include "partition_state.hpp"

#include <utility>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TPartitionState::TPartitionState(
    TPartitionId id,
    TEpoch lastMigrationEpoch,
    TPartitionLoad observedLoad,
    std::optional<THubEndpoint> hub)
    : Id_(id)
    , LastMigrationEpoch_(lastMigrationEpoch)
    , ObservedLoad_(observedLoad)
    , Hub_(std::move(hub))
{ }

TPartitionId TPartitionState::GetId() const
{
    return Id_;
}

TEpoch TPartitionState::GetLastMigrationEpoch() const
{
    return LastMigrationEpoch_;
}

TPartitionLoad TPartitionState::GetObservedLoad() const
{
    return ObservedLoad_;
}

const std::optional<THubEndpoint>& TPartitionState::GetHub() const
{
    return Hub_;
}

void TPartitionState::SetId(TPartitionId id)
{
    Id_ = id;
}

void TPartitionState::SetLastMigrationEpoch(TEpoch epoch)
{
    LastMigrationEpoch_ = epoch;
}

void TPartitionState::SetObservedLoad(TPartitionLoad load)
{
    ObservedLoad_ = load;
}

void TPartitionState::SetHub(std::optional<THubEndpoint> hub)
{
    Hub_ = std::move(hub);
}

} // namespace NCoordinator::NCore::NDomain
