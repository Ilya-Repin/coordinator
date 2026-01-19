#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>
#include <string>

namespace NCoordinator::NCore::NDomain {

using THubEndpoint = userver::utils::StrongTypedef<
    struct THubEndpointTag,
    std::string,
    userver::utils::StrongTypedefOps::kCompareStrong>;

using THubDC = userver::utils::StrongTypedef<
    struct THubDCTag,
    std::string,
    userver::utils::StrongTypedefOps::kCompareStrong>;

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

}  // namespace NCoordinator::NCore::NDomain
