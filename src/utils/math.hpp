#pragma once

#include <cstdint>
#include <cmath>
#include <numeric>
#include <ranges>

namespace NCoordinator::NUtils::NMath {

////////////////////////////////////////////////////////////////////////////////

template <std::ranges::range Range, typename Projection = std::identity>
std::uint64_t CalculateCV2(Range&& range, Projection proj = {}) {
    auto nums = range | std::views::transform(proj);

    if (nums.empty()) {
        return 0;
    }

    const auto sum = std::accumulate(nums.begin(), nums.end(), 0);

    const long double mean = static_cast<long double>(sum) / nums.size();
    if (mean == 0.0L) {
        return 0;
    }

    long double variance = 0.0L;
    for (const auto& num : nums) {
        const long double diff = static_cast<long double>(num) - mean;
        variance += diff * diff;
    }
    variance /= nums.size();

    const long double stddev = std::sqrt(variance);

    const long double cv = stddev / mean;

    return static_cast<std::size_t>(std::llround(cv));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NUtils::NMath
