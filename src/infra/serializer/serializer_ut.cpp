#include "serializer.hpp"

#include <gtest/gtest.h>

#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>

#include <cstdint>
#include <unordered_set>

namespace {

////////////////////////////////////////////////////////////////////////////////

using namespace NCoordinator::NCore::NDomain;
using namespace NCoordinator::NInfra;

TPartitionMap MakePartitionMap()
{
    TPartitionMap map;
    map.Epoch = TEpoch{42};

    map.Partitions.emplace_back(TPartitionId{1}, THubEndpoint{"hub-1"});
    map.Partitions.emplace_back(TPartitionId{2}, THubEndpoint{"hub-2"});
    map.Partitions.emplace_back(TPartitionId{3}, THubEndpoint{"hub-3"});

    return map;
}

TPartitionMap MakeEmptyPartitionMap()
{
    TPartitionMap map;
    map.Epoch = TEpoch{100};
    return map;
}

TCoordinationContext MakeCoordinationContext() 
{
    TCoordinationContext context;

    context.PartitionCooldowns[TPartitionId{10}] = TEpoch{100};
    context.PartitionWeights[TPartitionId{10}] = TPartitionWeight{50};

    context.PartitionCooldowns[TPartitionId{20}] = TEpoch{200};

    context.PartitionWeights[TPartitionId{30}] = TPartitionWeight{300};

    return context;
}

TCoordinationContext MakeEmptyCoordinationContext()
{
    return TCoordinationContext{};
}

THubReport MakeHubReport()
{
    THubReport report;
    report.Epoch = TEpoch{123};
    report.Endpoint = THubEndpoint{"hub.example.com"};
    report.DC = THubDC{"myt"};
    report.LoadFactor = TLoadFactor{85};
    
    report.PartitionWeights.emplace(TPartitionId{10}, TPartitionWeight{500});
    report.PartitionWeights.emplace(TPartitionId{20}, TPartitionWeight{600});
    report.PartitionWeights.emplace(TPartitionId{30}, TPartitionWeight{700});

    return report;
}

THubReport MakeEmptyHubReport()
{
    THubReport report;
    report.Epoch = TEpoch{1};
    report.Endpoint = THubEndpoint{"empty-hub"};
    report.DC = THubDC{"vla"};
    report.LoadFactor = TLoadFactor{0};
    return report;
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

TEST(PartitionMapSerializerTest, SerializeProducesValidJson) {
    const auto map = MakePartitionMap();
    const auto json = SerializePartitionMap(map);

    ASSERT_TRUE(json.HasMember("epoch"));
    ASSERT_TRUE(json.HasMember("partitions"));

    EXPECT_EQ(json["epoch"].As<std::uint64_t>(), 42);

    const auto& partitions = json["partitions"];
    ASSERT_TRUE(partitions.IsArray());
    ASSERT_EQ(partitions.GetSize(), 3);

    EXPECT_EQ(partitions[0]["id"].As<std::uint64_t>(), 1);
    EXPECT_EQ(partitions[0]["hub"].As<std::string>(), "hub-1");

    EXPECT_EQ(partitions[1]["id"].As<std::uint64_t>(), 2);
    EXPECT_EQ(partitions[1]["hub"].As<std::string>(), "hub-2");

    EXPECT_EQ(partitions[2]["id"].As<std::uint64_t>(), 3);
    EXPECT_EQ(partitions[2]["hub"].As<std::string>(), "hub-3");
}

TEST(PartitionMapSerializerTest, RoundTripPreservesData) {
    const auto original = MakePartitionMap();

    const auto json = SerializePartitionMap(original);
    const auto restored = DeserializePartitionMap(json);

    EXPECT_EQ(restored.Epoch, original.Epoch);
    ASSERT_EQ(restored.Partitions.size(), original.Partitions.size());

    for (size_t i = 0; i < original.Partitions.size(); ++i) {
        EXPECT_EQ(restored.Partitions[i].first, original.Partitions[i].first)
            << "Partition ID mismatch at index " << i;
        EXPECT_EQ(restored.Partitions[i].second, original.Partitions[i].second)
            << "Hub endpoint mismatch at index " << i;
    }
}

TEST(PartitionMapSerializerTest, HandlesEmptyPartitions) {
    const auto map = MakeEmptyPartitionMap();

    const auto json = SerializePartitionMap(map);
    const auto restored = DeserializePartitionMap(json);

    EXPECT_EQ(restored.Epoch, TEpoch{100});
    EXPECT_TRUE(restored.Partitions.empty());
    
    const auto& partitions = json["partitions"];
    EXPECT_TRUE(partitions.IsArray());
    EXPECT_EQ(partitions.GetSize(), 0);
}

TEST(PartitionMapSerializerTest, DeserializeParsesValidJson) {
    userver::formats::json::ValueBuilder builder;
    builder["epoch"] = 999;
    
    userver::formats::json::ValueBuilder partitions(userver::formats::common::Type::kArray);
    
    userver::formats::json::ValueBuilder p1;
    p1["id"] = 5;
    p1["hub"] = "test-hub-5";
    partitions.PushBack(p1.ExtractValue());
    
    userver::formats::json::ValueBuilder p2;
    p2["id"] = 7;
    p2["hub"] = "test-hub-7";
    partitions.PushBack(p2.ExtractValue());
    
    builder["partitions"] = partitions.ExtractValue();

    const auto restored = DeserializePartitionMap(builder.ExtractValue());

    EXPECT_EQ(restored.Epoch, TEpoch{999});
    ASSERT_EQ(restored.Partitions.size(), 2);
    EXPECT_EQ(restored.Partitions[0].first, TPartitionId{5});
    EXPECT_EQ(restored.Partitions[0].second, THubEndpoint{"test-hub-5"});
    EXPECT_EQ(restored.Partitions[1].first, TPartitionId{7});
    EXPECT_EQ(restored.Partitions[1].second, THubEndpoint{"test-hub-7"});
}

////////////////////////////////////////////////////////////////////////////////

TEST(CoordinationContextSerializerTest, SerializeProducesValidJson) {
    const auto context = MakeCoordinationContext();
    const auto json = SerializeCoordinationContext(context);

    ASSERT_TRUE(json.HasMember("partitions_context"));
    ASSERT_TRUE(json["partitions_context"].IsArray());
    EXPECT_EQ(json["partitions_context"].GetSize(), 3);

    std::unordered_set<std::uint64_t> foundIds;
    for (const auto& item : json["partitions_context"]) {
        ASSERT_TRUE(item.HasMember("id"));
        foundIds.insert(item["id"].As<std::uint64_t>());
    }

    EXPECT_TRUE(foundIds.count(10));
    EXPECT_TRUE(foundIds.count(20));
    EXPECT_TRUE(foundIds.count(30));
}

TEST(CoordinationContextSerializerTest, RoundTripPreservesData) {
    const auto original = MakeCoordinationContext();

    const auto json = SerializeCoordinationContext(original);
    const auto restored = DeserializeCoordinationContext(json);

    ASSERT_EQ(restored.PartitionCooldowns.size(), original.PartitionCooldowns.size());
    for (const auto& [id, epoch] : original.PartitionCooldowns) {
        ASSERT_TRUE(restored.PartitionCooldowns.count(id)) 
            << "Missing partition " << id.GetUnderlying() << " in cooldowns";
        EXPECT_EQ(restored.PartitionCooldowns.at(id), epoch);
    }

    ASSERT_EQ(restored.PartitionWeights.size(), original.PartitionWeights.size());
    for (const auto& [id, weight] : original.PartitionWeights) {
        ASSERT_TRUE(restored.PartitionWeights.count(id)) 
            << "Missing partition " << id.GetUnderlying() << " in weights";
        EXPECT_EQ(restored.PartitionWeights.at(id), weight);
    }
}

TEST(CoordinationContextSerializerTest, HandlesEmptyContext) {
    const auto context = MakeEmptyCoordinationContext();

    const auto json = SerializeCoordinationContext(context);
    const auto restored = DeserializeCoordinationContext(json);

    EXPECT_TRUE(restored.PartitionCooldowns.empty());
    EXPECT_TRUE(restored.PartitionWeights.empty());
    
    ASSERT_TRUE(json["partitions_context"].IsArray());
    EXPECT_EQ(json["partitions_context"].GetSize(), 0);
}

TEST(CoordinationContextSerializerTest, DeserializeParsesValidJson) {
    userver::formats::json::ValueBuilder builder;
    userver::formats::json::ValueBuilder items(userver::formats::common::Type::kArray);

    userver::formats::json::ValueBuilder item1;
    item1["id"] = 100;
    item1["cooldown_epoch"] = 500;
    item1["partition_weight"] = 250;
    items.PushBack(item1.ExtractValue());

    userver::formats::json::ValueBuilder item2;
    item2["id"] = 200;
    item2["cooldown_epoch"] = 600;
    items.PushBack(item2.ExtractValue());

    builder["partitions_context"] = items.ExtractValue();

    const auto context = DeserializeCoordinationContext(builder.ExtractValue());

    EXPECT_EQ(context.PartitionCooldowns.size(), 2);
    EXPECT_EQ(context.PartitionWeights.size(), 1);

    EXPECT_EQ(context.PartitionCooldowns.at(TPartitionId{100}), TEpoch{500});
    EXPECT_EQ(context.PartitionWeights.at(TPartitionId{100}), TPartitionWeight{250});
    EXPECT_EQ(context.PartitionCooldowns.at(TPartitionId{200}), TEpoch{600});
}

TEST(CoordinationContextSerializerTest, JsonStructureValidation) {
    const auto context = MakeCoordinationContext();
    const auto json = SerializeCoordinationContext(context);

    for (const auto& item : json["partitions_context"]) {
        std::uint64_t id = item["id"].As<std::uint64_t>();
        
        if (id == 10) {
            EXPECT_TRUE(item.HasMember("cooldown_epoch"));
            EXPECT_TRUE(item.HasMember("partition_weight"));
        } else if (id == 20) {
            EXPECT_TRUE(item.HasMember("cooldown_epoch"));
            EXPECT_FALSE(item.HasMember("partition_weight"));
        } else if (id == 30) {
            EXPECT_FALSE(item.HasMember("cooldown_epoch"));
            EXPECT_TRUE(item.HasMember("partition_weight"));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

TEST(HubReportSerializerTest, SerializeProducesValidJson) {
    const auto report = MakeHubReport();
    const auto json = SerializeHubReport(report);

    ASSERT_TRUE(json.HasMember("epoch"));
    ASSERT_TRUE(json.HasMember("endpoint"));
    ASSERT_TRUE(json.HasMember("dc"));
    ASSERT_TRUE(json.HasMember("load_factor"));
    ASSERT_TRUE(json.HasMember("partitions"));

    EXPECT_EQ(json["epoch"].As<std::string>(), "123");
    EXPECT_EQ(json["endpoint"].As<std::string>(), "hub.example.com");
    EXPECT_EQ(json["dc"].As<std::string>(), "myt");
    EXPECT_EQ(json["load_factor"].As<std::uint64_t>(), 85);

    const auto& partitions = json["partitions"];
    ASSERT_TRUE(partitions.IsObject());
    EXPECT_EQ(partitions.GetSize(), 3);

    EXPECT_EQ(partitions["10"].As<std::string>(), "500");
    EXPECT_EQ(partitions["20"].As<std::string>(), "600");
    EXPECT_EQ(partitions["30"].As<std::string>(), "700");
}

TEST(HubReportSerializerTest, RoundTripPreservesData) {
    const auto original = MakeHubReport();

    const auto json = SerializeHubReport(original);
    const auto restored = DeserializeHubReport(json);

    EXPECT_EQ(restored.Epoch, original.Epoch);
    EXPECT_EQ(restored.Endpoint, original.Endpoint);
    EXPECT_EQ(restored.DC, original.DC);
    EXPECT_EQ(restored.LoadFactor, original.LoadFactor);

    ASSERT_EQ(restored.PartitionWeights.size(), original.PartitionWeights.size());
    for (const auto& [id, weight] : original.PartitionWeights) {
        ASSERT_TRUE(restored.PartitionWeights.count(id)) 
            << "Missing partition " << id.GetUnderlying() << " in restored report";
        EXPECT_EQ(restored.PartitionWeights.at(id), weight);
    }
}

TEST(HubReportSerializerTest, HandlesEmptyPartitions) {
    const auto report = MakeEmptyHubReport();

    const auto json = SerializeHubReport(report);
    const auto restored = DeserializeHubReport(json);

    EXPECT_EQ(restored.Epoch, TEpoch{1});
    EXPECT_EQ(restored.Endpoint, THubEndpoint{"empty-hub"});
    EXPECT_EQ(restored.DC, THubDC{"vla"});
    EXPECT_EQ(restored.LoadFactor, TLoadFactor{0});
    EXPECT_TRUE(restored.PartitionWeights.empty());

    ASSERT_TRUE(json.HasMember("partitions"));
    EXPECT_TRUE(json["partitions"].IsObject());
    EXPECT_EQ(json["partitions"].GetSize(), 0);
}

TEST(HubReportSerializerTest, DeserializeParsesValidJson) {
    userver::formats::json::ValueBuilder builder;
    builder["epoch"] = "999";
    builder["endpoint"] = "test-hub.com";
    builder["dc"] = "eu-west";
    builder["load_factor"] = 42;

    userver::formats::json::ValueBuilder partitions;
    partitions["1"] = "100";
    partitions["2"] = "200";
    partitions["3"] = "300";
    builder["partitions"] = partitions.ExtractValue();

    const auto report = DeserializeHubReport(builder.ExtractValue());

    EXPECT_EQ(report.Epoch, TEpoch{999});
    EXPECT_EQ(report.Endpoint, THubEndpoint{"test-hub.com"});
    EXPECT_EQ(report.DC, THubDC{"eu-west"});
    EXPECT_EQ(report.LoadFactor, TLoadFactor{42});

    ASSERT_EQ(report.PartitionWeights.size(), 3);
    EXPECT_EQ(report.PartitionWeights.at(TPartitionId{1}), TPartitionWeight{100});
    EXPECT_EQ(report.PartitionWeights.at(TPartitionId{2}), TPartitionWeight{200});
    EXPECT_EQ(report.PartitionWeights.at(TPartitionId{3}), TPartitionWeight{300});
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
