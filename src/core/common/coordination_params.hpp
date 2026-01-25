#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

using TEpoch = userver::utils::StrongTypedef<
    struct TEpochTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

////////////////////////////////////////////////////////////////////////////////

inline TEpoch operator+(const TEpoch& lhs, const TEpoch& rhs) {
    return TEpoch{lhs.GetUnderlying() + rhs.GetUnderlying()};
}

inline TEpoch operator-(const TEpoch& lhs, const TEpoch& rhs) {
    return TEpoch{lhs.GetUnderlying() - rhs.GetUnderlying()};
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
