#pragma once

#include <app/exceptions.hpp>
#include <app/dto/admin/get_partition.hpp>
#include <core/coordination/coordination_gateway.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

class TGetPartitionTemporaryUnavailable
    : public TApplicationException
{
  using TApplicationException::TApplicationException;
};

class TGetPartitionUseCase final
{
public:
   TGetPartitionUseCase(NCore::NDomain::ICoordinationGateway& coordinationGateway);

   NDto::TGetPartitionResponse Execute(const NDto::TGetPartitionRequest& request) const;

private:
    NCore::NDomain::ICoordinationGateway& CoordinationGateway_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
