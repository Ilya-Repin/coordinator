#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

using TPartitionId = userver::utils::StrongTypedef<
    struct TPartitionIdTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

using TPartitionWeight = userver::utils::StrongTypedef<
    struct TPartitionWeightTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

////////////////////////////////////////////////////////////////////////////////

inline TPartitionWeight operator+(const TPartitionWeight& lhs, const TPartitionWeight& rhs) {
    return TPartitionWeight{lhs.GetUnderlying() + rhs.GetUnderlying()};
}

inline TPartitionWeight operator-(const TPartitionWeight& lhs, const TPartitionWeight& rhs) {
    return TPartitionWeight{lhs.GetUnderlying() - rhs.GetUnderlying()};
}

inline TPartitionWeight operator+=(TPartitionWeight& lhs, const TPartitionWeight& rhs) {
    lhs.GetUnderlying() += rhs.GetUnderlying();
    return lhs;
}

inline TPartitionWeight operator-=(TPartitionWeight& lhs, const TPartitionWeight& rhs) {
    lhs.GetUnderlying() -= rhs.GetUnderlying();
    return lhs;
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
