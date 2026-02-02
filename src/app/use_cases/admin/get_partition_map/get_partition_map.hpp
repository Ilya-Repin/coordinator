#pragma once

#include <app/exceptions.hpp>
#include <app/dto/admin/get_partition_map.hpp>
#include <core/coordination/coordination_gateway.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

class TGetPartitionMapTemporaryUnavailable
    : public TApplicationException
{
  using TApplicationException::TApplicationException;
};

class TGetPartitionMapUseCase final
{
public:
   TGetPartitionMapUseCase(NCore::NDomain::ICoordinationGateway& coordinationGateway);

   NDto::TGetPartitionMapResponse Execute() const;

private:
    NCore::NDomain::ICoordinationGateway& CoordinationGateway_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
