
#pragma once

#include <core/common/coordination_params.hpp>
#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class TPartitionState {
public:
    TPartitionState(
        TPartitionId id,
        TEpoch lastMigrationEpoch,
        TPartitionLoad observedLoad,
        THubEndpoint hub);

    TPartitionId GetId() const;
    TEpoch GetMigrationFreezeEpoch() const;
    TPartitionLoad GetObservedLoad() const;
    const THubEndpoint& GetAssignedHub() const;

    void SetId(TPartitionId id);
    void SetMigrationFreezeEpoch(TEpoch epoch);
    void SetObservedLoad(TPartitionLoad load);
    void SetAssignedHub(THubEndpoint hub);

private:
    TPartitionId Id_;
    TEpoch MigrationFreezeEpoch_;
    TPartitionLoad ObservedLoad_;
    THubEndpoint AssignedHub_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
