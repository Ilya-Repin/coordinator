
#pragma once

#include <core/domain/common/coordination_params.hpp>
#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>
#include <core/domain/hub/hub.hpp>

#include <optional>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TPartition {
    TPartitionId Id{};
    THubEndpoint Hub{};
};

struct TPartitionState {
    TEpoch ObservedAt{};
    TEpoch LastMigrationEpoch{};
    TPartitionLoad ObservedLoad{};
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
