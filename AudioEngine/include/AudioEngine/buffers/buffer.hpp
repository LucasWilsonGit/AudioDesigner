#pragma once

#include <span>

namespace AudioEngine {
    template <class T>
    concept dsp_buffer = requires (T const& readable, T& writable, std::span<typename T::ValueType> data, size_t offs, size_t len) {
        typename T::ValueType; //T must have ::ValueType
        { readable.get(offs) } -> std::same_as<typename T::ValueType&>;
        { readable.view(offs, len) } -> std::same_as<std::span<typename T::ValueType>>;
        { writable.store(data) } -> std::same_as<void>;
        { writable.store(offs, data) } -> std::same_as<void>;
    };
}