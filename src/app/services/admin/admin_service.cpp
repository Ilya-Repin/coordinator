#include "admin_service.hpp"

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

TAdminService::TAdminService(NCore::NDomain::ICoordinationRepository& coordinationRepository)
    : GetContextUseCase_(coordinationRepository)
{ }

NDto::TGetContextResponse TAdminService::GetCoordinationContext() const
{
  return GetContextUseCase_.Execute();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NApp::NService
