#pragma once

#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>
#include <core/domain/common/coordination_params.hpp>

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
        TPartitionImpact partitionImpact);

    const THubEndpoint& GetEndpoint() const;
    const THubDC& GetDC() const;
    EHubStatus GetStatus() const;
    TLoadFactor GetLoadFactor() const;
    TPartitionLoad GetExpectedLoadGrowth() const;
    TPartitionImpact GetPartitionImpact() const;

    void SetEndpoint(THubEndpoint endpoint);
    void SetDC(THubDC dc);
    void SetStatus(EHubStatus status);
    void SetLoadFactor(TLoadFactor loadFactor);
    void SetExpectedLoadGrowth(TPartitionLoad growth);
    void SetPartitionImpact(TPartitionImpact impact);

    void AddToExpectedLoadGrowth(TPartitionLoad growth);

private:
    THubEndpoint Endpoint_;
    THubDC DC_;
    EHubStatus Status_;
    TLoadFactor LoadFactor_;
    TPartitionLoad ExpectedLoadGrowth_; // For correct estimating of messages flow in partition
    TPartitionImpact PartitionImpact_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
