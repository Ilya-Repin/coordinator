#include "partition_map.hpp"

#include <core/partition/partition_map.hpp>
#include <core/hash_ring/hash_ring.hpp>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TEST(TPartitionMap, ThrowsOnZeroPartitions)
{
    EXPECT_THROW({
        BuildStartingPartitionMap(0);
    }, std::invalid_argument);
}

TEST(TPartitionMap, OnePartition)
{
    const auto result = BuildStartingPartitionMap(1);

    ASSERT_EQ(result.Partitions.size(), 1);
    ASSERT_EQ(result.Epoch.GetUnderlying(), 0);

    const auto& [partition, hub] = result.Partitions.front();

    EXPECT_EQ(partition.GetUnderlying(),
              std::numeric_limits<std::uint64_t>::max());
    EXPECT_TRUE(hub.empty());
}

TEST(TPartitionMap, CreatesCorrectAmountOfPartitions)
{
    constexpr std::size_t partitionsAmount = 10;

    const auto result = BuildStartingPartitionMap(partitionsAmount);

    ASSERT_EQ(result.Partitions.size(), partitionsAmount);
    ASSERT_EQ(result.Epoch.GetUnderlying(), 0);

    for (const auto& [partition, hub] : result.Partitions) {
        EXPECT_TRUE(hub.empty());
    }
}

TEST(TPartitionMap, PartitionsAreSortedByBoundary)
{
    constexpr std::size_t partitionsAmount = 8;

    const auto result = BuildStartingPartitionMap(partitionsAmount);
    
    ASSERT_EQ(result.Epoch.GetUnderlying(), 0);

    for (std::size_t i = 1; i < result.Partitions.size(); ++i) {
        EXPECT_LT(
            result.Partitions[i - 1].first.GetUnderlying(),
            result.Partitions[i].first.GetUnderlying()
        );
    }
}

TEST(TPartitionMap, LastPartitionHasMaxBoundary)
{
    constexpr std::size_t partitionsAmount = 16;

    const auto result = BuildStartingPartitionMap(partitionsAmount);

    ASSERT_FALSE(result.Partitions.empty());
    ASSERT_EQ(result.Epoch.GetUnderlying(), 0);

    EXPECT_EQ(
        result.Partitions.back().first.GetUnderlying(),
        std::numeric_limits<std::uint64_t>::max()
    );
}

TEST(TPartitionMap, UsesHashRingPartitionsAsIs)
{
    constexpr std::size_t partitionsAmount = 5;

    NCore::NDomain::THashRing hashRing(partitionsAmount);
    const auto expectedPartitions = hashRing.GetAllPartitions();

    const auto result = BuildStartingPartitionMap(partitionsAmount);
    
    ASSERT_EQ(result.Epoch.GetUnderlying(), 0);
    ASSERT_EQ(result.Partitions.size(), expectedPartitions.size());

    for (std::size_t i = 0; i < result.Partitions.size(); ++i) {
        EXPECT_EQ(
            result.Partitions[i].first.GetUnderlying(),
            expectedPartitions[i].GetUnderlying()
        );
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
