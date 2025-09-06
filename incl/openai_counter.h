/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

// Greedy BPE tokenizer counter for openai (tiktoken), no materialized IDs/ranks.
// Only quasi-parity, trading parity for speed (no templates).

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <stdexcept>
#include <fstream>
#include <mutex>
#include <memory>
#include <array>
#include <bit>
#include <unordered_map>
#include <deque>
#include <algorithm>

#include "misc.h"
#include "counter.h"

class OpenAiCounter final : public Counter {
public:
    using Tokens = std::unordered_set<std::string_view, std::hash<std::string_view>>;

    static void prepare(const std::string &model) {
        auto &map = cache();
        auto &mut = cacheMutex();
        {
            std::lock_guard<std::mutex> lock(mut);
            if (map.find(model) != map.end()) {
                return;
            }
        }
        auto entry = std::make_shared<CacheEntry>();
        loadEncoding(model, *entry);
        std::lock_guard<std::mutex> lock(mut);
        if (map.find(model) == map.end()) {
            map.emplace(model, std::move(entry));
        }
    }

    explicit OpenAiCounter(std::string model)
        : _model(std::move(model)) {
        auto &m = cache();
        auto &mu = cacheMutex();
        std::lock_guard<std::mutex> lock(mu);
        auto it = m.find(_model);
        if (it == m.end()) {
            throw std::runtime_error("Model not found: " + _model);
        }
        _entry = it->second;
    }

    std::string model() const override { return _model; }

    size_t count(std::string_view text) const override {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(text.data());
        size_t size = text.size();
        if (size == 0) {
            return 0;
        }
 
        const Tokens &tokens = _entry->tokens;
        const auto &mask1 = _entry->lenMaskByFirst;
        const auto &mask2 = _entry->lenMaskByFirstTwo;

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

private:
    struct CacheEntry {
        Tokens tokens;
        std::deque<std::string> storage;
        std::array<uint64_t, 256> lenMaskByFirst{};
        std::array<uint64_t, 65536> lenMaskByFirstTwo{};
    };

    //  Encoding loader (not storing ranks)
    // .tiktoken format: <base64(token)><space><rank>
    static void loadEncoding(const std::string &path, CacheEntry &out) {
        std::ifstream in(path);
        if (!in) {
            throw std::runtime_error("Encoding not found: " + path);
        }

        out.lenMaskByFirst.fill(1ull << 0);
        out.lenMaskByFirstTwo.fill(0ull);

        std::string line;
        while (std::getline(in, line)) {
            const size_t split = line.find_first_of(" \t");
            if (split == std::string::npos) {
                throw std::runtime_error("Invalid .tiktoken file");
            }

            std::string_view b64(line.data(), split);
            auto tokenBytesVec = base64Decode(b64);
            if (tokenBytesVec.empty()) {
                throw std::runtime_error("Invalid .tiktoken file");
            }

            uint32_t len = std::min<uint32_t>(tokenBytesVec.size(), 64u);
            out.storage.emplace_back(reinterpret_cast<const char *>(tokenBytesVec.data()), tokenBytesVec.size());
            std::string &s = out.storage.back();
            out.tokens.emplace(std::string_view(s.data(), s.size()));

            const uint8_t byte = static_cast<uint8_t>(s[0]);
            out.lenMaskByFirst[byte] |= (1ull << (len - 1));

            if (len >= 2u) {
                const uint16_t key = (uint16_t(byte) << 8) | static_cast<uint8_t>(s[1]);
                out.lenMaskByFirstTwo[key] |= (1ull << (len - 1));
            }
        }
    }

    static std::unordered_map<std::string, std::shared_ptr<CacheEntry>> &cache() {
        static std::unordered_map<std::string, std::shared_ptr<CacheEntry>> map;
        return map;
    }

    static std::mutex &cacheMutex() {
        static std::mutex mut;
        return mut;
    }

    std::shared_ptr<CacheEntry> _entry;
    std::string _model;
};
