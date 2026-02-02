#pragma once

#include <app/dto/admin/get_context.hpp>
#include <app/dto/admin/get_partition_map.hpp>
#include <app/use_cases/admin/get_context/get_context.hpp>
#include <app/use_cases/admin/get_partition_map/get_partition_map.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/coordination/coordination_repository.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NService {

////////////////////////////////////////////////////////////////////////////////

class TAdminService final
{
public:
   TAdminService(
        NCore::NDomain::ICoordinationRepository& coordinationRepository,
        NCore::NDomain::ICoordinationGateway& coordinationGateway);

   NDto::TGetContextResponse GetCoordinationContext() const;
   NDto::TGetPartitionMapResponse GetPartitionMap() const;

private:
    NUseCase::TGetContextUseCase GetContextUseCase_;
    NUseCase::TGetPartitionMapUseCase GetPartitionMapUseCase_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NService
