#pragma once

#include <cstdint>
#include <cmath>
#include <numeric>
#include <ranges>

namespace NCoordinator::NUtils::NMath {

////////////////////////////////////////////////////////////////////////////////

template <std::ranges::forward_range Range, typename Projection = std::identity>
std::uint64_t CalculateCV(Range&& range, Projection proj = {})
{
    auto nums = range | std::views::transform(proj);

    if (std::ranges::empty(nums)) {
        return 0;
    }

    using valueType = std::ranges::range_value_t<decltype(nums)>;
    const auto sum = std::accumulate(std::ranges::begin(nums), 
                                     std::ranges::end(nums), 
                                     static_cast<valueType>(0));

    const long double mean = static_cast<long double>(sum) / std::ranges::size(nums);
    if (mean == 0.0L) {
        return 0;
    }

    long double variance = 0.0L;
    for (const auto& num : nums) {
        const long double diff = static_cast<long double>(num) - mean;
        variance += diff * diff;
    }
    variance /= std::ranges::size(nums);

    const long double stddev = std::sqrt(variance);

    const long double cv = stddev / mean;

    return static_cast<std::uint64_t>(std::llround(cv * 100));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCoordinator::NUtils::NMath
