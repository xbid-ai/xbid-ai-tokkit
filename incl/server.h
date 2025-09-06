/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string_view>
#include <utility>
#include <memory>

#include "counter.h"

#ifdef _WIN32
    #include <fcntl.h>
    #include <io.h>
#endif

class Server {
public:
    explicit Server(std::unique_ptr<Counter> c)
        : counter(std::move(c)) {
        #ifdef _WIN32
          _setmode(_fileno(stdin),  _O_BINARY);
          _setmode(_fileno(stdout), _O_BINARY);
        #endif
    }

    int run() {
        std::vector<char> buf;
        for (;;) {
            uint32_t len = 0;
            if (!read(&len, sizeof(len))) {
                break;
            }

            if (len == 0) {
                static const char nl = '\n';
                write(&nl, 1);
                continue;
            }

            buf.resize(len);
            if (!read(buf.data(), len)) {
                break;
            }

            size_t count = counter->count(std::string_view(buf.data(), buf.size()));
            char out[64];
            write(out, static_cast<size_t>(std::snprintf(out, sizeof(out), "%zu\n", count)));
            std::fflush(stdout);
        }
        return 0;
    }

private:
    std::unique_ptr<Counter> counter;

    static bool read(void* dst, size_t n) {
        auto* p = static_cast<unsigned char*>(dst);
        size_t off = 0;
        while (off < n) {
            size_t got = std::fread(p + off, 1, n - off, stdin);
            if (got == 0) {
                return false;
            }
            off += got;
        }
        return true;
    }

    static void write(const void* src, size_t n) {
        const char* p = static_cast<const char*>(src);
        size_t off = 0;
        while (off < n) {
            size_t put = std::fwrite(p + off, 1, n - off, stdout);
            off += put;
        }
    }
};