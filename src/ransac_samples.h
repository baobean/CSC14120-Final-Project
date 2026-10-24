/*
 * ransac_samples.h - the list of random 4-match samples for RANSAC.
 * Authors: DHDH
 * External sources: none
 *
 * Header created by NDB (proposal from the Week 1 check-in). DHDH owns the implementation.
 */
#pragma once

#include <vector>
#include "common.h"

// Fills `samples` with RANSAC_ITERS * 4 match indexes (iteration it uses
// samples[4*it .. 4*it+3]), all in [0, num_matches), 4 different indexes per iteration.
// Built on the host from RANSAC_SEED. V0 and every GPU version use the same list,
// so they test the same hypotheses and find the same H.
// If num_matches < 4, `samples` is left empty.
void make_ransac_samples(int num_matches, std::vector<int>& samples);
