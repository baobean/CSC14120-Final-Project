/*
 * v0_describe.cpp - V0 stage 3: BRIEF descriptors.
 * Authors: LCK
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. LCK replaces the body.
 */
#include "v0.h"

void v0_describe(const ImageF& blurred, const std::vector<Keypoint>& kps,
                 std::vector<Keypoint>& kept, std::vector<Descriptor>& desc) {
    (void)blurred;
    (void)kps;
    // TODO(LCK): 256 fixed pixel pairs (seed BRIEF_SEED) in a BRIEF_PATCH x BRIEF_PATCH
    // patch, skip keypoints closer than 16 px to the border.
    kept.clear();
    desc.clear();
}
