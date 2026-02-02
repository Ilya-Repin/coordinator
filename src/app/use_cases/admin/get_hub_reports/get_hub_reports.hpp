#pragma once

#include <app/exceptions.hpp>
#include <app/dto/admin/get_hub_reports.hpp>
#include <core/coordination/coordination_gateway.hpp>
#include <core/hub/hub_gateway.hpp>

namespace NCoordinator::NApp::NUseCase {

////////////////////////////////////////////////////////////////////////////////

class TGetHubReportsTemporaryUnavailable
    : public TApplicationException
{
  using TApplicationException::TApplicationException;
};

class TGetHubReportsUseCase final
{
public:
    TGetHubReportsUseCase(
        NCore::NDomain::ICoordinationGateway& coordinationGateway,
        NCore::NDomain::IHubGateway& hubGateway);

   NDto::TGetHubReportsResponse Execute() const;

private:
    NCore::NDomain::ICoordinationGateway& CoordinationGateway_;
    NCore::NDomain::IHubGateway& HubGateway_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NApp::NUseCase
