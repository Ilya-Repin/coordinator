#include "serializer.hpp"

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>

#include <userver/formats/json/value_builder.hpp>

namespace {

////////////////////////////////////////////////////////////////////////////////

constexpr inline std::string_view EPOCH_KEY = "epoch";
constexpr inline std::string_view PARTITIONS_KEY = "partitions";
constexpr inline std::string_view PARTITION_ID_KEY = "id";
constexpr inline std::string_view HUB_ENDPOINT_KEY = "hub";

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
        pair[HUB_ENDPOINT_KEY.data()] = hub.GetUnderlying();

        result[PARTITIONS_KEY.data()].PushBack(pair.ExtractValue());
    }

    return result.ExtractValue();
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
            pair[HUB_ENDPOINT_KEY.data()].As<NCore::NDomain::THubEndpoint::UnderlyingType>()
        };

        result.Partitions.emplace_back(std::move(partitionId), std::move(hubEndpoint));
    }

    return result;
}

NCore::NDomain::THubReport DeserializeHubReport(const userver::formats::json::Value& jsonValue)
{
    NCore::NDomain::THubReport report;

    report.Epoch = NCore::NDomain::TEpoch{
        std::stoull(jsonValue["epoch"].As<std::string>())
    };
    report.Endpoint = NCore::NDomain::THubEndpoint{
        jsonValue["endpoint"].As<std::string>()
    };
    report.DC = NCore::NDomain::THubDC{
        jsonValue["dc"].As<std::string>()
    };
    report.LoadFactor = NCore::NDomain::TLoadFactor{
        jsonValue["load_factor"].As<std::uint64_t>()
    };

    const auto& partitionsJson = jsonValue["partitions"];
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
