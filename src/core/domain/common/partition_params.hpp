#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>

namespace NCoordinator::NCore::NDomain {

using TPartitionId = userver::utils::StrongTypedef<
    struct TPartitionIdTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

using TPartitionLoad = userver::utils::StrongTypedef<
    struct TPartitionLoadTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

using TPartitionImpact = userver::utils::StrongTypedef<
    struct TPartitionLoadTag,
    double,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

enum class EPartitionStatus {
    ACTIVE,
    MIGRATED,
};

}  // namespace NCoordinator::NCore::NDomain
