#include "coordination.hpp"

#include <core/coordination/coordination_state.hpp>

#include <userver/logging/log.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

TCoordinationUseCase::TCoordinationUseCase(
    NCore::NDomain::ICoordinationGateway& coordinationGateway_,
    NCore::NDomain::ICoordinationRepository& coordinationRepository_,
    NCore::NDomain::IHubGateway& hubGateway,
    NCore::ILoadFactorPredictor& loadFactorPredictor)
    : CoordinationGateway_(coordinationGateway_)
    , CoordinationRepository_(coordinationRepository_)
    , HubGateway_(hubGateway)
    , Balancer_(loadFactorPredictor)
{ }

void TCoordinationUseCase::Execute(const NDto::TCoordinationRequest& request) const
{
    auto hubDiscovery = CoordinationGateway_.GetHubDiscovery();
    auto hubReports = HubGateway_.GetHubReports(hubDiscovery);

    auto partitionMapOpt = CoordinationGateway_.GetPartitionMap();
    if (!partitionMapOpt.has_value()) {
        LOG_ERROR() << "Failed to get partition map";
        return;
    }
    auto partitionMap = partitionMapOpt.value();

    auto coordinationContext = CoordinationRepository_.GetCoordinationContext();

    auto coordinationState = NCore::NDomain::TCoordinationState(
        partitionMap,
        hubReports,
        coordinationContext,
        request.StateBuildingSettings);

    auto balancingResult = Balancer_.BalancePartitions(coordinationState, request.BalancingSettings);

    CoordinationGateway_.BroadcastPartitionMap(balancingResult.PartitionMap);
    CoordinationRepository_.SetCoordinationContext(balancingResult.Context);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
