#pragma once

#include <algorithm>
#include <numeric>
#include <random>

template <class C>
inline void rshuffle(C& vec, std::default_random_engine& g) {
	std::shuffle(vec.begin(), vec.end(), g);
}

template<typename T>
inline std::vector<size_t> argsort(const std::vector<T>& array) {
	std::vector<size_t> indices(array.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(),
		[&array](int left, int right) -> bool {
		// sort indices according to corresponding array element
		return array[left] < array[right];
	});

	return indices;
}

template <class T>
class Enumerator {
public:
	struct Iterator {
	public:
		Iterator(const std::vector<T>& cont, size_t i)
			: cont_(cont)
			, idx(i)
		{ }

		std::pair<const T&, size_t> operator*() {
			return { cont_.at(idx), idx };
		}

		void operator++() {
			idx++;
		}

		bool operator!=(const Iterator& rhs) {
			return idx != rhs.idx;
		}

		bool operator==(const Iterator& rhs) {
			return idx == rhs.idx;
		}

	private:
		const std::vector<T>& cont_;
		size_t idx;
	};

	Enumerator(const std::vector<T>& cont)
		: cont_(cont)
	{ }

	Iterator begin() {
		return Iterator{ cont_, 0 };
	}

	Iterator end() {
		return Iterator(cont_, (size_t)cont_.size());
	}

private:
	const std::vector<T>& cont_;
};

template <class T>
class Reverser {
public:
	struct Iterator {
	public:
		Iterator(const std::vector<T>& cont, size_t i)
			: cont_(cont)
			, idx(i)
		{ }

		const T& operator*() {
			return cont_.at(cont_.size() - idx - 1);
		}

		void operator++() {
			idx++;
		}

		bool operator!=(const Iterator& rhs) {
			return idx != rhs.idx;
		}

		bool operator==(const Iterator& rhs) {
			return idx == rhs.idx;
		}

	private:
		const std::vector<T>& cont_;
		size_t idx;
	};

	Reverser(const std::vector<T>& cont)
		: cont_(cont)
	{ }

	Iterator begin() {
		return Iterator{ cont_, 0 };
	}

	Iterator end() {
		return Iterator(cont_, (size_t)cont_.size());
	}

private:
	const std::vector<T>& cont_;
};

template <class T>
inline Enumerator<T> enumerate(const std::vector<T>& cont) {
	return Enumerator(cont);
}

template <class T>
inline Reverser<T> reversed(const std::vector<T>& cont) {
	return Reverser(cont);
}
