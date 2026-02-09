#pragma once

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/formats/json/value.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

struct THubGatewaySettings {
    std::chrono::seconds Timeout{5};
    std::size_t Retries{3};
};

inline const userver::dynamic_config::Key<THubGatewaySettings> HUB_GATEWAY_CONFIG{
    "HUB_GATEWAY_CONFIG",
    userver::dynamic_config::DefaultAsJsonString{R"(
{
    "timeout_seconds": 5,
    "retries": 3
}
)"}};

THubGatewaySettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<THubGatewaySettings>);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
