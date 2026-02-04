#include "get_partition.hpp"

#include <app/dto/admin/get_partition.hpp>

#include <infra/components/admin/admin_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/logging/log.hpp>

namespace NCoordinator::NApi::NHandlers {

////////////////////////////////////////////////////////////////////////////////

TGetPartitionHandler::TGetPartitionHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
    , AdminService_(context.FindComponent<NInfra::NComponents::TAdminServiceComponent>().GetService())
{ }

userver::formats::json::Value TGetPartitionHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& /*request_context*/) const
{
    NApp::NDto::TGetPartitionResponse result;

    const auto& channel = request.GetPathArg("channel");

    try {
        result = AdminService_.GetPartition(NApp::NDto::TGetPartitionRequest{channel});
    } catch (const NApp::NUseCase::TGetPartitionTemporaryUnavailable& ex) {
        LOG_ERROR() << "Get partition unavailable: " << ex.what();
        throw TServerException("Get partition temporary unavailable");
    }

    userver::formats::json::ValueBuilder builder;
    builder["partition"] = result.Partition.GetUnderlying();

    return builder.ExtractValue();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NApi::NHandlers
