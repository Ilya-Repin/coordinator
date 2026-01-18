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

}  // namespace NCoordinator::NCore::NDomain
