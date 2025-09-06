/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#pragma once

#include <iomanip>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <cstdint>

std::vector<uint8_t> base64Decode(std::string_view input) {
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> output;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[alphabet[i]] = i;
    int val = 0, valb = -8;
    for (uint8_t c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back(uint8_t((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

void printHex(const std::vector<uint8_t>& data) {
    for (const auto& byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    std::cout << std::dec << std::endl;
}