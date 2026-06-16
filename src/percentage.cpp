// this software is distributed under the MIT License (http://www.opensource.org/licenses/MIT):
//
// Copyright (c) 2026 Calin Pop George
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "percentage.hpp"

#include "common.hpp"
#include "automata.hpp"
#include <ranges>

automata::State* automata::percentage::constructPrefixAutomaton(const std::basic_string<uint8_t> &pattern, size_t index, const Encoder &encoder, const std::vector<State*>& precomputedEnds, const std::function<State*()>& createState, State* start) {
    assert(index < pattern.size());
    uint8_t current_byte = pattern[index];
    constexpr auto addForwardTransition = [](State* start, uint8_t symbol, State* end) {
        start->addTransition(symbol, end);
        start->level = std::min(start->level, end->level + 1);
    };
    constexpr auto addForwardEndTransition = [](State* start, uint8_t symbol, uint8_t endIndex, const std::vector<State*>& precomputedEnds) {
        start->addEndTransition(symbol, endIndex, precomputedEnds);
        State* destination = start->transition(symbol);
        start->level = std::min(start->level, destination->level + 1);
    };

    // bitmap containing all the symbols prefixed by `current_byte`
    Bitmap bitmap = encoder.createPrefixBitmap(current_byte);

    // if `n_char_sym[0]` is not empty, then `match[current_idx]` is a symbol
    // if `n_char_sym[1]` is not empty, then `match[current_idx] match[current_idx + 1]` is a symbol
    // if `n_char_sym[2]` is not empty, then `match[current_idx] match[current_idx + 1] match[current_idx + 2]` is a symbol
    // in the case either optionals are not empty, they contain the index of the respective symbol
    std::array<std::optional<uint8_t>, 3> n_char_sym = bitmap.getExactPrefixMatches(pattern, index, encoder);

    std::deque<std::pair<uint8_t, State*>> transitions{};
    std::deque<std::pair<uint8_t, uint8_t>> endTransitions{};
    switch (pattern.size() - index) {
        case 1:
            {
                // if `current_byte` is escapable, that is one way to encode it
                // we are aware that if it is escapable, then, by the algorithm, it cannot also be a symbol
                if (encoder.isEscapable(current_byte)) {
                    State* transitionState = createState();
                    addForwardTransition(transitionState, current_byte, precomputedEnds[0]);
                    transitions.emplace_back(FSST_ESC, transitionState);
                }

                // check all the symbols `current_byte` is a prefix of
                for (uint8_t idx: bitmap) {
                    endTransitions.emplace_back(idx, 1);
                }
                break;
            }
        case 2:
            {
            std::vector<uint8_t> prefixes;
            uint8_t next_byte = pattern[index + 1];
            // find all the symbols prefixed by `match[current_idx] match[current_idx + 1]`
            for (uint8_t idx: bitmap) {
                const libfsst::Symbol& symbol = encoder.symbols()[idx];
                const uint8_t* symBytes = reinterpret_cast<const uint8_t*>(symbol.val.str);
                if (symbol.length() >= 2 && symBytes[1] == next_byte) {
                    prefixes.push_back(idx);
                }
            }

            // cannot encode p_{n-1} p_{n}
            // meaning impossible path => return nullptr
            if (prefixes.empty() && !n_char_sym[0].has_value() && !encoder.isEscapable(current_byte)) {
                break;
            }

            // nothing is prefixed by `current_byte next_byte`
            // this means it is not a standalone symbol, either
            if (!n_char_sym[1].has_value() && (n_char_sym[0].has_value() || encoder.isEscapable(current_byte))) {
                State* next = constructPrefixAutomaton(pattern, index + 1, encoder, precomputedEnds, createState, nullptr);
                if (next != nullptr) {
                    if (encoder.isEscapable(current_byte)) {
                        State* transitionState = createState();
                        addForwardTransition(transitionState, current_byte, next);
                        transitions.emplace_back(FSST_ESC, transitionState);
                    } else {
                        transitions.emplace_back(n_char_sym[0].value(), next);
                    }
                }
            }

            // at least something is prefixed by `current_byte next_byte`
            for (uint8_t idx: prefixes) {
                endTransitions.emplace_back(idx, 2);
            }
            break;
            }
        default: {
            bool full_match = false;
            // if there is a symbol prefixed by `current_byte next_byte nextnext_byte`
            if (n_char_sym[2].has_value()) {
                full_match = true;
                size_t max_i = std::min(static_cast<size_t>(encoder.symbols()[n_char_sym[2].value()].length()), pattern.size() - index);

                // check if it is actually a valid symbol for current matching string
                const uint8_t* symBytes = reinterpret_cast<const uint8_t*>(encoder.symbols()[n_char_sym[2].value()].val.str);
                for (size_t i = 3; i < max_i; ++i) {
                    // TODO: we can do this faster with a simple comparison.
                    if (symBytes[i] != pattern[index + i]) {
                        full_match = false;
                        break;
                    }
                }
            }

            // our longer-than-3 prefix completely is within the match
            // as such, we must encode it as such
            if (full_match) {
                if (encoder.symbols()[n_char_sym[2].value()].length() < pattern.size() - index) {
                    State* transitionState = constructPrefixAutomaton(pattern, index + encoder.symbols()[n_char_sym[2].value()].length(), encoder, precomputedEnds, createState, nullptr);
                    if (transitionState != nullptr) {
                        transitions.emplace_back(n_char_sym[2].value(), transitionState);
                    }
                    break;
                }

                uint8_t usedBytes = static_cast<uint8_t>(pattern.size() - index);
                uint8_t remainingBytes = encoder.symbols()[n_char_sym[2].value()].length() - usedBytes;
                endTransitions.emplace_back(n_char_sym[2].value(), usedBytes);
                if (remainingBytes == 0) {
                    break;
                }
            }


            // we don't match the >=3 length prefix => how do we encode, then?
            // can be done deterministically
            // `current_byte next_byte` is a symbol => we encode it as such
            if (n_char_sym[1].has_value()) {
                State* transitionState = constructPrefixAutomaton(pattern, index + 2, encoder, precomputedEnds, createState, nullptr);
                if (transitionState != nullptr) {
                    transitions.emplace_back(n_char_sym[1].value(), transitionState);
                }
            } else {
                if (n_char_sym[0].has_value() || encoder.isEscapable(current_byte)) {
                    State* transitionState = constructPrefixAutomaton(pattern, index + 1, encoder, precomputedEnds, createState, nullptr);
                    if (transitionState != nullptr) {
                        if (n_char_sym[0].has_value()) {
                            transitions.emplace_back(n_char_sym[0].value(), transitionState);
                        } else {
                            State* intermediaryState = createState();
                            addForwardTransition(intermediaryState, current_byte, transitionState);
                            transitions.emplace_back(FSST_ESC, intermediaryState);
                        }
                    }
                }
            }

            break;
        }
    }

    if (transitions.empty() && endTransitions.empty()) {
        return nullptr;
    }

    State* currentStart = start == nullptr ? createState() : start;
    for (const auto& [symbol, destination]: transitions) {
        addForwardTransition(currentStart, symbol, destination);
    }

    for (const auto& [symbol, endIndex]: endTransitions) {
        addForwardEndTransition(currentStart, symbol, endIndex, precomputedEnds);
    }
    return currentStart;
}

std::vector<automata::State*> automata::percentage::initialisePseudoEnds(FiniteAutomaton* automaton, const std::vector<State*>& precomputedEnds) {
    std::vector<State*> pseudoEnds(9, nullptr);
    for (uint8_t i = 0; i < 8; ++i) {
        pseudoEnds[i] = automaton->createState();
        pseudoEnds[i]->defaultTransition = precomputedEnds[i];
        pseudoEnds[i]->transitions[FSST_ESC] = automaton->errorState;
        pseudoEnds[i]->level = 0;
        pseudoEnds[i]->endIdx = i;
    }
    return pseudoEnds;
}

void automata::percentage::constructSuffixAutomaton(
    const std::span<const uint8_t> &pattern, const Encoder &encoder, FiniteAutomaton* automaton, const std::vector<State*>& pseudoEnds, State* startState
) {
    // Byte-level backward matcher. q[r] is the state reached when r pattern bytes (counted from
    // the RIGHT) still have to be matched; reading a code backward consumes bytes off the LEFT of
    // the unmatched region. For every symbol whose decompressed bytes line up with the pattern at
    // this position we add a backward transition; a symbol that extends before the pattern start
    // ("straddle") accepts via the pseudo-end states (which already perform the one-byte escape
    // lookahead). This accepts exactly the code sequences whose decompressed tail equals `pattern`,
    // for ANY decomposition -- not just FSST's greedy one.
    //
    // It replaces the original construction, which built a backward chain for only the greedy
    // encoding of the pattern and so missed rows whose tail is encoded with shorter symbols (e.g.
    // %cation matched as [c][a][t][ion] where greedy is [c][at][ion]). Unique Prefix still holds,
    // but a substring's tail decomposes many ways depending on the bytes that precede it.
    const size_t patLen = pattern.size();
    const uint8_t* pat = pattern.data();
    std::vector<State*> q(patLen + 1, nullptr);
    std::function<State*(size_t)> build = [&](size_t r) -> State* {
        if (q[r] != nullptr) return q[r];
        State* s = (r == patLen) ? startState : automaton->createState();
        q[r] = s;
        // level(s) = minimum number of codes that must be read (backward) from s to reach an
        // accept. It is exactly min over s's transitions of (dest->level + 1), which is a true
        // lower bound, so the parse's `remaining >= level` early-rejection never drops a match.
        uint64_t lvl = UINT64_MAX;
        const uint16_t nSym = encoder.nSymbols();
        for (uint16_t code = 0; code < nSym; ++code) {
            const libfsst::Symbol& sym = encoder.symbols()[code];
            const uint8_t symLen = sym.length();
            if (symLen == 0) continue;
            const uint8_t* symBytes = reinterpret_cast<const uint8_t*>(sym.val.str);
            if (symLen <= r) {
                // symbol lies entirely within the still-unmatched region pat[r-symLen : r]
                if (std::memcmp(symBytes, pat + (r - symLen), symLen) == 0 && !s->canTransition(static_cast<uint8_t>(code))) {
                    State* dest = (symLen == r) ? pseudoEnds[0] : build(r - symLen);
                    s->addTransition(static_cast<uint8_t>(code), dest);
                    lvl = std::min(lvl, dest->level + 1);
                }
            } else {
                // symbol straddles the pattern start: its last r bytes must equal pat[0:r]
                const uint8_t startIndex = static_cast<uint8_t>(symLen - r);
                if (startIndex < pseudoEnds.size()
                    && std::memcmp(symBytes + startIndex, pat, r) == 0 && !s->canTransition(static_cast<uint8_t>(code))) {
                    State* dest = pseudoEnds[startIndex];
                    s->addTransition(static_cast<uint8_t>(code), dest);
                    lvl = std::min(lvl, dest->level + 1);
                }
            }
        }
        // Interior escape: pat[r-1] has no single-byte symbol, so it is stored as [255, byte];
        // read backward that is `byte` then FSST_ESC. The literal byte's code equals its value and
        // can collide with a real symbol code, so if a symbol transition already exists on that
        // code we merge -- keep the symbol-interpretation transitions, but route FSST_ESC to the
        // literal continuation (a trailing 255 means the byte was a literal, not a symbol).
        if (encoder.isEscapable(pat[r - 1])) {
            State* next = (r == 1) ? pseudoEnds[0] : build(r - 1);
            const uint8_t lit = pat[r - 1];
            if (s->canTransition(lit)) {
                State* existing = s->transition(lit);
                State* merged = automaton->createState();
                merged->defaultTransition = existing->defaultTransition;
                merged->copyTransitions(existing);
                merged->transitions[FSST_ESC] = next;
                merged->level = std::min(existing->level, next->level + 1);
                s->transitions[lit] = merged;
                lvl = std::min(lvl, merged->level + 1);
            } else {
                State* mid = automaton->createState();
                mid->addTransition(FSST_ESC, next);
                mid->level = next->level + 1;
                s->addTransition(lit, mid);
                lvl = std::min(lvl, mid->level + 1);
            }
        }
        s->level = (lvl == UINT64_MAX) ? 1 : lvl;
        return s;
    };
    build(patLen);
}

automata::State* automata::percentage::constructCachedPrefixAutomaton(
    const basic_string<uint8_t>& pattern, size_t index, const Encoder &encoder, const std::vector<State*>& precomputedEnds,
    const std::function<State*()>& createState, bool enableCaching, std::vector<State*>& cache, State* errorState,
    const std::unordered_set<const State*>& currentStates, State* start
) {
    assert(index < pattern.size());
    uint8_t current_byte = pattern[index];
    if (cache[index] != nullptr) {
        if (cache[index] == errorState) {
            return nullptr;
        }
        if (enableCaching) {
            return cache[index];
        } else {
            State* currentStart = start == nullptr ? createState() : start;
            support::deepCopyAutomaton(currentStart, cache[index], currentStates, createState);
            cache[index] = currentStart;
            return currentStart;
        }
    }

    // bitmap containing all the symbols prefixed by `current_byte`
    Bitmap bitmap = encoder.createPrefixBitmap(current_byte);

    // if `n_char_sym[0]` is not empty, then `match[current_idx]` is a symbol
    // if `n_char_sym[1]` is not empty, then `match[current_idx] match[current_idx + 1]` is a symbol
    // if `n_char_sym[2]` is not empty, then `match[current_idx] match[current_idx + 1] match[current_idx + 2]` is a symbol
    // in the case either optionals are not empty, they contain the index of the respective symbol
    std::array<std::optional<uint8_t>, 3> n_char_sym = bitmap.getExactPrefixMatches(pattern, index, encoder);

    std::deque<std::pair<uint8_t, State*>> transitions{};
    std::deque<std::pair<uint8_t, uint8_t>> endTransitions{};
    switch (pattern.size() - index) {
        case 1:
            {
                // if `current_byte` is escapable, that is one way to encode it
                // we are aware that if it is escapable, then, by the algorithm, it cannot also be a symbol
                if (encoder.isEscapable(current_byte)) {
                    State* transitionState = createState();
                    transitionState->addTransition(current_byte, precomputedEnds[0]);
                    transitions.emplace_back(FSST_ESC, transitionState);
                }

                // check all the symbols `current_byte` is a prefix of
                for (uint8_t idx: bitmap) {
                    endTransitions.emplace_back(idx, 1);
                }
                break;
            }
        case 2:
            {
            std::vector<uint8_t> prefixes;
            uint8_t next_byte = pattern[index + 1];
            // find all the symbols prefixed by `match[current_idx] match[current_idx + 1]`
            for (uint8_t idx: bitmap) {
                const libfsst::Symbol& symbol = encoder.symbols()[idx];
                const uint8_t* symBytes = reinterpret_cast<const uint8_t*>(symbol.val.str);
                if (symbol.length() >= 2 && symBytes[1] == next_byte) {
                    prefixes.push_back(idx);
                }
            }

            // cannot encode p_{n-1} p_{n}
            // meaning impossible path => return nullptr
            if (prefixes.empty() && !n_char_sym[0].has_value() && !encoder.isEscapable(current_byte)) {
                break;
            }

            // `current_byte` as a single byte (then continue), IN ADDITION to the 2-byte
            // `prefixes` below. A needle byte-run can be encoded as the 2-byte symbol OR as the
            // single byte depending on the row's continuation; upstream gated this on
            // `!n_char_sym[1]`, dropping the single-byte encoding whenever a 2-byte symbol existed.
            if (n_char_sym[0].has_value() || encoder.isEscapable(current_byte)) {
                State* next = constructCachedPrefixAutomaton(
                    pattern, index + 1, encoder, precomputedEnds, createState, enableCaching, cache, errorState, currentStates, nullptr
                );
                if (next != nullptr) {
                    if (encoder.isEscapable(current_byte)) {
                        State* transitionState = createState();
                        transitionState->addTransition(current_byte, next);
                        transitions.emplace_back(FSST_ESC, transitionState);
                    } else {
                        transitions.emplace_back(n_char_sym[0].value(), next);
                    }
                }
            }

            // at least something is prefixed by `current_byte next_byte`
            for (uint8_t idx: prefixes) {
                endTransitions.emplace_back(idx, 2);
                // create a new branch and add that prefix onto that branch
            }

            // we want to deterministically encode `current_byte next_byte`
            // if `current_byte next_byte` is a symbol, we have already done that
            break;
            }
        default: {
            bool full_match = false;
            // if there is a symbol prefixed by `current_byte next_byte nextnext_byte`
            if (n_char_sym[2].has_value()) {
                full_match = true;
                size_t max_i = std::min(static_cast<size_t>(encoder.symbols()[n_char_sym[2].value()].length()), pattern.size() - index);

                // check if it is actually a valid symbol for current matching string
                const uint8_t* symBytes = reinterpret_cast<const uint8_t*>(encoder.symbols()[n_char_sym[2].value()].val.str);
                for (size_t i = 3; i < max_i; ++i) {
                    // TODO: we can do this faster with a simple comparison.
                    if (symBytes[i] != pattern[index + i]) {
                        full_match = false;
                        break;
                    }
                }
            }

            // The >=3-byte symbol matching here is unique (FSST's unique-prefix property); keep
            // upstream's single-symbol handling but do NOT `break` -- the same bytes are encoded
            // with shorter symbols in other rows (continuation-dependent greedy choice), so emit
            // this transition AND fall through to the shorter decompositions below.
            if (full_match) {
                if (encoder.symbols()[n_char_sym[2].value()].length() < pattern.size() - index) {
                    State* transitionState = constructCachedPrefixAutomaton(
                        pattern, index + encoder.symbols()[n_char_sym[2].value()].length(), encoder, precomputedEnds, createState, enableCaching, cache, errorState, currentStates, nullptr
                    );
                    if (transitionState != nullptr) {
                        transitions.emplace_back(n_char_sym[2].value(), transitionState);
                    }
                } else {
                    uint8_t usedBytes = static_cast<uint8_t>(pattern.size() - index);
                    endTransitions.emplace_back(n_char_sym[2].value(), usedBytes);
                }
            }

            // Shorter decompositions, emitted IN ADDITION to the >=3-byte symbol above and to
            // each other (upstream chose exactly one via if/else, then `break`ed).
            if (n_char_sym[1].has_value()) {
                State* transitionState = constructCachedPrefixAutomaton(
                    pattern, index + 2, encoder, precomputedEnds, createState, enableCaching, cache, errorState, currentStates, nullptr
                );
                if (transitionState != nullptr) {
                    transitions.emplace_back(n_char_sym[1].value(), transitionState);
                }
            }
            if (n_char_sym[0].has_value() || encoder.isEscapable(current_byte)) {
                State* transitionState = constructCachedPrefixAutomaton(
                    pattern, index + 1, encoder, precomputedEnds, createState, enableCaching, cache, errorState, currentStates, nullptr
                );
                if (transitionState != nullptr) {
                    if (n_char_sym[0].has_value()) {
                        transitions.emplace_back(n_char_sym[0].value(), transitionState);
                    } else {
                        State* intermediaryState = createState();
                        intermediaryState->addTransition(current_byte, transitionState);
                        transitions.emplace_back(FSST_ESC, intermediaryState);
                    }
                }
            }

            break;
        }
    }

    if (transitions.empty() && endTransitions.empty()) {
        cache[index] = errorState;
        return nullptr;
    }
    State* currentStart = start == nullptr ? createState() : start;
    for (const auto& [symbol, destination]: transitions) {
        currentStart->addTransition(symbol, destination);
    }

    for (const auto& [symbol, endIndex]: endTransitions) {
        currentStart->addEndTransition(symbol, endIndex, precomputedEnds);
    }

    cache[index] = currentStart;
    return currentStart;
}

void automata::percentage::integrateSymbolsContainingPattern(
    const basic_string<uint8_t>& pattern, const Encoder &encoder, const std::vector<State*>& precomputedEnds, std::vector<State*>& startStates
) {
    if (pattern.size() > 7) {
        return;
    }
    char in_symbol[8] = {0};
    memcpy(in_symbol, pattern.data(), pattern.size());
    for (uint16_t symbolIdx = 0; symbolIdx < encoder.nSymbols(); ++symbolIdx) {
        const libfsst::Symbol& symbol = encoder.symbols()[symbolIdx];
        if (symbol.length() < pattern.size() + 1) {
            continue;
        }
        for (uint8_t start = 1; start <= symbol.length() - pattern.size(); ++start) {
            // check if symbol startings from `start` up to `match.size()` is equal to what we want
            uint64_t shifted = symbol.val.num >> (8 * start);
            uint64_t mask = (static_cast<uint64_t>(1) << (8 * pattern.size())) - 1;
            if ((shifted & mask) == *reinterpret_cast<uint64_t*>(in_symbol)) {
                uint8_t usedBytes = start + pattern.size();
                startStates[start]->addEndTransition(symbolIdx, usedBytes, precomputedEnds);
            }
        }
    }
}

void automata::percentage::connectStartsToSubautomaton(
    const basic_string<uint8_t>& pattern, const Encoder &encoder, const std::vector<State*>& precomputedEnds,
    const std::function<State*()>& createState, State* errorState, std::vector<State*>& startStates, const std::unordered_set<const State*>& currentStates,
    std::array<State*, 255>& transitionStates, std::vector<State*>& cache, uint8_t i, const std::vector<uint8_t>& symbols) {
    bool disableCaching = std::any_of(symbols.begin(), symbols.end(), [&](uint8_t symbol) {
        return transitionStates[symbol] != nullptr;
    });

    State* subautomatonRoot = constructCachedPrefixAutomaton(
        pattern, i + 1, encoder, precomputedEnds, createState, !disableCaching, cache, errorState, currentStates, nullptr
    );
    if (subautomatonRoot == nullptr) {
        return;
    }
    for (uint8_t symbol: symbols) {
        uint8_t startIndex = encoder.symbols()[symbol].length() - 1 - i;
        startStates[startIndex]->addTransition(symbol, subautomatonRoot);
        transitionStates[symbol] = subautomatonRoot;
    }
}

void automata::percentage::linkStarts(std::vector<State*>& startStates) {
    std::unordered_map<uint8_t, State*> symbolToState{};
    for (State* start: std::views::reverse(startStates)) {
        if (start == nullptr) {
            continue;
        }
        for (const auto& [symbol, state]: symbolToState) {
            if (!start->canTransition(symbol)) {
                start->addTransition(symbol, state);
            }
        }

        for (const auto& [symbol, state]: start->transitions) {
            symbolToState[symbol] = state;
        }
    }
}

void automata::percentage::splitStarts(std::vector<State*>& startStates, const std::unordered_set<const State*>& currentStates) {
    std::array<State*, 255> transitionStates{nullptr};
    std::unordered_map<State*, State*> postStartToTransitionState{};
    std::list<State*> orderedPostStarts{};
    for (uint8_t idx = 8; idx >= 1; --idx) {
        State* start = startStates[idx];

        for (auto& [symbol, next]: start->transitions) {
            if (transitionStates[symbol] != nullptr) {
                // mathematically, there is at most one such state
                if (!postStartToTransitionState.contains(next)) {
                    orderedPostStarts.push_back(next);
                    postStartToTransitionState[next] = transitionStates[symbol];
                }
            }
        }

        for (auto& [symbol, next]: start->transitions) {
            transitionStates[symbol] = next;
        }
    }
    for (State* postStart: orderedPostStarts) {
        support::shallowCopyAutomaton(postStart, postStartToTransitionState[postStart], currentStates);
    }

    for (const auto& [symbol, destination]: startStates[0]->transitions) {
        if (symbol == 255 || transitionStates[symbol] == nullptr) {
            continue;
        }
        // if (endMappings[destination] == transitionStates[symbol]) {
        //     continue;
        // }
        support::shallowCopyAutomaton(destination, transitionStates[symbol], currentStates);
    }
}


void automata::percentage::constructFallbackTransitions(std::vector<State*>& startStates, const std::unordered_set<const State*>& currentStates) {
    std::queue<std::tuple<State*, const State*>> queue{};
    std::unordered_map<const State*, const State*> trailingMappings{};
    std::deque<State*> topoSorted{};

    for (State* start: startStates) {
        if (start == nullptr) {
            continue;
        }

        for (auto& [symbol, postStart]: start->transitions) {
            State* trailingState = symbol == FSST_ESC ? nullptr : startStates[0];
            if (!trailingMappings.contains(postStart) && currentStates.contains(postStart)) {
                trailingMappings[postStart] = trailingState;
                queue.emplace(postStart, trailingState);
            }
        }
    }

    while (!queue.empty()) {
        auto [dest, src] = queue.front();
        topoSorted.push_back(dest);
        queue.pop();

        for (auto& [symbol, next]: dest->transitions) {
            if (!trailingMappings.contains(next) && currentStates.contains(next)) {
                const State* fallback = support::transitionTrailingState(src, symbol, trailingMappings, startStates[0]);
                trailingMappings[next] = fallback;
                queue.emplace(next, fallback);
            }
        }
    }
    support::copyFallbackTransitions(trailingMappings, topoSorted);
}
