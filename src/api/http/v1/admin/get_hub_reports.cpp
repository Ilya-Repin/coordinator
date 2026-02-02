#include "get_hub_reports.hpp"

#include <app/dto/admin/get_hub_reports.hpp>

#include <infra/components/admin/admin_service_component.hpp>
#include <infra/serializer/serializer.hpp>

#include <api/http/exceptions/handler_exceptions.hpp>

#include <userver/logging/log.hpp>

namespace NCoordinator::NApi::NHandlers {

////////////////////////////////////////////////////////////////////////////////

TGetHubReportsHandler::TGetHubReportsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context)
    , AdminService_(context.FindComponent<NInfra::NComponents::TAdminServiceComponent>().GetService())
{ }

userver::formats::json::Value TGetHubReportsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& /*request*/,
    const userver::formats::json::Value& /*request_json*/,
    userver::server::request::RequestContext& /*request_context*/) const
{
    NApp::NDto::TGetHubReportsResponse result;

    try {
        result = AdminService_.GetHubReports();
    } catch (const NApp::NUseCase::TGetPartitionMapTemporaryUnavailable& ex) {
        LOG_ERROR() << "Get hub reports unavailable: " << ex.what();
        throw TServerException("Get hub reports temporary unavailable");
    }

    userver::formats::json::ValueBuilder builder;
    builder["hub_reports"] = userver::formats::common::Type::kArray;

    for (const auto& hubReport : result.HubReports) {
        auto serializedReport = NInfra::SerializeHubReport(hubReport);
        builder["hub_reports"].PushBack(serializedReport);
    }

    return builder.ExtractValue();
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NChat::NApi::NHandlers
