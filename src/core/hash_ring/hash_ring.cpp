#include "hash_ring.hpp"

#include <algorithm>
#include <stdexcept>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

THashRing::THashRing(const std::vector<TPartitionId>& partitions, HashFunction hasher)
    : Hasher_(std::move(hasher))
{
    LoadPartitions(partitions);
}

THashRing::THashRing(std::size_t partitionCount, HashFunction hasher)
    : Hasher_(std::move(hasher))
{
    if (partitionCount == 0) {
        throw std::invalid_argument("Partition count must be at least 1");
    }

    Partitions_.reserve(partitionCount);

    const std::uint64_t maxVal = std::numeric_limits<std::uint64_t>::max();
    const std::uint64_t step = maxVal / partitionCount; 

    for (std::size_t i = 1; i <= partitionCount; ++i) {
        std::uint64_t boundary;

        if (i == partitionCount) {
            boundary = maxVal;
        } else {
            boundary = step * i;
        }

        Partitions_.emplace_back(boundary);
    }
}

void THashRing::LoadPartitions(const std::vector<TPartitionId>& partitions)
{
    if (partitions.empty()) {
        throw std::invalid_argument("Partitions list cannot be empty");
    }

    Partitions_ = partitions;
    SortAndDeduplicatePartitions();
}

TPartitionId THashRing::GetPartition(const std::string& key) const
{
    const std::uint64_t hash = Hasher_(key);

    auto it = std::lower_bound(Partitions_.begin(), Partitions_.end(), hash);

    if (it == Partitions_.end()) {
        return Partitions_.front();
    }

    return *it;
}

const std::vector<TPartitionId>& THashRing::GetAllPartitions() const
{
    return Partitions_;
}

void THashRing::SortAndDeduplicatePartitions()
{
    std::sort(Partitions_.begin(), Partitions_.end());
    auto last = std::unique(Partitions_.begin(), Partitions_.end());
    Partitions_.erase(last, Partitions_.end());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
