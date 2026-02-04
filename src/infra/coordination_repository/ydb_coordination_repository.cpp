#include "ydb_coordination_repository.hpp"

#include <core/common/coordination_params.hpp>
#include <core/common/partition_params.hpp>
#include <infra/dynconfig/leader/repository_config.hpp>

#include <userver/ydb/io/structs.hpp>

#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <optional>
#include <vector>

namespace {

////////////////////////////////////////////////////////////////////////////////

const userver::ydb::Query LOAD_COORDINATION_CONTEXT_QUERY = {
R"-(
SELECT 
    partition_id, 
    cooldown_epoch, 
    weight 
FROM 
    coordination_context;
)-",
    userver::ydb::Query::NameLiteral("load_coordination_context"),
    userver::ydb::Query::LogMode::kFull,
};

const userver::ydb::Query SET_COORDINATION_CONTEXT_QUERY = {
R"-(
DECLARE $rows AS List<Struct<partition_id: Uint64, cooldown_epoch: Uint64?, weight: Uint64?>>;
DECLARE $active_ids AS List<Uint64>;

DELETE FROM coordination_context 
WHERE partition_id NOT IN $active_ids;

REPLACE INTO coordination_context
SELECT * FROM AS_TABLE($rows);
)-",
    userver::ydb::Query::NameLiteral("update_coordination_context"),
    userver::ydb::Query::LogMode::kFull,
};

struct TSetCoordinationContextRequest {
    static constexpr userver::ydb::StructMemberNames<3> kYdbMemberNames{{
        {"PartitionId", "partition_id"},
        {"CooldownEpoch", "cooldown_epoch"},
        {"Weight", "weight"},
    }};

    std::uint64_t PartitionId;
    std::optional<std::uint64_t> CooldownEpoch;
    std::optional<std::uint64_t> Weight;
};

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NInfra::NRepository {

////////////////////////////////////////////////////////////////////////////////

TYdbCoordinationRepository::TYdbCoordinationRepository(
    std::shared_ptr<userver::ydb::TableClient> ydbClient,
    userver::dynamic_config::Source configSource)
    : YdbClient_(std::move(ydbClient))
    , ConfigSource_(std::move(configSource))
{ }

NCore::NDomain::TCoordinationContext TYdbCoordinationRepository::GetCoordinationContext() const
{
    const auto snapshot = ConfigSource_.GetSnapshot();
    
    userver::ydb::OperationSettings queryParams = snapshot[REPOSITORY_CONFIG].GetContextSettings;
    queryParams.tx_mode = userver::ydb::TransactionMode::kOnlineRO;

    auto response = YdbClient_->ExecuteDataQuery(
        queryParams,
        LOAD_COORDINATION_CONTEXT_QUERY);

    if (response.GetCursorCount() != 1) {
        throw std::runtime_error("Unexpected response data");
    }

    NCore::NDomain::TCoordinationContext context;
    for (auto row : response.GetSingleCursor()) {
        auto partitionId = NCore::NDomain::TPartitionId{
            row.Get<std::uint64_t>("partition_id")
        };
        auto cooldownEpoch = row.Get<std::optional<std::uint64_t>>("cooldown_epoch");
        auto weight = row.Get<std::optional<std::uint64_t>>("weight");

        if (cooldownEpoch.has_value()) {
            context.PartitionCooldowns[partitionId] = NCore::NDomain::TEpoch{cooldownEpoch.value()};
        }
        if (weight.has_value()) {
            context.PartitionWeights[partitionId] =  NCore::NDomain::TPartitionWeight{weight.value()};
        }
    }

    return context;
}

void TYdbCoordinationRepository::SetCoordinationContext(const NCore::NDomain::TCoordinationContext& context) const
{
    std::vector<TSetCoordinationContextRequest> rows;
    std::vector<std::uint64_t> activeIds;

    std::unordered_set<NCore::NDomain::TPartitionId> partitionIds;
    for (const auto& [partitionId, _] : context.PartitionCooldowns) {
        partitionIds.insert(partitionId);
    }
    for (const auto& [partitionId, _] : context.PartitionWeights) {
        partitionIds.insert(partitionId);
    }

    rows.reserve(partitionIds.size());
    activeIds.reserve(partitionIds.size());

    for (auto partitionId : partitionIds) {
        TSetCoordinationContextRequest row;
        row.PartitionId = partitionId.GetUnderlying();

        if (auto it = context.PartitionCooldowns.find(partitionId);
            it != context.PartitionCooldowns.end())
        {
            row.CooldownEpoch = it->second.GetUnderlying();
        }

        if (auto it = context.PartitionWeights.find(partitionId);
            it != context.PartitionWeights.end())
        {
            row.Weight = it->second.GetUnderlying();
        }

        rows.emplace_back(std::move(row));
        activeIds.emplace_back(partitionId);
    }

    const auto snapshot = ConfigSource_.GetSnapshot();
    
    userver::ydb::OperationSettings queryParams = snapshot[REPOSITORY_CONFIG].SetContextSettings;
    queryParams.tx_mode = userver::ydb::TransactionMode::kSerializableRW;

    auto response = YdbClient_->ExecuteDataQuery(
        queryParams,
        SET_COORDINATION_CONTEXT_QUERY,
        "$rows",
        rows,
        "$active_ids",
        activeIds);

    // TODO handling errors

    if (response.GetCursorCount()) {
        throw std::runtime_error("Unexpected response data");
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra::NRepository
