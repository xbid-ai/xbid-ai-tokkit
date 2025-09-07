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
#include "counter.h"

// Naive token counter. Should only be used as fallback.
class DefaultCounter final : public Counter {
public:
    explicit DefaultCounter(std::string model)
        : _model(std::move(model)) {}

    std::string model() const override { return _model; }

    void prepare() const override { Counter::prepare(); }

    size_t count(std::string_view text) const override {
        const size_t n = text.size();
        // ceil(n/4) = (n + 3) >> 2 (size_t)
        return (n + 3) >> 2;
    }

private:
    std::string _model;
};
