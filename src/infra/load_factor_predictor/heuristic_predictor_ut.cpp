#include "heuristic_predictor.hpp"

#include <gtest/gtest.h>

namespace {

////////////////////////////////////////////////////////////////////////////////

using namespace NCoordinator::NCore;
using namespace NCoordinator::NCore::NDomain;
using namespace NCoordinator::NInfra;

TPredictionParams MakeParams(
    uint64_t currentLf,
    uint64_t totalWeight,
    uint64_t deltaWeight,
    bool increasing,
    size_t totalPartitions = 10)
{
    TPredictionParams p;
    p.LoadFactor = TLoadFactor{currentLf};
    p.PartitionsWeight = TPartitionWeight{totalWeight};
    p.PartitionWeight = TPartitionWeight{deltaWeight};
    p.Increasing = increasing;
    p.TotalPartitions = totalPartitions;

    p.OriginalLoadFactor = TLoadFactor{currentLf}; 
    return p;
}

////////////////////////////////////////////////////////////////////////////////

} // anonymous namespace

namespace NCoordinator::NInfra {

////////////////////////////////////////////////////////////////////////////////

TEST(THeuristicPredictor, LinearGrowth)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(50, 100, 20, true);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 60);
}

TEST(THeuristicPredictor, LinearDecline)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(50, 100, 20, false);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 40);
}

TEST(THeuristicPredictor, FirstPartitionInsertionUsesDefault)
{
    const THeuristicPredictor predictor;

    TPredictionParams params;
    params.LoadFactor = TLoadFactor{0};
    params.PartitionsWeight = TPartitionWeight{0};
    params.PartitionWeight = TPartitionWeight{10};
    params.Increasing = true;
    params.TotalPartitions = 0;

    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 5);
}

TEST(THeuristicPredictor, ClampsToMaxLoadFactor)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(80, 100, 50, true);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 100);
}

TEST(THeuristicPredictor, ClampsToZeroLoadFactor)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(10, 100, 100, false);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 0);
}

TEST(THeuristicPredictor, RoundsUpPessimistically)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(10, 100, 1, true);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 11);
}

TEST(THeuristicPredictor, HandlesZeroDeltaWeight)
{
    const THeuristicPredictor predictor;

    auto params = MakeParams(50, 100, 0, true);
    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 50);
}

TEST(THeuristicPredictor, HandlesRemovalFromEmptySafeGuard)
{
    const THeuristicPredictor predictor;

    TPredictionParams params;
    params.PartitionsWeight = TPartitionWeight{0};
    params.Increasing = false;
    params.TotalPartitions = 0;

    auto result = predictor.PredictLoadFactor(params);

    EXPECT_EQ(result.GetUnderlying(), 0);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NInfra
