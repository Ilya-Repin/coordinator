#include "ring.hpp"

#include <algorithm>
#include <stdexcept>
#include <limits>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

THashRing::THashRing(HashFunction hasher)
    : Hasher_(std::move(hasher))
{ }

THashRing::THashRing(std::size_t partitionsCount, HashFunction hasher) 
    : Hasher_(std::move(hasher)) 
{
    InitializeWithCount(partitionsCount);
}

THashRing::THashRing(const std::vector<TPartitionId>& partitions, HashFunction hasher)
    : Hasher_(std::move(hasher))
{
    LoadPartitions(partitions);
}

void THashRing::InitializeWithCount(std::size_t partitionsCount)
{
    if (partitionsCount == 0) {
        throw std::invalid_argument("Partitions count must be > 0");
    }

    GeneratePartitions(partitionsCount);
}

void THashRing::LoadPartitions(const std::vector<TPartitionId>& partitions)
{
    if (partitions.empty()) {
        throw std::invalid_argument("Partitions list cannot be empty");
    }

    Partitions_ = partitions;
    SortAndDeduplicatePartitions();
}

bool THashRing::IsInitialized() const
{
    return !Partitions_.empty();
}

TPartitionId THashRing::GetPartition(const std::string& key) const
{
   if (!IsInitialized()) {
        throw std::logic_error("Hash ring is not initialized");
    }

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

void THashRing::GeneratePartitions(std::size_t count)
{
    Partitions_.clear();
    Partitions_.reserve(count);

    uint64_t step = std::numeric_limits<uint64_t>::max() / count;

    for (size_t i = 0; i < count; ++i) {
        Partitions_.emplace_back(i * step);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
