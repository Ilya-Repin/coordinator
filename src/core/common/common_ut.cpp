#include "coordination_params.hpp"
#include "hub_params.hpp"
#include "partition_params.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

TEST(TPartitionWeight, ArithmeticOperators)
{
    TPartitionWeight w1{100};
    TPartitionWeight w2{50};

    auto sum = w1 + w2;
    EXPECT_EQ(sum.GetUnderlying(), 150);
    static_assert(std::is_same_v<decltype(sum), TPartitionWeight>);

    auto diff = w1 - w2;
    EXPECT_EQ(diff.GetUnderlying(), 50);

    w1 += w2;
    EXPECT_EQ(w1.GetUnderlying(), 150);

    w1 -= TPartitionWeight{100};
    EXPECT_EQ(w1.GetUnderlying(), 50);
}

TEST(TEpoch, ArithmeticOperators)
{
    TEpoch e1{100};
    TEpoch e2{50};

    auto sum = e1 + e2;
    EXPECT_EQ(sum.GetUnderlying(), 150);
    static_assert(std::is_same_v<decltype(sum), TEpoch>);

    auto diff = e1 - e2;
    EXPECT_EQ(diff.GetUnderlying(), 50);
}

TEST(TLoadFactor, ArithmeticOperators)
{
    TLoadFactor lf1{35};
    TLoadFactor lf2{10};

    auto sum = lf1 + lf2;
    EXPECT_EQ(sum.GetUnderlying(), 45);
    static_assert(std::is_same_v<decltype(sum), TLoadFactor>);

    auto diff = lf1 - lf2;
    EXPECT_EQ(diff.GetUnderlying(), 25);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain