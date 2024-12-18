#pragma once
#include <array>
#include <cstddef>
#include <iostream>

template < typename T, std::size_t CAPACITY >
struct ArrayVec {
    std::size_t elements;
    std::array<T, CAPACITY> buffer;

    explicit ArrayVec(int m_elements) : elements(m_elements), buffer() {
    }
    explicit ArrayVec(std::array<T, CAPACITY> m_buffer, int m_elements) : elements(m_elements), buffer(m_buffer) {
    }
};
