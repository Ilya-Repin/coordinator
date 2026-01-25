#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>

#include <unordered_set>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

struct TStateBuildingSettings {
    std::unordered_set<NDomain::THubDC> BlockedDCs;
    std::unordered_set<NDomain::THubEndpoint> BlockedHubs;
    TLoadFactor OverloadThreshold;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
