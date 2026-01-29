#include "serializer.hpp"

#include <gtest/gtest.h>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace {

using namespace NCoordinator::NCore::NDomain;
using namespace NCoordinator::NInfra;

////////////////////////////////////////////////////////////////////////////////

TPartitionMap MakePartitionMap()
{
    TPartitionMap map;
    map.Epoch = TEpoch{42};

    map.Partitions.emplace_back(
        TPartitionId{1},
        THubEndpoint{"hub-1"}
    );
    map.Partitions.emplace_back(
        TPartitionId{2},
        THubEndpoint{"hub-2"}
    );

    return map;
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

TEST(PartitionMapSerializer, SerializeProducesValidJson)
{
    const auto map = MakePartitionMap();

    const auto json = SerializePartitionMap(map);

    ASSERT_TRUE(json.HasMember("epoch"));
    ASSERT_TRUE(json.HasMember("partitions"));

    EXPECT_EQ(json["epoch"].As<uint64_t>(), 42);

    const auto& partitions = json["partitions"];
    ASSERT_EQ(partitions.GetSize(), 2);

    EXPECT_EQ(partitions[0]["id"].As<uint64_t>(), 1);
    EXPECT_EQ(partitions[0]["hub"].As<std::string>(), "hub-1");

    EXPECT_EQ(partitions[1]["id"].As<uint64_t>(), 2);
    EXPECT_EQ(partitions[1]["hub"].As<std::string>(), "hub-2");
}

TEST(PartitionMapSerializer, DeserializeRestoresOriginalData)
{
    const auto original = MakePartitionMap();

    const auto json = SerializePartitionMap(original);
    const auto restored = DeserializePartitionMap(json);

    EXPECT_EQ(restored.Epoch, original.Epoch);
    ASSERT_EQ(restored.Partitions.size(), original.Partitions.size());

    for (size_t i = 0; i < original.Partitions.size(); ++i) {
        EXPECT_EQ(restored.Partitions[i].first, original.Partitions[i].first);
        EXPECT_EQ(restored.Partitions[i].second, original.Partitions[i].second);
    }
}

TEST(PartitionMapSerializer, HandlesEmptyPartitions)
{
    TPartitionMap map;
    map.Epoch = TEpoch{123};

    const auto json = SerializePartitionMap(map);
    const auto restored = DeserializePartitionMap(json);

    EXPECT_EQ(restored.Epoch, TEpoch{123});
    EXPECT_TRUE(restored.Partitions.empty());
}

TEST(HubReportDeserializer, ParsesValidReport)
{
    userver::formats::json::ValueBuilder json;

    json["epoch"] = std::to_string(100);
    json["endpoint"] = "hub-42";
    json["dc"] = "eu-west";
    json["load_factor"] = 73;

    userver::formats::json::ValueBuilder partitions;
    partitions["1"] = std::to_string(10);
    partitions["2"] = std::to_string(20);

    json["partitions"] = partitions.ExtractValue();

    const auto report = DeserializeHubReport(json.ExtractValue());

    EXPECT_EQ(report.Epoch, TEpoch{100});
    EXPECT_EQ(report.Endpoint, THubEndpoint{"hub-42"});
    EXPECT_EQ(report.DC, THubDC{"eu-west"});
    EXPECT_EQ(report.LoadFactor, TLoadFactor{73});

    ASSERT_EQ(report.PartitionWeights.size(), 2);

    EXPECT_EQ(report.PartitionWeights.at(TPartitionId{1}), TPartitionWeight{10});
    EXPECT_EQ(report.PartitionWeights.at(TPartitionId{2}), TPartitionWeight{20});
}

TEST(HubReportDeserializer, HandlesEmptyPartitions)
{
    userver::formats::json::ValueBuilder json;

    json["epoch"] = "1";
    json["endpoint"] = "hub";
    json["dc"] = "dc";
    json["load_factor"] = 0;
    json["partitions"] = userver::formats::json::ValueBuilder{}.ExtractValue();

    const auto report = DeserializeHubReport(json.ExtractValue());

    EXPECT_TRUE(report.PartitionWeights.empty());
}
