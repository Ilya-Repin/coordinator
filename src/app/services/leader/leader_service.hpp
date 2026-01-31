#pragma once

#include <app/dto/leader/coordination.hpp>
#include <app/use_cases/leader/coordination/coordination.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/coordination/coordination_repository.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

class TLeaderService final
{
public:
   TLeaderService(
        NCore::NDomain::ICoordinationGateway& coordinationGateway_,
        NCore::NDomain::ICoordinationRepository& coordinationRepository_,
        NCore::NDomain::IHubGateway& hubGateway,
        NCore::ILoadFactorPredictor& loadFactorPredictor);

   void Coordinate(const NDto::TCoordinationRequest& request) const;

private:
    NUseCase::TCoordinationUseCase CoordinationUseCase_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NService
