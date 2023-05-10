//
// Created by Dieter Baron on 09.05.23.
//

#ifndef SIZE_RANGE_H
#define SIZE_RANGE_H

#include <cstdint>
#include <optional>

class SizeRange {
public:
    SizeRange() = default;
    explicit SizeRange(uint64_t size): minimum(size), maximum(size) {}
    SizeRange(uint64_t minimum, std::optional<uint64_t> maximum): minimum(minimum), maximum(maximum) {}

    [[nodiscard]] std::optional<uint64_t> size() {return minimum == maximum ? minimum : std::optional<uint64_t>();}

    SizeRange operator+(const SizeRange& other) const;
    SizeRange operator+=(const SizeRange& other) {*this = *this + other; return *this;}

    uint64_t minimum = 0;
    std::optional<uint64_t> maximum;
};


#endif // SIZE_RANGE_H
