#pragma once

#include <cstdint>
#include <unordered_map>

// TODO to delete

namespace NCoordinator::NCore::NDomain {

struct TCoordinationContext {
    std::unordered_map<std::uint64_t, std::uint64_t> PartitionFreezeEpoch;
    std::unordered_map<std::uint64_t, std::uint64_t> PartitionLoads; 
};

} 