/* Copyright (c) 2025 XBID LABS
 *
 * This file is part of XBID-AI-TOKKIT project.
 * Licensed under the MIT License.
 * Author: Fred Kyung-jin Rezeau (오경진 吳景振) <hello@kyungj.in>
 */

#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <sentencepiece_processor.h>

#include "counter.h"

// Thin adapter over the sentencepiece library.
class SentencePieceCounter final : public Counter {
public:
    explicit SentencePieceCounter(std::string model)
        : _model(std::move(model)) {}

    std::string model() const override { return _model; }

    void prepare() const override {
        std::call_once(_once, [this]() {
            _proc = std::make_unique<sentencepiece::SentencePieceProcessor>();
            auto status = _proc->Load(_model);
            if (!status.ok()) {
                throw std::runtime_error(std::string("Prepare failed: ") + status.ToString());
            }
        });
    }

    size_t count(std::string_view text) const override {
        std::vector<int> ids;
        auto status = _proc->Encode(std::string(text), &ids);
        if (!status.ok()) {
            throw std::runtime_error(std::string("Count failed: ") + status.ToString());
        }
        return ids.size();
    }

private:
    std::string _model;
    mutable std::unique_ptr<sentencepiece::SentencePieceProcessor> _proc;
    mutable std::once_flag _once;
};
