#pragma once

#include <core/hub/hub_report.hpp>
#include <core/common/hub_params.hpp>

namespace NCoordinator::NApp::NDto {

////////////////////////////////////////////////////////////////////////////////

struct TGetHubReportsResponse {
    std::vector<NCore::NDomain::THubReport> HubReports;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NDto
