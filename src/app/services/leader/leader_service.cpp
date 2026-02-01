#include "leader_service.hpp"

#include <userver/logging/log.hpp>

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

TLeaderService::TLeaderService(
    NCore::NDomain::ICoordinationGateway& coordinationGateway,
    NCore::NDomain::ICoordinationRepository& coordinationRepository,
    NCore::NDomain::IHubGateway& hubGateway,
    NCore::ILoadFactorPredictor& loadFactorPredictor)
    : CoordinationUseCase_(coordinationGateway, coordinationRepository, hubGateway, loadFactorPredictor)
{ }

void TLeaderService::Coordinate(const NDto::TCoordinationRequest& request) const
{
    CoordinationUseCase_.Execute(request);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NService
