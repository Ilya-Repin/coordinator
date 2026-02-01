#pragma once

#include <app/dto/admin/get_context.hpp>
#include <app/use_cases/admin/get_context/get_context.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/coordination/coordination_repository.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

class TAdminService final
{
public:
   TAdminService(NCore::NDomain::ICoordinationRepository& coordinationRepository);

   NDto::TGetContextResponse GetCoordinationContext() const;

private:
    NUseCase::TGetContextUseCase GetContextUseCase_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NService
