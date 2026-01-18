#include "partition_state.hpp"

#include <utility>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TPartitionState::TPartitionState(
    TPartitionId id,
    TEpoch migrationFreezeEpoch,
    TPartitionLoad observedLoad,
    THubEndpoint assignedHub)
    : Id_(id)
    , MigrationFreezeEpoch_(migrationFreezeEpoch)
    , ObservedLoad_(observedLoad)
    , AssignedHub_(std::move(assignedHub))
{ }

TPartitionId TPartitionState::GetId() const
{
    return Id_;
}

TEpoch TPartitionState::GetMigrationFreezeEpoch() const
{
    return MigrationFreezeEpoch_;
}

TPartitionLoad TPartitionState::GetObservedLoad() const
{
    return ObservedLoad_;
}

const THubEndpoint& TPartitionState::GetAssignedHub() const
{
    return AssignedHub_;
}

void TPartitionState::SetId(TPartitionId id)
{
    Id_ = id;
}

void TPartitionState::SetMigrationFreezeEpoch(TEpoch epoch)
{
    MigrationFreezeEpoch_ = epoch;
}

void TPartitionState::SetObservedLoad(TPartitionLoad load)
{
    ObservedLoad_ = load;
}

void TPartitionState::SetAssignedHub(THubEndpoint hub)
{
    AssignedHub_ = std::move(hub);
}

} // namespace NCoordinator::NCore::NDomain
