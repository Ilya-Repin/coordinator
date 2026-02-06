#include "repository_config.hpp"

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/ydb/settings.hpp>

namespace userver::ydb {

////////////////////////////////////////////////////////////////////////////////

OperationSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<OperationSettings>)
{
    OperationSettings result;
    result.retries = value["retries"].As<std::uint32_t>();
    result.operation_timeout_ms = value["operation-timeout"].As<std::chrono::milliseconds>();
    result.cancel_after_ms = value["cancel-after"].As<std::chrono::milliseconds>();
    result.client_timeout_ms = value["client-timeout"].As<std::chrono::milliseconds>();
    result.get_session_timeout_ms = value["get-session-timeout"].As<std::chrono::milliseconds>();
    result.tx_mode = userver::ydb::TransactionMode::kSerializableRW;

    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace userver::ydb

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

TRepositorySettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TRepositorySettings>)
{
    const auto getContextSettings = value["get_context_settings"].As<userver::ydb::OperationSettings>();
    const auto setContextSettings = value["set_context_settings"].As<userver::ydb::OperationSettings>();

    return TRepositorySettings{
        .GetContextSettings = getContextSettings,
        .SetContextSettings = setContextSettings,
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
