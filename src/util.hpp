#pragma once
#include <fstream>
#include <sstream>
#include <optional>
#include <variant>

// Rust style error handling
template<typename T, typename E>
inline constexpr bool isOk(std::variant<T, E> const& variant) {
    return std::holds_alternative<T>(variant);
}

template<typename T, typename E>
inline constexpr bool isErr(std::variant<T, E> const& variant) {
    return std::holds_alternative<E>(variant);
}

template<typename V, typename E>
inline constexpr V unwrap(std::variant<V,E>&& variant) noexcept {
    // assert(isOk(variant));
    return std::get<V>(std::move(variant));
}

template<typename V, typename E>
inline constexpr E unwrapErr(std::variant<V,E>&& variant) noexcept {
    // assert(isErr(variant));
    return std::get<E>(std::move(variant));
}

auto readTextFile(std::string_view filepath) -> std::optional<std::string>;