#pragma once

#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>
#include <core/domain/common/coordination_params.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

enum class EHubStatus {
    UNKNOWN,
    HEALTHY,
    DRAINING,
    OVERLOADED,
    // SUSPECTED,
    // UNHEALTHY,
    LAGGED,
};

struct THubReport {
    THubEndpoint Endpoint{};
    THubDC DC{};
    TLoadFactor LoadFactor{};
    TEpoch AppliedEpoch{};
    std::unordered_map<TPartitionId, TPartitionLoad> TPartitionLoads{};
};

struct THubState {
    TEpoch ObservedAt{};
    THubEndpoint Endpoint{};
    EHubStatus Status{};
    std::size_t LaggedEpochs{};
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
