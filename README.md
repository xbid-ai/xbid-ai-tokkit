[![Build Status](https://github.com/xbid-ai/xbid-ai-tokkit/actions/workflows/build.yml/badge.svg)](https://github.com/xbid-ai/xbid-ai-tokkit/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](/LICENSE)

# xbid-ai-tokkit

- C++ BPE counter compatible with `.tiktoken` (OpenAI) encodings. 
- Quasi-parity (no templates)
- 60% faster than OpenAI's official [tiktoken](https://www.npmjs.com/package/tiktoken) (JS/WASM)
- No external dependencies (standard C++20 toolchain)

Optional support for Google's [SentencePiece](https://github.com/google/sentencepiece) binary models (full parity, only a thin wrapper).

This library is used in the [xbid.ai](https://github.com/xbid-ai/xbid-ai) project, where we need a **low-overhead, fast, BPE counter** that is *accurate enough* for billing estimates. `xbid-ai-tokkit` uses greedy longest-match search without materializing token IDs and skips templates, trading exact parity (<1.5% error) for speed and simplicity.

We may extend the project with support for additional LLM tokenizers and token utilities.

## Accuracy Benchmarks

Evaluated on a corpus of **2,628 GPT-4o requests (16.08 MB)** collected directly from [xbid.ai](https://xbid.ai) live calls, with reference values from the OpenAI API usage counters.

**Mean size:** 1877 tokens/request

| Bias    | MAE   | MAPE (%) | Stdev error | R²    |
|--------:|------:|---------:|------------:|------:|
| -11.93  | 11.93 | 1.48     | 2.04        | 1.000 |

By design, `xbid-ai-tokkit` achieves **near-equivalence**: ~12 tokens off per request (<1.5% miss rate) with narrow, predictable error distribution.

## Speed Benchmarks

On the same dataset (2,628 GPT-4o requests, 16.08 MB) `xbid-ai-tokkit` processed data at **10.87 MB/s**, ~60% faster than OpenAI's official [tiktoken](https://www.npmjs.com/package/tiktoken) JS/WASM (6.65 MB/s).

## Build

Default (OpenAI BPE only, no external deps):
```bash
make clean
make
```

For building with `SentencePiece` support, install the library from [google/sentencepiece](https://github.com/google/sentencepiece), then:
```bash
make clean
make SENTENCEPIECE=1
```

## Usage

See `xbid-ai` project for an example client implementation of server IPC mode.

```bash
# inline
./tokkit --provider openai --model /path/o200k_base.tiktoken --text "hello"

# file
./tokkit --provider openai --model /path/o200k_base.tiktoken --file prompt.txt

# stdin
echo -n "hello" | ./tokkit --provider openai --model /path/o200k_base.tiktoken --stdin

# server mode (binary IPC)
./tokkit --provider openai --model /path/o200k_base.tiktoken --serve
# protocol: client [u32 LE length][bytes] → server "<count>\n"
```

## Disclaimer

This software is experimental and provided *as-is*, without warranties or guarantees of any kind. Use at your own risk.

## License

[MIT License](LICENSE)