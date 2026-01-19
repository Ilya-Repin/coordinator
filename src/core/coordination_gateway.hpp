#pragma once

#include <core/partitions/partition_map.hpp>

namespace NCoordinator::NCore {

////////////////////////////////////////////////////////////////////////////////

class ICoordinationGateway {
 public:
  virtual NDomain::TPartitionMap GetPartitionMap() const = 0;
  virtual void BroadcastPartitionMap(const NDomain::TPartitionMap& partitionMap) const = 0;

  virtual ~ICoordinationGateway() = default;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore
