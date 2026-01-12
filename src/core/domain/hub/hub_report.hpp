#pragma once

#include <core/domain/common/hub_params.hpp>
#include <core/domain/common/partition_params.hpp>
#include <core/domain/common/coordination_params.hpp>

#include <unordered_map>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

class THubReport {
public:
    using TPartitionLoads = std::unordered_map<TPartitionId, std::pair<TPartitionLoad, EPartitionStatus>>;
public:
    THubReport(
        TEpoch epoch,
        THubEndpoint endpoint,
        THubDC dc,
        TLoadFactor loadFactor,
        TPartitionLoads partitionLoads);

    TEpoch GetEpoch() const;
    const THubEndpoint& GetEndpoint() const;
    const THubDC& GetDC() const;
    TLoadFactor GetLoadFactor() const;
    const TPartitionLoads& GetPartitionLoads() const;

    void SetEpoch(TEpoch epoch);
    void SetEndpoint(THubEndpoint endpoint);
    void SetDC(THubDC dc);
    void SetLoadFactor(TLoadFactor loadFactor);
    void SetPartitionLoads(TPartitionLoads loads);

private:
    TEpoch Epoch_;
    THubEndpoint Endpoint_;
    THubDC DC_;
    TLoadFactor LoadFactor_;
    TPartitionLoads PartitionLoads_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore::NDomain
