#include "hub_gateway.hpp"

#include <infra/serializer/serializer.hpp>
#include <infra/dynconfig/hub_gateway/hub_gateway_config.hpp>

#include <userver/engine/wait_any.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>

#include <chrono>

namespace NCoordinator::NInfra::NGateway {

////////////////////////////////////////////////////////////////////////////////

THubGateway::THubGateway(userver::clients::http::Client& client, userver::dynamic_config::Source configSource)
    : Client_(client)
    , ConfigSource_(configSource)
{ }

std::vector<NCore::NDomain::THubReport> THubGateway::GetHubReports(
    const std::vector<NCore::NDomain::THubEndpoint>& hubs) const
{
    std::vector<userver::clients::http::ResponseFuture> requests;
    requests.reserve(hubs.size());

    const auto snapshot = ConfigSource_.GetSnapshot();
    const auto config = snapshot[HUB_GATEWAY_CONFIG];   

    for (const auto& hub : hubs) {
        auto request = Client_.CreateRequest()
            .follow_redirects(false)
            .get(hub.GetUnderlying())
            .timeout(config.Timeout)
            .retry(config.Retries)
            .async_perform();

        requests.emplace_back(std::move(request));
    }
    
    std::vector<NCore::NDomain::THubReport> reports;
    reports.reserve(requests.size());

    auto deadline = userver::engine::Deadline::FromDuration(DEFAULT_TIMEOUT);

    while (auto index = userver::engine::WaitAnyUntil(deadline, requests)) {
        const std::shared_ptr<userver::clients::http::Response> response = requests[*index].Get();

        if (response->IsOk()) {
            userver::formats::json::Value json;
            try {
                json = userver::formats::json::FromString(response->body());
            } catch(std::exception& ex) {
                LOG_ERROR() << "Can't parse json: " << ex;
                continue;
            }

            try {
                auto report = DeserializeHubReport(json);
                reports.emplace_back(std::move(report));
            } catch (std::exception& ex) {
                LOG_ERROR() << "Can't deserialize hub report " << ex;
                continue;
            }
        }
    }

    return reports;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NGateway
