#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>
#include <core/coordination/coordination_context.hpp>
#include <core/partition/partition_map.hpp>
#include <core/hub/hub_report.hpp>

#include <userver/formats/json/value.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

userver::formats::json::Value SerializePartitionMap(const NCore::NDomain::TPartitionMap& partitionMap);

userver::formats::json::Value SerializeCoordinationContext(const NCore::NDomain::TCoordinationContext& context);

userver::formats::json::Value SerializeHubReport(const NCore::NDomain::THubReport& report);

NCore::NDomain::TPartitionMap DeserializePartitionMap(const userver::formats::json::Value& jsonValue);

NCore::NDomain::TCoordinationContext DeserializeCoordinationContext(const userver::formats::json::Value& json);

NCore::NDomain::THubReport DeserializeHubReport(const userver::formats::json::Value& jsonValue);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
