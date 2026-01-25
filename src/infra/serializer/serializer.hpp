#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>
#include <core/partition/partition_map.hpp>

#include <userver/formats/json/value.hpp>

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

userver::formats::json::Value SerializePartitionMap(const NCore::NDomain::TPartitionMap& partitionMap);

NCore::NDomain::TPartitionMap DeserializePartitionMap(const userver::formats::json::Value& jsonValue);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
