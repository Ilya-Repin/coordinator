#pragma once

#include <core/coordination_gateway.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/ydb/coordination.hpp>
#include <infra/components/coordination/coordination_dist_lock_component.hpp>

#include <userver/ydb/component.hpp>
#include <memory>
#include <infra/kesus_gateway/kesus_gateway.hpp>


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
    std::shared_ptr<userver::ydb::CoordinationClient> ydb_client_;
    std::unique_ptr<NCoordinator::NCore::ICoordinationGateway> gateway;
    NCoordinator::NInfra::NComponents::TCoordinationDistLockComponent& dist_;
};

}  // namespace coordinator