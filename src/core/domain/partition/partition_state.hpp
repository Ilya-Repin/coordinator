
#pragma once

#include <core/domain/common/coordination_params.hpp>
#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>

#include <optional>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class TPartitionState {
public:
    TPartitionState(
        TPartitionId id,
        TEpoch lastMigrationEpoch,
        TPartitionLoad observedLoad,
        std::optional<THubEndpoint> hub);

    TPartitionId GetId() const;
    TEpoch GetLastMigrationEpoch() const;
    TPartitionLoad GetObservedLoad() const;
    const std::optional<THubEndpoint>& GetHub() const;

    void SetId(TPartitionId id);
    void SetLastMigrationEpoch(TEpoch epoch);
    void SetObservedLoad(TPartitionLoad load);
    void SetHub(std::optional<THubEndpoint> hub);

private:
    TPartitionId Id_;
    TEpoch LastMigrationEpoch_;
    TPartitionLoad ObservedLoad_;
    std::optional<THubEndpoint> Hub_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
