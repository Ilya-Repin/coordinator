#include "get_partition_map.hpp"

#include <app/dto/admin/get_partition_map.hpp>

#include <infra/components/admin/admin_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/logging/log.hpp>

namespace NCoordinator::NApi::NHandlers {

////////////////////////////////////////////////////////////////////////////////

TGetPartitionMapHandler::TGetPartitionMapHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
    , AdminService_(context.FindComponent<NInfra::NComponents::TAdminServiceComponent>().GetService())
{ }

userver::formats::json::Value TGetPartitionMapHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& /*request*/,
    const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& /*request_context*/) const
{
    NApp::NDto::TGetPartitionMapResponse result;

    try {
        result = AdminService_.GetPartitionMap();
    } catch (const NApp::NUseCase::TGetPartitionMapTemporaryUnavailable& ex) {
        LOG_ERROR() << "Get partition maps unavailable: " << ex.what();
        throw TServerException("Get partition map temporary unavailable");
    }

    userver::formats::json::ValueBuilder builder;
    builder["partition_map"] = NInfra::SerializePartitionMap(result.PartitionMap);

    return builder.ExtractValue();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NApi::NHandlers
