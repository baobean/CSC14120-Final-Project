/*
 * test_match.cpp - unit test for v0_match (stage 4): ties at the best distance,
 * the ratio test, and clearing the output.
 * Authors: NDB (test for LCK's stage)
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * Lowe ratio test: keep a match only if best < 0.8 * second_best.
 * If two descriptors of B are both at the best distance, then second_best == best,
 * so best < 0.8 * best is false and the match must be REJECTED (it is ambiguous).
 *
 * Run: make test   (g++ only, no GPU needed)
 */
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <vector>

#include "v0.h"

// Descriptor with the listed bits set to 1, all other bits 0.
static Descriptor make_desc(std::initializer_list<int> bits) {
    Descriptor d;
    memset(d.bits, 0, sizeof(d.bits));
    for (int b : bits) d.bits[b / 32] |= 1u << (b % 32);
    return d;
}

static int failures = 0;

static void check(bool ok, const char* name, const char* detail) {
    printf("[%s] %s%s%s\n", ok ? "PASS" : "FAIL", name, ok ? "" : "  -> ", ok ? "" : detail);
    if (!ok) failures++;
}

int main() {
    const Descriptor zero = make_desc({});
    std::vector<Match> m;

    // 1. Two B descriptors tie for the best distance (1 and 1): reject.
    {
        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {make_desc({0}), make_desc({1})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        check(m.empty(), "tie of the 2 nearest (dist 1, 1) is rejected",
              "got a match, expected none");
    }

    // 2. Same tie, plus a far third descriptor (dist 10): still reject.
    //    (With the bug, the far one becomes "second best", so 1 < 0.8 * 10 passes.)
    {
        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {make_desc({0}), make_desc({1}),
                                     make_desc({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        check(m.empty(), "tie (dist 1, 1) plus a far one (dist 10) is rejected",
              "got a match, expected none");
    }

    // 3. Tie found after a worse one was seen first (dist 5, 1, 1): reject.
    {
        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {make_desc({0, 1, 2, 3, 4}), make_desc({7}), make_desc({8})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        check(m.empty(), "tie found late (dist 5, 1, 1) is rejected",
              "got a match, expected none");
    }

    // 4. Exact duplicates in B (dist 0, 0): reject.
    {
        std::vector<Descriptor> A = {make_desc({3, 40, 200})};
        std::vector<Descriptor> B = {make_desc({3, 40, 200}), make_desc({3, 40, 200})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        check(m.empty(), "duplicate descriptors in B (dist 0, 0) are rejected",
              "got a match, expected none");
    }

    // 5. Clear winner (dist 1 vs 20): accept, with the right index and distance.
    //    Makes sure the fix does not break normal matches.
    {
        std::vector<int> far;
        for (int b = 100; b < 120; b++) far.push_back(b);
        Descriptor d_far = zero;
        for (int b : far) d_far.bits[b / 32] |= 1u << (b % 32);

        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {d_far, make_desc({5})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        bool ok = m.size() == 1 && m[0].idxA == 0 && m[0].idxB == 1 && m[0].dist == 1;
        check(ok, "clear winner (dist 20, 1) is kept: idxA=0 idxB=1 dist=1",
              "expected exactly 1 match {0, 1, 1}");
    }

    // 6. Second best only just too close (8 vs 10): 8 < 0.8 * 10 is false, so reject.
    {
        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {make_desc({0, 1, 2, 3, 4, 5, 6, 7}),
                                     make_desc({20, 21, 22, 23, 24, 25, 26, 27, 28, 29})};
        m.clear();  // test 7 checks that v0_match clears it itself
        v0_match(A, B, m);
        check(m.empty(), "ratio exactly 0.8 (dist 8 vs 10) is rejected",
              "got a match, expected none");
    }

    // 7. The output vector is cleared on entry (no stale entries from a previous call).
    {
        std::vector<Descriptor> A = {zero};
        std::vector<Descriptor> B = {make_desc({0}), make_desc({1})};  // gives 0 matches
        std::vector<Match> stale = {{99, 99, 0}};
        v0_match(A, B, stale);
        check(stale.empty(), "matches is cleared at the start of v0_match",
              "an old entry {99, 99, 0} is still in the vector");
    }

    printf("\n%s (%d failed)\n", failures == 0 ? "ALL PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
