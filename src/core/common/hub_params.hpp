#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>
#include <string>

namespace NCoordinator::NCore::NDomain {

////////////////////////////////////////////////////////////////////////////////

using THubEndpoint = userver::utils::StrongTypedef<
    struct THubEndpointTag,
    std::string,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

using THubDC = userver::utils::StrongTypedef<
    struct THubDCTag,
    std::string,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

using TLoadFactor = userver::utils::StrongTypedef<
    struct TLoadFactorTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

enum class EHubStatus {
    HEALTHY,
    DRAINING,
    OVERLOADED,
    // SUSPECTED,
    // UNHEALTHY,
    LAGGED,
};

////////////////////////////////////////////////////////////////////////////////

inline TLoadFactor operator+(const TLoadFactor& lhs, const TLoadFactor& rhs) {
    return TLoadFactor{lhs.GetUnderlying() + rhs.GetUnderlying()};
}

inline TLoadFactor operator-(const TLoadFactor& lhs, const TLoadFactor& rhs) {
    return TLoadFactor{lhs.GetUnderlying() - rhs.GetUnderlying()};
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace NCoordinator::NCore::NDomain
