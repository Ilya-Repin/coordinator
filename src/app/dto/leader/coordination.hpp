#pragma once

#include <core/coordination/coordination_settings.hpp>
#include <core/partition_balancing/balancing_settings.hpp>

namespace NCoordinator::NApp::NDto {

////////////////////////////////////////////////////////////////////////////////

struct TCoordinationRequest {
    NCore::NDomain::TStateBuildingSettings StateBuildingSettings;
    NCore::TBalancingSettings BalancingSettings;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NDto
