#pragma once

#include <core/common/partition_params.hpp>

namespace NCoordinator::NApp::NDto {

////////////////////////////////////////////////////////////////////////////////

struct TGetPartitionRequest {
    std::string ChannelId;
};

struct TGetPartitionResponse {
    NCore::NDomain::TPartitionId Partition;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NDto
