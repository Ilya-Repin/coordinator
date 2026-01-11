#pragma once

#include <core/domain/hub/hub.hpp>
#include <core/domain/partition/partition_map.hpp>
#include <core/domain/ring/ring.hpp>

#include <unordered_set>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

class TPartitionBalancer {
public:
  using TPartitionStates = std::unordered_map<NDomain::TPartitionId, NDomain::TPartitionState>;
  using THubStates = std::unordered_map<NDomain::THubEndpoint, NDomain::THubState>;

  struct TCoordinationState {
    TPartitionStates StateByPartition{};
    THubStates StateByHub{};
  };

  struct TBalancingSettings {
    std::size_t LaggedEpochsThreshold;
    std::size_t MinBalancingCV;
    NDomain::TLoadFactor OverloadThreshold;
    std::unordered_set<NDomain::THubDC> BlockedDCs{};
    std::unordered_set<NDomain::THubEndpoint> BlockedHubs{};
  };

public:
  using TClusterSnapshot = std::unordered_map<NDomain::THubEndpoint, NDomain::THubReport>;

  bool GetIsLeader() const;
  NDomain::TEpoch GetCoordinationEpoch() const;
  NDomain::TPartitionMap GetPartitionMap() const;
  const TCoordinationState& GetCoordinationState() const;

  void Rebalance(const TClusterSnapshot& snapshot, const TBalancingSettings& settings);
  
  void SetIsLeader(const bool isLeader);
  void SetEpoch(const NDomain::TEpoch epoch);
  
  void InitializePartitions(const std::vector<NDomain::TPartitionId>& partitions);
  void InitializePartitions(const std::size_t partitionsCount);

  void ApplyPartitionMap(const NDomain::TPartitionMap& partitionMap);
  void ApplyPartitionStates(const TPartitionStates& stat);
  void ApplyHubStates(const THubStates& stat);

private:
  NDomain::EHubStatus DetermineHubStatus(
    const NDomain::THubState& hubState,
    const NDomain::THubReport& report,
    const TBalancingSettings& settings) const;

  void DrainHub(
    const NDomain::THubEndpoint& hub,
    std::unordered_set<NDomain::TPartitionId>& orphanedPartitions);

private:
  bool IsLeader_{false};
  NDomain::TEpoch CoordinationEpoch_{0};
  NDomain::THashRing HashRing_{};

  std::unordered_map<NDomain::THubEndpoint, std::unordered_set<NDomain::TPartitionId>> PartitionsByHub_;
  TCoordinationState CoordinationState_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NCore
