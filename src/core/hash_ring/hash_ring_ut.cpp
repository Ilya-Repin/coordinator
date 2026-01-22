#include "hash_ring.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace {

////////////////////////////////////////////////////////////////////////////////

using namespace NCoordinator::NCore::NDomain;

uint64_t MockHasher(const std::string& key) {
    return std::stoull(key);
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TEST(THashRing, ThrowsOnEmptyPartitions)
{
    EXPECT_THROW({
        THashRing ring({}, MockHasher);
    }, std::invalid_argument);
}

TEST(THashRing, SortsAndDeduplicatesPartitions)
{
    std::vector<TPartitionId> raw = {
        TPartitionId(30), 
        TPartitionId(10), 
        TPartitionId(20), 
        TPartitionId(10)
    };
    
    THashRing ring(raw, MockHasher);
    
    const auto& partitions = ring.GetAllPartitions();
    
    ASSERT_EQ(partitions.size(), 3);
    EXPECT_EQ(partitions.at(0), TPartitionId(10));
    EXPECT_EQ(partitions.at(1), TPartitionId(20));
    EXPECT_EQ(partitions.at(2), TPartitionId(30));
}

TEST(THashRing, FindsCorrectPartition)
{
    THashRing ring({TPartitionId(100), TPartitionId(200), TPartitionId(300)}, MockHasher);

    EXPECT_EQ(ring.GetPartition("100"), TPartitionId(100));

    EXPECT_EQ(ring.GetPartition("150"), TPartitionId(200));
    EXPECT_EQ(ring.GetPartition("250"), TPartitionId(300));

    EXPECT_EQ(ring.GetPartition("099"), TPartitionId(100));
}

TEST(THashRing, WrapsAroundAtTheEndOfTheRing)
{
    THashRing ring({TPartitionId(10), TPartitionId(20), TPartitionId(30)}, MockHasher);

    EXPECT_EQ(ring.GetPartition("31"), TPartitionId(10));
    EXPECT_EQ(ring.GetPartition("999"), TPartitionId(10));
}

TEST(THashRing, WorksWithSinglePartition)
{
    THashRing ring({TPartitionId(42)}, MockHasher);

    EXPECT_EQ(ring.GetPartition("1"), TPartitionId(42));
    EXPECT_EQ(ring.GetPartition("42"), TPartitionId(42));
    EXPECT_EQ(ring.GetPartition("100"), TPartitionId(42));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
