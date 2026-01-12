#pragma once

#include <core/domain/common/partition_params.hpp>

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class THashRing {
public:
    using HashFunction = std::function<std::uint64_t(const std::string&)>;

    explicit THashRing(const std::vector<TPartitionId>& partitions, HashFunction hasher = std::hash<std::string>{});

    void LoadPartitions(const std::vector<TPartitionId>& partitions);

    TPartitionId GetPartition(const std::string& key) const;
    const std::vector<TPartitionId>& GetAllPartitions() const;

private:
    void SortAndDeduplicatePartitions();

private:
    std::vector<TPartitionId> Partitions_;
    HashFunction Hasher_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain