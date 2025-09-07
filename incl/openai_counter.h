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
#include <fstream>
#include <mutex>
#include <memory>
#include <array>
#include <bit>
#include <unordered_map>
#include <deque>
#include <algorithm>

#include "misc.h"
#include "bpe_counter.h"

// BPE Counter specialization for openai (tiktoken)
// Only quasi-parity, trading parity for speed (no templates).
class OpenAiCounter final : public BpeCounter {
public:
    explicit OpenAiCounter(std::string model)
        : BpeCounter(std::move(model)) {}

private:
    //  Encoding loader (not storing ranks)
    // .tiktoken format: <base64(token)><space><rank>
    void loadEncoding(const std::string &path, Entry &out) const override {
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
};
