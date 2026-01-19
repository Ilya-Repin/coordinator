#pragma once

#include <userver/utils/strong_typedef.hpp>

#include <cstdint>

namespace NCoordinator::NCore::NDomain {

using TEpoch = userver::utils::StrongTypedef<
    struct TEpochTag,
    std::uint64_t,
    userver::utils::StrongTypedefOps::kCompareTransparent>;

}  // namespace NCoordinator::NCore::NDomain
