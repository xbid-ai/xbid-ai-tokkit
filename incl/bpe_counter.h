/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <stdexcept>
#include <array>
#include <bit>
#include <deque>
#include <algorithm>

#include "misc.h"
#include "counter.h"

// Greedy BPE tokenizer counter, no materialized IDs/ranks.
class BpeCounter : public Counter {
public:
    using Tokens = std::unordered_set<std::string_view, std::hash<std::string_view>>;

    explicit BpeCounter(std::string model)
        : _model(std::move(model)) {
    }

    std::string model() const override { return _model; }

    void prepare() const override {
        if (!_entry.tokens.empty()) {
            return;
        }
        Entry entry;
        entry.lenMaskByFirst.fill(1ull << 0);
        entry.lenMaskByFirstTwo.fill(0ull);
        loadEncoding(_model, entry);
        _entry = std::move(entry);
    }

    size_t count(std::string_view text) const override {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(text.data());
        size_t size = text.size();
        if (size == 0) {
            return 0;
        }
 
        const Tokens &tokens = _entry.tokens;
        const auto &mask1 = _entry.lenMaskByFirst;
        const auto &mask2 = _entry.lenMaskByFirstTwo;

        // Greedy longest-match search.
        size_t i = 0, count = 0;
        while (i < size) {
            auto remain = std::min<uint32_t>(size - i, 64u);
            uint64_t m = mask1[data[i]];
            if (remain >= 2) {
                if (uint64_t m2 = mask2[(uint16_t(data[i]) << 8) | data[i + 1]]) {
                    m = (m2 | 1ull);
                }
            }

            m &= (remain == 64u) ? ~0ull : ((1ull << remain) - 1ull);

            const char *base = reinterpret_cast<const char *>(data + i);
            uint32_t len = 1;
            uint64_t scan = m;
            while (scan) {
                const unsigned bit = 63u - std::countl_zero(scan);
                if (tokens.contains(std::string_view(base, bit + 1))) {
                    len = bit + 1;
                    break;
                }
                scan ^= (1ull << bit);
            }

            i += len;
            ++count;
        }
        return count;
    }

protected:
    struct Entry {
        Tokens tokens;
        std::deque<std::string> storage;
        std::array<uint64_t, 256> lenMaskByFirst{};
        std::array<uint64_t, 65536> lenMaskByFirstTwo{};
    };

    virtual void loadEncoding(const std::string &path, Entry &out) const = 0;

    mutable Entry _entry;
    std::string _model;
};
