#include "get_hub_reports.hpp"

#include <core/coordination/coordination_state.hpp>

#include <userver/logging/log.hpp>

#include <fmt/format.h>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

TGetHubReportsUseCase::TGetHubReportsUseCase(
    NCore::NDomain::ICoordinationGateway& coordinationGateway,
    NCore::NDomain::IHubGateway& hubGateway)
    : CoordinationGateway_(coordinationGateway)
    , HubGateway_(hubGateway)
{ }

NDto::TGetHubReportsResponse TGetHubReportsUseCase::Execute() const
{
    std::vector<NCore::NDomain::THubReport> hubReports;
    
    try {
        auto hubDiscovery = CoordinationGateway_.GetHubDiscovery();
        hubReports = HubGateway_.GetHubReports(hubDiscovery);
    } catch (std::exception& ex) {
        throw TGetHubReportsTemporaryUnavailable(fmt::format("Failed to get hub reports: {}", ex.what()));
    }

    NDto::TGetHubReportsResponse response{
        .HubReports = std::move(hubReports),
    };

    return response;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
