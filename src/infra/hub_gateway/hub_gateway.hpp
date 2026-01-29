#pragma once

#include <core/hub/hub_gateway.hpp>
#include <core/hub/hub_report.hpp>

#include <userver/clients/http/client.hpp>

#include <vector>

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

class THubGateway
    : public NCore::NDomain::IHubGateway 
{
public:
    THubGateway(userver::clients::http::Client& client);

    std::vector<NCore::NDomain::THubReport> GetHubReports(
        const std::vector<NCore::NDomain::THubEndpoint>& hubs) const override;

private:
    userver::clients::http::Client& Client_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
