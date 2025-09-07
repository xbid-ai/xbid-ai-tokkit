/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class Counter {
public:
    virtual ~Counter() = default;
    virtual std::string model() const = 0;
    virtual void prepare() const = 0;
    virtual size_t count(std::string_view text) const = 0;
};

inline void Counter::prepare() const {}