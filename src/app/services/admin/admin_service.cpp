#include "admin_service.hpp"

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

TAdminService::TAdminService(
    NCore::NDomain::ICoordinationRepository& coordinationRepository,
    NCore::NDomain::ICoordinationGateway& coordinationGateway)
    : GetContextUseCase_(coordinationRepository)
    , GetPartitionMapUseCase_(coordinationGateway)
{ }

NDto::TGetContextResponse TAdminService::GetCoordinationContext() const
{
    return GetContextUseCase_.Execute();
}

NDto::TGetPartitionMapResponse TAdminService::GetPartitionMap() const
{
    return GetPartitionMapUseCase_.Execute();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NApp::NService
