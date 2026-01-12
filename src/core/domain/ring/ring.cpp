#include "ring.hpp"

#include <algorithm>
#include <stdexcept>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

THashRing::THashRing(const std::vector<TPartitionId>& partitions, HashFunction hasher)
    : Hasher_(std::move(hasher))
{
    LoadPartitions(partitions);
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
    const uint64_t hash = Hasher_(key);

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
