#pragma once

#include <core/coordination/coordination_settings.hpp>
#include <core/partition_balancing/balancing_settings.hpp>

#include <userver/dynamic_config/snapshot.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/ydb/settings.hpp>

#include <chrono>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

struct TRepositorySettings {
    userver::ydb::OperationSettings GetContextSettings;
    userver::ydb::OperationSettings SetContextSettings;
};

inline const userver::dynamic_config::Key<TRepositorySettings> REPOSITORY_CONFIG{
    "REPOSITORY_CONFIG",
    userver::dynamic_config::DefaultAsJsonString{R"(
{
  "get_context_settings": {
    "operation_timeout_ms": 3,
    "cancel_after_ms": 1000,
    "client_timeout_ms": 1100,
    "get_session_timeout_ms": 1000 
  },
  "set_context_settings": {
    "operation_timeout_ms": 3,
    "cancel_after_ms": 1000,
    "client_timeout_ms": 1100,
    "get_session_timeout_ms": 1000 
  }
}
)"}};

userver::ydb::OperationSettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<userver::ydb::OperationSettings>);

TRepositorySettings Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<TRepositorySettings>);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
