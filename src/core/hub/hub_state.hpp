#pragma once

#include <core/common/hub_params.hpp>
#include <core/common/partition_params.hpp>
#include <core/common/coordination_params.hpp>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class THubState {
public:
    THubState(
        THubEndpoint endpoint,
        THubDC dc,
        EHubStatus status,
        TLoadFactor loadFactor,
        TPartitionLoad expectedLoadGrowth,
        TPartitionLoad partitionsLoad,
        std::size_t totalPartitions);

    const THubEndpoint& GetEndpoint() const;
    const THubDC& GetDC() const;
    EHubStatus GetStatus() const;
    TLoadFactor GetLoadFactor() const;
    TPartitionLoad GetExpectedLoadGrowth() const;
    TPartitionLoad GetPartitionsLoad() const;
    std::size_t GetTotalPartitions() const;

    void SetEndpoint(THubEndpoint endpoint);
    void SetDC(THubDC dc);
    void SetStatus(EHubStatus status);
    void SetLoadFactor(TLoadFactor loadFactor);
    void SetExpectedLoadGrowth(TPartitionLoad growth);
    void SetPartitionsLoad(TPartitionLoad load);
    void SetTotalPartitions(std::size_t totalPartitions);

    void AddToExpectedLoadGrowth(TPartitionLoad growth);

private:
    THubEndpoint Endpoint_;
    THubDC DC_;
    EHubStatus Status_;
    TLoadFactor LoadFactor_;
    TPartitionLoad ExpectedLoadGrowth_; // For correct estimating of messages flow in partition
    TPartitionLoad PartitionsLoad_;
    std::size_t TotalPartitions_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
