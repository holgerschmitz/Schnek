#ifndef SCHNEK_UTIL_VOLUME_HPP
#define SCHNEK_UTIL_VOLUME_HPP

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

namespace schnek {

	template<class T, std::size_t rank, template<std::size_t> class CheckingPolicy>
	class Range;

	constexpr std::size_t dynamic_rank = std::numeric_limits<std::size_t>::max();

	namespace detail {

		template<typename T>
		struct volume_rank : std::integral_constant<std::size_t, dynamic_rank> {};

		template<typename T, std::size_t rank, template<std::size_t> class CheckingPolicy>
		struct volume_rank<Range<T, rank, CheckingPolicy>> : std::integral_constant<std::size_t, rank> {};

		template<typename T>
		struct is_bounds_pair : std::false_type {};

		template<typename T, typename U>
		struct is_bounds_pair<std::pair<T, U>> : std::true_type {};

	}  // namespace detail

	template<std::size_t Rank = dynamic_rank, typename Bounds>
	std::size_t volume(const Bounds &bounds) {
		constexpr std::size_t boundsRank = detail::volume_rank<std::decay_t<Bounds>>::value;
		constexpr std::size_t rank = Rank == dynamic_rank ? boundsRank : Rank;

		if constexpr (detail::is_bounds_pair<std::decay_t<Bounds>>::value) {
			const auto &lo = bounds.first;
			const auto &hi = bounds.second;
			const std::size_t dimensions = [&] {
				if constexpr (rank == dynamic_rank) {
					return lo.size();
				} else {
					return rank;
				}
			}();

			std::size_t result = 1;
			for (std::size_t d = 0; d < dimensions; ++d) {
				const auto extent = hi[d] - lo[d] + 1;
				if (extent <= 0) {
					return 0;
				}
				result *= static_cast<std::size_t>(extent);
			}
			return result;
		} else {
			const auto &lo = bounds.getLo();
			const auto &hi = bounds.getHi();
			const std::size_t dimensions = [&] {
				if constexpr (rank == dynamic_rank) {
					return lo.size();
				} else {
					return rank;
				}
			}();

			std::size_t result = 1;
			for (std::size_t d = 0; d < dimensions; ++d) {
				const auto extent = hi[d] - lo[d] + 1;
				if (extent <= 0) {
					return 0;
				}
				result *= static_cast<std::size_t>(extent);
			}
			return result;
		}
	}

}  // namespace schnek

#endif
