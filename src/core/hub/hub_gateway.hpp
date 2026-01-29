#pragma once

#include "hub_report.hpp"

#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class IHubGateway {
public:
    virtual std::vector<THubReport> GetHubReports(const std::vector<THubEndpoint>& hubs) const = 0;

    virtual ~IHubGateway() = default;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
