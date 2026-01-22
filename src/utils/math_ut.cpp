#include "math.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace {

using namespace NCoordinator::NUtils::NMath;

////////////////////////////////////////////////////////////////////////////////

struct TItem {
    std::uint64_t Weight;
};

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NUtils::NMath {

TEST(CalculateCV, ReturnsZeroForEmptyRange)
{
    EXPECT_EQ(CalculateCV(std::vector<uint64_t>{}), 0);
}

TEST(CalculateCV, ReturnsZeroForConstantValues)
{
    EXPECT_EQ(CalculateCV(std::vector{100, 100, 100}), 0);
}

TEST(CalculateCV, ReturnsZeroForZeroMean)
{
    EXPECT_EQ(CalculateCV(std::vector{0, 0, 0}), 0);
}

TEST(CalculateCV, CalculatesSimpleCV)
{
    // Data: {10, 30}
    // Mean = 20
    // Variance = ((10-20)^2 + (30-20)^2) / 2 = 100
    // StdDev = 10
    // CV = 10 / 20 = 0.5
    // CV * 100 = 50
    EXPECT_EQ(CalculateCV(std::vector<uint64_t>{10, 30}), 50);
}

TEST(CalculateCV, HandlesSmallVariationWithPrecision)
{
    // Data: {100, 110}
    // Mean = 105, StdDev ≈ 5
    // CV ≈ 5 / 105 ≈ 0.0476
    // CV * 100 ≈ 4.76 -> round to 5
    EXPECT_EQ(CalculateCV(std::vector<uint64_t>{100, 110}), 5);
}

TEST(CalculateCV, HighVariationTest)
{
    // Data: {0, 100}
    // Mean = 50, StdDev = 50
    // CV = 1.0 -> 100%
    EXPECT_EQ(CalculateCV(std::vector<uint64_t>{0, 100}), 100);
}

TEST(CalculateCV, WorksWithProjectionsAndWeights)
{
    std::vector<TItem> items = {{20}, {40}, {60}};
    // Mean = 40, StdDev = sqrt((400 + 0 + 400)/3) = sqrt(266.6) ≈ 16.33
    // CV = 16.33 / 40 = 0.408...
    // CV * 100 ≈ 41%
    
    auto cv = CalculateCV(items, &TItem::Weight);
    EXPECT_EQ(cv, 41);
}

} // namespace NCoordinator::NUtils::NMath
