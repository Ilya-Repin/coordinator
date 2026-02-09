#include "hub_gateway_config.hpp"

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

THubGatewaySettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<THubGatewaySettings>)
{
    return THubGatewaySettings{
        .Timeout = value["timeout_seconds"].As<std::chrono::seconds>(),
        .Retries = value["retries"].As<std::size_t>(),
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
