#pragma once

#include <app/exceptions.hpp>
#include <app/dto/admin/get_context.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/coordination/coordination_repository.hpp>
#include <core/partition_balancing/partition_balancer.hpp>
#include <core/partition_balancing/load_factor_predictor.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

class TGetContextTemporaryUnavailable
    : public TApplicationException
{
  using TApplicationException::TApplicationException;
};

class TGetContextUseCase final
{
public:
   TGetContextUseCase(NCore::NDomain::ICoordinationRepository& coordinationRepository);

   NDto::TGetContextResponse Execute() const;

private:
    NCore::NDomain::ICoordinationRepository& CoordinationRepository_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
