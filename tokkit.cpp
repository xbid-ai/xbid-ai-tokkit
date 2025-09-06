/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <utility>
#include <stdexcept>

#include "incl/counter.h"
#include "incl/server.h"
#include "incl/default_counter.h"
#include "incl/openai_counter.h"

class CounterFactory {
public:
    static std::unique_ptr<Counter> create(const std::string& provider, const std::string& model) {
        if (provider == "openai") {
            OpenAiCounter::prepare(model);
            return std::make_unique<OpenAiCounter>(model);
        } else if (provider == "default") {
            return std::make_unique<DefaultCounter>(model);
        }
        throw std::runtime_error("Unsupported provider: " + provider);
    }
};

struct Args {
    std::string provider = "default"; // default|openai
    std::string model; // e.g. /data/o200k_base.tiktoken
    std::string text;
    std::string filePath;
    bool fromStdin { false };
    bool serve { false };
};

static bool parseArgs(int argc, char** argv, Args& args) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--provider" && i + 1 < argc) {
            args.provider = argv[++i];
        } else if (a == "--model" && i + 1 < argc) {
            args.model = argv[++i];
        } else if (a == "--text" && i + 1 < argc) {
            args.text = argv[++i];
        } else if (a == "--file" && i + 1 < argc) {
            args.filePath = argv[++i];
        } else if (a == "--stdin") {
            args.fromStdin = true;
        } else if (a == "--serve") {
            args.serve = true;
        } else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }
    return !args.model.empty() && (args.serve || !args.text.empty() || !args.filePath.empty() || args.fromStdin);
}

static std::string read(std::istream& in) {
    std::ostringstream ss; ss << in.rdbuf();
    return ss.str();
}

static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    return read(f);
}

static void usage(const char* prog) {
    std::cerr
        << "Usage:\n"
        << "  " << prog
        << " --provider (openai|default) --model /data/o200k_base.tiktoken"
        << " [--text \"...\"] [--file <path>] [--stdin]"
        << "  " << prog << " --provider (openai|default) --model /data/o200k_base.tiktoken --serve\n"
        << "    Serve protocol: client [u32 LE length][bytes]; server \"<count>\\n\".\n";
}

int main(int argc, char** argv) {
    Args args;
    if (!parseArgs(argc, argv, args)) {
        usage(argv[0]);
        return 2;
    }

    try {
        auto counter = CounterFactory::create(args.provider, args.model);

        if (args.serve) {
            Server server(std::move(counter));
            return server.run();
        }

        std::string text;
        if (!args.text.empty()) {
            text = args.text;
        } else if (!args.filePath.empty()) {
            text = readFile(args.filePath);
        } else {
            text = read(std::cin);
        }
        std::cout << counter->count(text) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
