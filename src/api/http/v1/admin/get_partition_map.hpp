#pragma once

#include <app/services/admin/admin_service.hpp>

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/ydb/component.hpp>

namespace NCoordinator::NApi::NHandlers {

////////////////////////////////////////////////////////////////////////////////

class TGetPartitionMapHandler final
    : public userver::server::handlers::HttpHandlerJsonBase
{
public:
    static constexpr std::string_view kName = "handler-get-partition-map";

    TGetPartitionMapHandler(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&);

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& request_json,
        userver::server::request::RequestContext& context) const override;

private:
    NApp::NService::TAdminService& AdminService_;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NApi::NHandlers
