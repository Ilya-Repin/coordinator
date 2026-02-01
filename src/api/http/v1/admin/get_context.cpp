#include "get_context.hpp"

#include <app/dto/admin/get_context.hpp>

#include <infra/components/admin/admin_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/logging/log.hpp>

namespace NCoordinator::NInfra::NHandlers {

////////////////////////////////////////////////////////////////////////////////

TGetContextHandler::TGetContextHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
    , AdminService_(context.FindComponent<NComponents::TAdminServiceComponent>().GetService())
{ }

userver::formats::json::Value TGetContextHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& /*request*/,
    const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& /*request_context*/) const
{
    NApp::NDto::TGetContextResponse result;

    try {
        result = AdminService_.GetCoordinationContext();
    } catch (const NApp::NUseCase::TGetContextTemporaryUnavailable& ex) {
        LOG_ERROR() << "Get context unavailable: " << ex.what();
        throw TServerException("Get context temporary unavailable");
    }

    userver::formats::json::ValueBuilder builder;
    builder["context"] = SerializeCoordinationContext(result.Context);

    return builder.ExtractValue();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NInfra::NHandlers
