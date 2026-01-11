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

    explicit THashRing(HashFunction hasher = std::hash<std::string>{});

    explicit THashRing(std::size_t partitionsCount, HashFunction hasher = std::hash<std::string>{});

    explicit THashRing(const std::vector<TPartitionId>& partitions, HashFunction hasher = std::hash<std::string>{});

    void InitializeWithCount(std::size_t partitionsCount);
    void LoadPartitions(const std::vector<TPartitionId>& partitions);

    bool IsInitialized() const;

    TPartitionId GetPartition(const std::string& key) const;
    const std::vector<TPartitionId>& GetAllPartitions() const;

private:
    void SortAndDeduplicatePartitions();
    void GeneratePartitions(std::size_t count);

private:
    std::vector<TPartitionId> Partitions_;
    HashFunction Hasher_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain