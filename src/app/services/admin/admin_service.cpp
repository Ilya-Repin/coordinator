#include "admin_service.hpp"

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

TAdminService::TAdminService(
    NCore::NDomain::ICoordinationRepository& coordinationRepository,
    NCore::NDomain::ICoordinationGateway& coordinationGateway,
    NCore::NDomain::IHubGateway& hubGateway)
    : GetContextUseCase_(coordinationRepository)
    , GetPartitionMapUseCase_(coordinationGateway)
    , GetHubReportsUseCase_(coordinationGateway, hubGateway)
{ }

NDto::TGetContextResponse TAdminService::GetCoordinationContext() const
{
    return GetContextUseCase_.Execute();
}

NDto::TGetPartitionMapResponse TAdminService::GetPartitionMap() const
{
    return GetPartitionMapUseCase_.Execute();
}

NDto::TGetHubReportsResponse TAdminService::GetHubReports() const
{
    return GetHubReportsUseCase_.Execute();
}


////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NApp::NService
