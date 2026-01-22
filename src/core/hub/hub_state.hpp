#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct THubState {
    THubEndpoint Endpoint;
    THubDC DC;
    EHubStatus Status;
    TLoadFactor LoadFactor;
    TPartitionWeight ExpectedWeightGrowth;
    TPartitionWeight PartitionsWeight;
    std::size_t TotalPartitions;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
