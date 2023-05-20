//
// Created by Dieter Baron on 09.05.23.
//

#ifndef SIZE_RANGE_H
#define SIZE_RANGE_H

#include <cstdint>
#include <optional>
#include <ostream>

class SizeRange {
public:
    SizeRange() = default;
    explicit SizeRange(uint64_t size): minimum(size), maximum(size) {}
    SizeRange(uint64_t minimum, std::optional<uint64_t> maximum): minimum(minimum), maximum(maximum) {}

    [[nodiscard]] std::optional<uint64_t> size() const;

    SizeRange operator-(const SizeRange& other) const;
    SizeRange operator+(const SizeRange& other) const;
    SizeRange operator+=(const SizeRange& other) {*this = *this + other; return *this;}

    uint64_t minimum = 0;
    std::optional<uint64_t> maximum = 0;
};

std::ostream& operator<<(std::ostream& stream, const SizeRange& size_range);

#endif // SIZE_RANGE_H
