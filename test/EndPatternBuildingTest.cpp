//
// Created by pop on 6/25/25.
//
#include "gtest/gtest.h"
#include "env.hpp"
#include "automata.hpp"
#include "pattern.hpp"

namespace test {
    std::vector<const automata::State*> findPseudoEnds(const automata::SingleStartFiniteAutomaton& automaton) {
        std::vector<const automata::State*> pseudoEnds(9, nullptr);
        for (const automata::State& node: automaton.states) {
            if (node.endIdx.has_value()) {
                pseudoEnds[node.endIdx.value()] = &node;
            }
        }
        return pseudoEnds;
    }

    // TestBuild{First..Sixth}Pattern below are white-box structural fixtures for the suffix
    // (`%pattern`) automaton. The suffix construction is now a byte-level backward matcher that
    // accepts every FSST decomposition (not just the greedy one), so these were regenerated from
    // the constructed automaton: each `sN` is a state reached by walking `transition()` from
    // `actualStartState`, and the asserts pin its level, defaultTransition, fan-out, and edges
    // (to other states or `pseudoEnds[i]`). On a future construction change, regenerate rather
    // than hand-edit. TestBuildSeventhPattern is shape-stable across the change and is unchanged.
    TEST(EndPatternBuildTest, TestBuildFirstPattern) {
        StringPattern sp{patterns[0]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({238}));
        ASSERT_EQ(automaton.startState->level, 3);
        ASSERT_EQ(automaton.actualStartState, automaton.startState->transition(238));
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 2);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 3);
        ASSERT_TRUE(s0->canTransition(129));
        automata::State* s1 = s0->transition(129);
        ASSERT_TRUE(s0->canTransition(211));
        automata::State* s2 = s0->transition(211);
        ASSERT_TRUE(s0->canTransition(225));
        automata::State* s3 = s0->transition(225);
        ASSERT_EQ(s1->level, 2);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 1);
        ASSERT_TRUE(s1->canTransition(241));
        ASSERT_EQ(s1->transition(241), s2);
        ASSERT_EQ(s2->level, 1);
        ASSERT_EQ(s2->defaultTransition, defaultState);
        ASSERT_EQ(s2->transitions.size(), 2);
        ASSERT_TRUE(s2->canTransition(73));
        ASSERT_EQ(s2->transition(73), pseudoEnds[0]);
        ASSERT_TRUE(s2->canTransition(236));
        automata::State* s4 = s2->transition(236);
        ASSERT_EQ(s3->level, 2);
        ASSERT_EQ(s3->defaultTransition, defaultState);
        ASSERT_EQ(s3->transitions.size(), 2);
        ASSERT_TRUE(s3->canTransition(86));
        ASSERT_EQ(s3->transition(86), s2);
        ASSERT_TRUE(s3->canTransition(223));
        ASSERT_EQ(s3->transition(223), s1);
        ASSERT_EQ(s4->level, 1);
        ASSERT_EQ(s4->defaultTransition, defaultState);
        ASSERT_EQ(s4->transitions.size(), 4);
        ASSERT_TRUE(s4->canTransition(114));
        ASSERT_EQ(s4->transition(114), pseudoEnds[1]);
        ASSERT_TRUE(s4->canTransition(215));
        ASSERT_EQ(s4->transition(215), pseudoEnds[2]);
        ASSERT_TRUE(s4->canTransition(219));
        ASSERT_EQ(s4->transition(219), pseudoEnds[4]);
        ASSERT_TRUE(s4->canTransition(241));
        ASSERT_EQ(s4->transition(241), pseudoEnds[0]);
    }

    TEST(EndPatternBuildTest, TestBuildSecondPattern) {
        StringPattern sp{patterns[1]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({}));
        ASSERT_EQ(automaton.startState->level, 3);
        ASSERT_EQ(automaton.actualStartState, automaton.startState.get());
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 3);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 3);
        ASSERT_TRUE(s0->canTransition(129));
        automata::State* s1 = s0->transition(129);
        ASSERT_TRUE(s0->canTransition(170));
        automata::State* s2 = s0->transition(170);
        ASSERT_TRUE(s0->canTransition(225));
        automata::State* s3 = s0->transition(225);
        ASSERT_EQ(s1->level, 3);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 2);
        ASSERT_TRUE(s1->canTransition(9));
        automata::State* s4 = s1->transition(9);
        ASSERT_TRUE(s1->canTransition(226));
        ASSERT_EQ(s1->transition(226), s2);
        ASSERT_EQ(s2->level, 2);
        ASSERT_EQ(s2->defaultTransition, defaultState);
        ASSERT_EQ(s2->transitions.size(), 3);
        ASSERT_TRUE(s2->canTransition(132));
        automata::State* s5 = s2->transition(132);
        ASSERT_TRUE(s2->canTransition(152));
        automata::State* s6 = s2->transition(152);
        ASSERT_TRUE(s2->canTransition(223));
        ASSERT_EQ(s2->transition(223), s4);
        ASSERT_EQ(s3->level, 3);
        ASSERT_EQ(s3->defaultTransition, defaultState);
        ASSERT_EQ(s3->transitions.size(), 2);
        ASSERT_TRUE(s3->canTransition(130));
        ASSERT_EQ(s3->transition(130), s2);
        ASSERT_TRUE(s3->canTransition(223));
        ASSERT_EQ(s3->transition(223), s1);
        ASSERT_EQ(s4->level, 2);
        ASSERT_EQ(s4->defaultTransition, defaultState);
        ASSERT_EQ(s4->transitions.size(), 2);
        ASSERT_TRUE(s4->canTransition(129));
        ASSERT_EQ(s4->transition(129), s6);
        ASSERT_TRUE(s4->canTransition(225));
        ASSERT_EQ(s4->transition(225), s5);
        ASSERT_EQ(s5->level, 2);
        ASSERT_EQ(s5->defaultTransition, defaultState);
        ASSERT_EQ(s5->transitions.size(), 1);
        ASSERT_TRUE(s5->canTransition(223));
        ASSERT_EQ(s5->transition(223), s6);
        ASSERT_EQ(s6->level, 1);
        ASSERT_EQ(s6->defaultTransition, defaultState);
        ASSERT_EQ(s6->transitions.size(), 1);
        ASSERT_TRUE(s6->canTransition(253));
        ASSERT_EQ(s6->transition(253), pseudoEnds[0]);
    }

    TEST(EndPatternBuildTest, TestBuildThirdPattern) {
        StringPattern sp{patterns[2]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({}));
        ASSERT_EQ(automaton.startState->level, 1);
        ASSERT_EQ(automaton.actualStartState, automaton.startState.get());
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 1);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 13);
        ASSERT_TRUE(s0->canTransition(129));
        ASSERT_EQ(s0->transition(129), pseudoEnds[0]);
        ASSERT_TRUE(s0->canTransition(136));
        ASSERT_EQ(s0->transition(136), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(139));
        ASSERT_EQ(s0->transition(139), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(146));
        ASSERT_EQ(s0->transition(146), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(149));
        ASSERT_EQ(s0->transition(149), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(158));
        ASSERT_EQ(s0->transition(158), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(165));
        ASSERT_EQ(s0->transition(165), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(169));
        ASSERT_EQ(s0->transition(169), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(170));
        ASSERT_EQ(s0->transition(170), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(183));
        ASSERT_EQ(s0->transition(183), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(193));
        ASSERT_EQ(s0->transition(193), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(211));
        ASSERT_EQ(s0->transition(211), pseudoEnds[1]);
        ASSERT_TRUE(s0->canTransition(225));
        automata::State* s1 = s0->transition(225);
        ASSERT_EQ(s1->level, 1);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 34);
        ASSERT_TRUE(s1->canTransition(1));
        ASSERT_EQ(s1->transition(1), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(5));
        ASSERT_EQ(s1->transition(5), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(8));
        ASSERT_EQ(s1->transition(8), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(17));
        ASSERT_EQ(s1->transition(17), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(24));
        ASSERT_EQ(s1->transition(24), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(34));
        ASSERT_EQ(s1->transition(34), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(49));
        ASSERT_EQ(s1->transition(49), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(86));
        ASSERT_EQ(s1->transition(86), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(108));
        ASSERT_EQ(s1->transition(108), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(113));
        ASSERT_EQ(s1->transition(113), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(118));
        ASSERT_EQ(s1->transition(118), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(122));
        ASSERT_EQ(s1->transition(122), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(124));
        ASSERT_EQ(s1->transition(124), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(125));
        ASSERT_EQ(s1->transition(125), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(126));
        ASSERT_EQ(s1->transition(126), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(127));
        ASSERT_EQ(s1->transition(127), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(128));
        ASSERT_EQ(s1->transition(128), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(130));
        ASSERT_EQ(s1->transition(130), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(131));
        ASSERT_EQ(s1->transition(131), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(132));
        ASSERT_EQ(s1->transition(132), pseudoEnds[1]);
        ASSERT_TRUE(s1->canTransition(134));
        ASSERT_EQ(s1->transition(134), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(135));
        ASSERT_EQ(s1->transition(135), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(140));
        ASSERT_EQ(s1->transition(140), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(152));
        ASSERT_EQ(s1->transition(152), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(156));
        ASSERT_EQ(s1->transition(156), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(161));
        ASSERT_EQ(s1->transition(161), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(167));
        ASSERT_EQ(s1->transition(167), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(174));
        ASSERT_EQ(s1->transition(174), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(175));
        ASSERT_EQ(s1->transition(175), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(190));
        ASSERT_EQ(s1->transition(190), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(197));
        ASSERT_EQ(s1->transition(197), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(209));
        ASSERT_EQ(s1->transition(209), pseudoEnds[2]);
        ASSERT_TRUE(s1->canTransition(216));
        ASSERT_EQ(s1->transition(216), pseudoEnds[3]);
        ASSERT_TRUE(s1->canTransition(223));
        ASSERT_EQ(s1->transition(223), pseudoEnds[0]);
    }

    TEST(EndPatternBuildTest, TestBuildFourthPattern) {
        StringPattern sp{patterns[3]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({}));
        ASSERT_EQ(automaton.startState->level, 3);
        ASSERT_EQ(automaton.actualStartState, automaton.startState.get());
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 3);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 3);
        ASSERT_TRUE(s0->canTransition(74));
        automata::State* s1 = s0->transition(74);
        ASSERT_TRUE(s0->canTransition(178));
        automata::State* s2 = s0->transition(178);
        ASSERT_TRUE(s0->canTransition(226));
        automata::State* s3 = s0->transition(226);
        ASSERT_EQ(s1->level, 3);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 1);
        ASSERT_TRUE(s1->canTransition(227));
        ASSERT_EQ(s1->transition(227), s2);
        ASSERT_EQ(s2->level, 2);
        ASSERT_EQ(s2->defaultTransition, defaultState);
        ASSERT_EQ(s2->transitions.size(), 3);
        ASSERT_TRUE(s2->canTransition(14));
        automata::State* s4 = s2->transition(14);
        ASSERT_TRUE(s2->canTransition(205));
        automata::State* s5 = s2->transition(205);
        ASSERT_TRUE(s2->canTransition(229));
        automata::State* s6 = s2->transition(229);
        ASSERT_EQ(s3->level, 3);
        ASSERT_EQ(s3->defaultTransition, defaultState);
        ASSERT_EQ(s3->transitions.size(), 2);
        ASSERT_TRUE(s3->canTransition(112));
        ASSERT_EQ(s3->transition(112), s2);
        ASSERT_TRUE(s3->canTransition(230));
        ASSERT_EQ(s3->transition(230), s1);
        ASSERT_EQ(s4->level, 2);
        ASSERT_EQ(s4->defaultTransition, defaultState);
        ASSERT_EQ(s4->transitions.size(), 2);
        ASSERT_TRUE(s4->canTransition(42));
        automata::State* s7 = s4->transition(42);
        ASSERT_TRUE(s4->canTransition(224));
        ASSERT_EQ(s4->transition(224), s5);
        ASSERT_EQ(s5->level, 1);
        ASSERT_EQ(s5->defaultTransition, defaultState);
        ASSERT_EQ(s5->transitions.size(), 3);
        ASSERT_TRUE(s5->canTransition(107));
        ASSERT_EQ(s5->transition(107), pseudoEnds[0]);
        ASSERT_TRUE(s5->canTransition(191));
        ASSERT_EQ(s5->transition(191), pseudoEnds[1]);
        ASSERT_TRUE(s5->canTransition(229));
        ASSERT_EQ(s5->transition(229), s7);
        ASSERT_EQ(s6->level, 2);
        ASSERT_EQ(s6->defaultTransition, defaultState);
        ASSERT_EQ(s6->transitions.size(), 2);
        ASSERT_TRUE(s6->canTransition(82));
        ASSERT_EQ(s6->transition(82), s5);
        ASSERT_TRUE(s6->canTransition(231));
        ASSERT_EQ(s6->transition(231), s4);
        ASSERT_EQ(s7->level, 1);
        ASSERT_EQ(s7->defaultTransition, defaultState);
        ASSERT_EQ(s7->transitions.size(), 8);
        ASSERT_TRUE(s7->canTransition(53));
        ASSERT_EQ(s7->transition(53), pseudoEnds[1]);
        ASSERT_TRUE(s7->canTransition(69));
        ASSERT_EQ(s7->transition(69), pseudoEnds[1]);
        ASSERT_TRUE(s7->canTransition(72));
        ASSERT_EQ(s7->transition(72), pseudoEnds[1]);
        ASSERT_TRUE(s7->canTransition(75));
        ASSERT_EQ(s7->transition(75), pseudoEnds[1]);
        ASSERT_TRUE(s7->canTransition(95));
        ASSERT_EQ(s7->transition(95), pseudoEnds[1]);
        ASSERT_TRUE(s7->canTransition(168));
        ASSERT_EQ(s7->transition(168), pseudoEnds[2]);
        ASSERT_TRUE(s7->canTransition(180));
        ASSERT_EQ(s7->transition(180), pseudoEnds[2]);
        ASSERT_TRUE(s7->canTransition(232));
        ASSERT_EQ(s7->transition(232), pseudoEnds[0]);
    }

    TEST(EndPatternBuildTest, TestBuildFifthPattern) {
        StringPattern sp{patterns[4]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({}));
        ASSERT_EQ(automaton.startState->level, 3);
        ASSERT_EQ(automaton.actualStartState, automaton.startState.get());
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 3);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 3);
        ASSERT_TRUE(s0->canTransition(106));
        automata::State* s1 = s0->transition(106);
        ASSERT_TRUE(s0->canTransition(137));
        automata::State* s2 = s0->transition(137);
        ASSERT_TRUE(s0->canTransition(229));
        automata::State* s3 = s0->transition(229);
        ASSERT_EQ(s1->level, 3);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 1);
        ASSERT_TRUE(s1->canTransition(238));
        ASSERT_EQ(s1->transition(238), s2);
        ASSERT_EQ(s2->level, 2);
        ASSERT_EQ(s2->defaultTransition, defaultState);
        ASSERT_EQ(s2->transitions.size(), 1);
        ASSERT_TRUE(s2->canTransition(239));
        automata::State* s4 = s2->transition(239);
        ASSERT_EQ(s3->level, 3);
        ASSERT_EQ(s3->defaultTransition, defaultState);
        ASSERT_EQ(s3->transitions.size(), 2);
        ASSERT_TRUE(s3->canTransition(84));
        ASSERT_EQ(s3->transition(84), s2);
        ASSERT_TRUE(s3->canTransition(233));
        ASSERT_EQ(s3->transition(233), s1);
        ASSERT_EQ(s4->level, 1);
        ASSERT_EQ(s4->defaultTransition, defaultState);
        ASSERT_EQ(s4->transitions.size(), 2);
        ASSERT_TRUE(s4->canTransition(117));
        ASSERT_EQ(s4->transition(117), pseudoEnds[0]);
        ASSERT_TRUE(s4->canTransition(229));
        automata::State* s5 = s4->transition(229);
        ASSERT_EQ(s5->level, 1);
        ASSERT_EQ(s5->defaultTransition, defaultState);
        ASSERT_EQ(s5->transitions.size(), 4);
        ASSERT_TRUE(s5->canTransition(0));
        ASSERT_EQ(s5->transition(0), pseudoEnds[1]);
        ASSERT_TRUE(s5->canTransition(94));
        ASSERT_EQ(s5->transition(94), pseudoEnds[1]);
        ASSERT_TRUE(s5->canTransition(164));
        ASSERT_EQ(s5->transition(164), pseudoEnds[2]);
        ASSERT_TRUE(s5->canTransition(227));
        ASSERT_EQ(s5->transition(227), pseudoEnds[0]);
    }

    TEST(EndPatternBuildTest, TestBuildSixthPattern) {
        StringPattern sp{patterns[5]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);
        automata::State* defaultState = &errorState;

        ASSERT_EQ(automaton.deterministicPath, std::basic_string<uint8_t>({}));
        ASSERT_EQ(automaton.startState->level, 4);
        ASSERT_EQ(automaton.actualStartState, automaton.startState.get());
        automata::State* s0 = automaton.actualStartState;
        ASSERT_EQ(s0->level, 4);
        ASSERT_EQ(s0->defaultTransition, defaultState);
        ASSERT_EQ(s0->transitions.size(), 2);
        ASSERT_TRUE(s0->canTransition(105));
        automata::State* s1 = s0->transition(105);
        ASSERT_TRUE(s0->canTransition(233));
        automata::State* s2 = s0->transition(233);
        ASSERT_EQ(s1->level, 4);
        ASSERT_EQ(s1->defaultTransition, defaultState);
        ASSERT_EQ(s1->transitions.size(), 2);
        ASSERT_TRUE(s1->canTransition(116));
        automata::State* s3 = s1->transition(116);
        ASSERT_TRUE(s1->canTransition(230));
        automata::State* s4 = s1->transition(230);
        ASSERT_EQ(s2->level, 3);
        ASSERT_EQ(s2->defaultTransition, defaultState);
        ASSERT_EQ(s2->transitions.size(), 4);
        ASSERT_TRUE(s2->canTransition(12));
        ASSERT_EQ(s2->transition(12), s4);
        ASSERT_TRUE(s2->canTransition(143));
        ASSERT_EQ(s2->transition(143), s3);
        ASSERT_TRUE(s2->canTransition(220));
        automata::State* s5 = s2->transition(220);
        ASSERT_TRUE(s2->canTransition(224));
        ASSERT_EQ(s2->transition(224), s1);
        ASSERT_EQ(s3->level, 3);
        ASSERT_EQ(s3->defaultTransition, defaultState);
        ASSERT_EQ(s3->transitions.size(), 2);
        ASSERT_TRUE(s3->canTransition(84));
        ASSERT_EQ(s3->transition(84), s5);
        ASSERT_TRUE(s3->canTransition(233));
        automata::State* s6 = s3->transition(233);
        ASSERT_EQ(s4->level, 4);
        ASSERT_EQ(s4->defaultTransition, defaultState);
        ASSERT_EQ(s4->transitions.size(), 1);
        ASSERT_TRUE(s4->canTransition(226));
        ASSERT_EQ(s4->transition(226), s3);
        ASSERT_EQ(s5->level, 2);
        ASSERT_EQ(s5->defaultTransition, defaultState);
        ASSERT_EQ(s5->transitions.size(), 3);
        ASSERT_TRUE(s5->canTransition(106));
        automata::State* s7 = s5->transition(106);
        ASSERT_TRUE(s5->canTransition(151));
        automata::State* s8 = s5->transition(151);
        ASSERT_TRUE(s5->canTransition(229));
        automata::State* s9 = s5->transition(229);
        ASSERT_EQ(s6->level, 3);
        ASSERT_EQ(s6->defaultTransition, defaultState);
        ASSERT_EQ(s6->transitions.size(), 1);
        ASSERT_TRUE(s6->canTransition(238));
        ASSERT_EQ(s6->transition(238), s5);
        ASSERT_EQ(s7->level, 1);
        ASSERT_EQ(s7->defaultTransition, defaultState);
        ASSERT_EQ(s7->transitions.size(), 2);
        ASSERT_TRUE(s7->canTransition(46));
        ASSERT_EQ(s7->transition(46), pseudoEnds[0]);
        ASSERT_TRUE(s7->canTransition(224));
        ASSERT_EQ(s7->transition(224), s8);
        ASSERT_EQ(s8->level, 1);
        ASSERT_EQ(s8->defaultTransition, defaultState);
        ASSERT_EQ(s8->transitions.size(), 12);
        ASSERT_TRUE(s8->canTransition(12));
        ASSERT_EQ(s8->transition(12), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(31));
        ASSERT_EQ(s8->transition(31), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(42));
        ASSERT_EQ(s8->transition(42), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(45));
        ASSERT_EQ(s8->transition(45), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(46));
        ASSERT_EQ(s8->transition(46), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(66));
        ASSERT_EQ(s8->transition(66), pseudoEnds[1]);
        ASSERT_TRUE(s8->canTransition(143));
        ASSERT_EQ(s8->transition(143), pseudoEnds[2]);
        ASSERT_TRUE(s8->canTransition(172));
        ASSERT_EQ(s8->transition(172), pseudoEnds[2]);
        ASSERT_TRUE(s8->canTransition(179));
        ASSERT_EQ(s8->transition(179), pseudoEnds[2]);
        ASSERT_TRUE(s8->canTransition(196));
        ASSERT_EQ(s8->transition(196), pseudoEnds[2]);
        ASSERT_TRUE(s8->canTransition(220));
        ASSERT_EQ(s8->transition(220), pseudoEnds[4]);
        ASSERT_TRUE(s8->canTransition(224));
        ASSERT_EQ(s8->transition(224), pseudoEnds[0]);
        ASSERT_EQ(s9->level, 2);
        ASSERT_EQ(s9->defaultTransition, defaultState);
        ASSERT_EQ(s9->transitions.size(), 2);
        ASSERT_TRUE(s9->canTransition(105));
        ASSERT_EQ(s9->transition(105), s8);
        ASSERT_TRUE(s9->canTransition(233));
        ASSERT_EQ(s9->transition(233), s7);
    }

    TEST(EndPatternBuildTest, TestBuildSeventhPattern) {
        StringPattern sp{patterns[6]};
        automata::SingleStartFiniteAutomaton automaton = sp.createEndAutomaton(encoder, precomputedEnds, &errorState);
        std::vector<const automata::State*> pseudoEnds = findPseudoEnds(automaton);

        automata::State* defaultState = &errorState;
        automata::State* currentState = automaton.startState.get();
        ASSERT_EQ(currentState->level, 1);
        ASSERT_EQ(currentState->defaultTransition, defaultState);
        ASSERT_EQ(currentState->transitions.size(), 21);

        ASSERT_TRUE(currentState->canTransition(9));
        ASSERT_EQ(currentState->transition(9), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(16));
        ASSERT_EQ(currentState->transition(16), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(26));
        ASSERT_EQ(currentState->transition(26), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(28));
        ASSERT_EQ(currentState->transition(28), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(29));
        ASSERT_EQ(currentState->transition(29), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(52));
        ASSERT_EQ(currentState->transition(52), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(57));
        ASSERT_EQ(currentState->transition(57), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(74));
        ASSERT_EQ(currentState->transition(74), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(88));
        ASSERT_EQ(currentState->transition(88), pseudoEnds[1]);

        ASSERT_TRUE(currentState->canTransition(133));
        ASSERT_EQ(currentState->transition(133), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(145));
        ASSERT_EQ(currentState->transition(145), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(150));
        ASSERT_EQ(currentState->transition(150), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(153));
        ASSERT_EQ(currentState->transition(153), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(155));
        ASSERT_EQ(currentState->transition(155), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(166));
        ASSERT_EQ(currentState->transition(166), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(171));
        ASSERT_EQ(currentState->transition(171), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(178));
        ASSERT_EQ(currentState->transition(178), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(188));
        ASSERT_EQ(currentState->transition(188), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(213));
        ASSERT_EQ(currentState->transition(213), pseudoEnds[2]);

        ASSERT_TRUE(currentState->canTransition(218));
        ASSERT_EQ(currentState->transition(218), pseudoEnds[4]);

        ASSERT_TRUE(currentState->canTransition(226));
        ASSERT_EQ(currentState->transition(226), pseudoEnds[0]);
    }
}