#include <core/partition_balancing/balancing_impl.hpp>

#include "test_helpers_ut.hpp"
#include "load_factor_predictor_mock.hpp"

#include <gtest/gtest.h>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

using namespace testing;
using namespace NDomain;
using namespace NDetail;

class AccumulateMigratingWeightTest
    : public TBalancingTestBase
{ };

TEST_F(AccumulateMigratingWeightTest, CorrectlyAccumulatesWeight) {
    TMigratingWeight migratingWeight{
        {HUB("hub-a"), {3, PW(1200)}},
        {HUB("hub-b"), {1, PW(100)}},
        {HUB("hub-c"), {10, PW(900)}},
        {HUB("hub-d"), {4, PW(700)}},
    };
    
    auto result = AccumulateMigratingWeight(migratingWeight);
    EXPECT_EQ(result, PW(2900));
}

TEST_F(AccumulateMigratingWeightTest, HandlesEmptyMigratingWeight) {
    TMigratingWeight migratingWeight;
    
    auto result = AccumulateMigratingWeight(migratingWeight);
    EXPECT_EQ(result, PW(0));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
