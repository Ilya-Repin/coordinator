#pragma once

#include <app/dto/leader/coordination.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/coordination/coordination_repository.hpp>
#include <core/partition_balancing/partition_balancer.hpp>
#include <core/partition_balancing/load_factor_predictor.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

class TCoordinationUseCase final
{
public:
   TCoordinationUseCase(
        NCore::NDomain::ICoordinationGateway& coordinationGateway_,
        NCore::NDomain::ICoordinationRepository& coordinationRepository_,
        NCore::NDomain::IHubGateway& hubGateway,
        NCore::ILoadFactorPredictor& loadFactorPredictor);

   void Execute(const NDto::TCoordinationRequest& request) const;

private:
    NCore::NDomain::ICoordinationGateway& CoordinationGateway_;
    NCore::NDomain::ICoordinationRepository& CoordinationRepository_;
    NCore::NDomain::IHubGateway& HubGateway_;

    NCore::TPartitionBalancer Balancer_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
