/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_TARGETING_AD_TARGETING_SEGMENT_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_TARGETING_AD_TARGETING_SEGMENT_INFO_H_

#include <string>
#include <vector>

namespace ads {
// TODO(tmancey): Should this not be namespaced for ad_targeting?

using SegmentList = std::vector<std::string>;

struct SegmentsInfo {
  // TODO(tmancey): Rename data structure
  SegmentsInfo();
  SegmentsInfo(const SegmentsInfo& info);
  ~SegmentsInfo();

  SegmentList text_classification_segments;
  SegmentList epsilon_greedy_bandit_segments;
  SegmentList purchase_intent_segments;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_TARGETING_AD_TARGETING_SEGMENT_INFO_H_
