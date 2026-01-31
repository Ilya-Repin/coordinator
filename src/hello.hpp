#pragma once

#include <core/coordination/coordination_gateway.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/ydb/coordination.hpp>
#include <infra/components/leader/leader_dist_lock_component.hpp>
#include <infra/coordination_gateway/kesus_coordination_gateway.hpp>

#include <userver/ydb/component.hpp>
#include <memory>


namespace coordinator {

class Hello final : public userver::server::handlers::HttpHandlerJsonBase {
public:
    static constexpr std::string_view kName = "handler-hello";

    Hello(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context);

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& request_json,
        userver::server::request::RequestContext& context
    ) const override;

private:
    NYdb::NCoordination::TSessionSettings settings;
    NCoordinator::NCore::NDomain::ICoordinationGateway& gateway;
};

}  // namespace coordinator