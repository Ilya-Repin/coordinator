#include "serializer.hpp"

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>

#include <userver/formats/json/value_builder.hpp>

#include <unordered_set>

namespace {

////////////////////////////////////////////////////////////////////////////////

constexpr inline std::string_view EPOCH_KEY = "epoch";
constexpr inline std::string_view DC_KEY = "dc";
constexpr inline std::string_view LOAD_FACTOR_KEY = "load_factor";
constexpr inline std::string_view PARTITIONS_KEY = "partitions";
constexpr inline std::string_view PARTITION_ID_KEY = "id";
constexpr inline std::string_view PARTITION_WEIGHT_KEY = "partition_weight";
constexpr inline std::string_view PARTITIONS_CONTEXT_KEY = "partitions_context";
constexpr inline std::string_view HUB_KEY = "hub";
constexpr inline std::string_view ENDPOINT_KEY = "endpoint";
constexpr inline std::string_view COOLDOWN_EPOCH_KEY = "cooldown_epoch";

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

userver::formats::json::Value SerializePartitionMap(
    const NCore::NDomain::TPartitionMap& partitionMap)
{
    userver::formats::json::ValueBuilder result;
    result[PARTITIONS_KEY.data()] = userver::formats::common::Type::kArray;
    result[EPOCH_KEY.data()] = partitionMap.Epoch.GetUnderlying();
    
    for (const auto& [partition, hub] : partitionMap.Partitions) {
        userver::formats::json::ValueBuilder pair;
        pair[PARTITION_ID_KEY.data()] = partition.GetUnderlying();
        pair[HUB_KEY.data()] = hub.GetUnderlying();

        result[PARTITIONS_KEY.data()].PushBack(pair.ExtractValue());
    }

    return result.ExtractValue();
}

userver::formats::json::Value SerializeCoordinationContext(const NCore::NDomain::TCoordinationContext& context)
{
    userver::formats::json::ValueBuilder result;
    result[PARTITIONS_CONTEXT_KEY.data()] = userver::formats::common::Type::kArray;

    std::unordered_set<NCore::NDomain::TPartitionId> allIds;
    for (const auto& [id, _] : context.PartitionCooldowns) {
        allIds.insert(id);
    }
    for (const auto& [id, _] : context.PartitionWeights) {
        allIds.insert(id);
    }

    for (const auto& partitionId : allIds) {
        userver::formats::json::ValueBuilder item;
        item[PARTITION_ID_KEY.data()] = partitionId.GetUnderlying();

        if (auto it = context.PartitionCooldowns.find(partitionId); it != context.PartitionCooldowns.end()) {
            item[COOLDOWN_EPOCH_KEY.data()] = it->second.GetUnderlying();
        }

        if (auto it = context.PartitionWeights.find(partitionId); it != context.PartitionWeights.end()) {
            item[PARTITION_WEIGHT_KEY.data()] = it->second.GetUnderlying();
        }

        result[PARTITIONS_CONTEXT_KEY.data()].PushBack(item.ExtractValue());
    }

    return result.ExtractValue();
}

userver::formats::json::Value SerializeHubReport(const NCore::NDomain::THubReport& report)
{
    userver::formats::json::ValueBuilder builder;

    builder[EPOCH_KEY.data()] = std::to_string(report.Epoch.GetUnderlying());

    builder[ENDPOINT_KEY.data()] = report.Endpoint.GetUnderlying();
    builder[DC_KEY.data()] = report.DC.GetUnderlying();

    builder[LOAD_FACTOR_KEY.data()] = report.LoadFactor.GetUnderlying();

    userver::formats::json::ValueBuilder partitionsBuilder(userver::formats::common::Type::kObject);

    for (const auto& [partitionId, partitionWeight] : report.PartitionWeights) {
        partitionsBuilder[std::to_string(partitionId.GetUnderlying())] = 
            std::to_string(partitionWeight.GetUnderlying());
    }

    builder[PARTITIONS_KEY.data()] = partitionsBuilder;

    return builder.ExtractValue();
}

NCore::NDomain::TPartitionMap DeserializePartitionMap(const userver::formats::json::Value& jsonValue)
{
    NCore::NDomain::TPartitionMap result;
    result.Epoch = NCore::NDomain::TEpoch{
        jsonValue[EPOCH_KEY.data()].As<NCore::NDomain::TEpoch::UnderlyingType>()
    };
    
    for (const auto& pair : jsonValue[PARTITIONS_KEY.data()]) {
        NCore::NDomain::TPartitionId partitionId{
            pair[PARTITION_ID_KEY.data()].As<NCore::NDomain::TPartitionId::UnderlyingType>()
        };
        NCore::NDomain::THubEndpoint hubEndpoint{
            pair[HUB_KEY.data()].As<NCore::NDomain::THubEndpoint::UnderlyingType>()
        };

        result.Partitions.emplace_back(std::move(partitionId), std::move(hubEndpoint));
    }

    return result;
}

NCore::NDomain::TCoordinationContext DeserializeCoordinationContext(const userver::formats::json::Value& json)
{
    NCore::NDomain::TCoordinationContext context;

    for (const auto& item : json[PARTITIONS_CONTEXT_KEY.data()]) {
        auto partitionId = NCore::NDomain::TPartitionId{item[PARTITION_ID_KEY.data()].As<uint64_t>()};

        if (item.HasMember(COOLDOWN_EPOCH_KEY.data())) {
            context.PartitionCooldowns[partitionId] = 
                NCore::NDomain::TEpoch{item[COOLDOWN_EPOCH_KEY.data()].As<uint64_t>()};
        }

        if (item.HasMember(PARTITION_WEIGHT_KEY.data())) {
            context.PartitionWeights[partitionId] = 
                NCore::NDomain::TPartitionWeight{item[PARTITION_WEIGHT_KEY.data()].As<uint64_t>()};
        }
    }

    return context;
}

NCore::NDomain::THubReport DeserializeHubReport(const userver::formats::json::Value& jsonValue)
{
    NCore::NDomain::THubReport report;

    report.Epoch = NCore::NDomain::TEpoch{
        std::stoull(jsonValue[EPOCH_KEY.data()].As<std::string>())
    };
    report.Endpoint = NCore::NDomain::THubEndpoint{
        jsonValue[ENDPOINT_KEY.data()].As<std::string>()
    };
    report.DC = NCore::NDomain::THubDC{
        jsonValue[DC_KEY.data()].As<std::string>()
    };
    report.LoadFactor = NCore::NDomain::TLoadFactor{
        jsonValue[LOAD_FACTOR_KEY.data()].As<std::uint64_t>()
    };

    const auto& partitionsJson = jsonValue[PARTITIONS_KEY];
    for (auto it = partitionsJson.begin(); it != partitionsJson.end(); ++it) {
        NCore::NDomain::TPartitionId partitionId{
            std::stoull(it.GetName())
        };
        NCore::NDomain::TPartitionWeight partitionWeight{
            std::stoull(it->As<std::string>())
        };

        report.PartitionWeights.emplace(partitionId, partitionWeight);
    }

    return report;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
