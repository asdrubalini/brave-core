/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ad_targeting/ad_targeting.h"

#include "bat/ads/internal/ad_serving/ad_targeting/models/behavioral/bandits/epsilon_greedy_bandit_model.h"
#include "bat/ads/internal/ad_serving/ad_targeting/models/behavioral/purchase_intent/purchase_intent_model.h"
#include "bat/ads/internal/ad_serving/ad_targeting/models/contextual/text_classification/text_classification_model.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_segment_util.h"
#include "bat/ads/internal/features/bandits/epsilon_greedy_bandit_features.h"
#include "bat/ads/internal/features/purchase_intent/purchase_intent_features.h"
#include "bat/ads/internal/features/text_classification/text_classification_features.h"
#include "bat/ads/internal/logging.h"

namespace ads {
namespace ad_targeting {

namespace {

const int kTopTextClassificationSegmentCount = 3;

SegmentList FilterSegments(const SegmentList& segments, const int max_count) {
  SegmentList top_segments;

  int count = 0;

  for (const auto& segment : segments) {
    if (ShouldFilterSegment(segment)) {
      BLOG(1,
           "Excluding " << segment
                        << " segment due to being marked to no longer receive");
      continue;
    }

    top_segments.push_back(segment);
    count++;
    if (count == max_count) {
      break;
    }
  }

  return top_segments;
}

SegmentList GetTopSegments(const SegmentsInfo& segments,
                           const bool for_parent) {
  SegmentList list;

  const SegmentList text_classification_segments =
      for_parent ? segments.text_classification_segments
                 : GetParentSegments(segments.text_classification_segments);
  const SegmentList filtered_text_classification_segments = FilterSegments(
      text_classification_segments, kTopTextClassificationSegmentCount);
  list.insert(list.end(), filtered_text_classification_segments.begin(),
              filtered_text_classification_segments.end());

  list.insert(list.end(), segments.epsilon_greedy_bandit_segments.begin(),
              segments.epsilon_greedy_bandit_segments.end());

  list.insert(list.end(), segments.purchase_intent_segments.begin(),
              segments.purchase_intent_segments.end());

  return list;
}

}  // namespace

SegmentsInfo GetSegments() {
  SegmentsInfo segments;

  if (features::IsTextClassificationEnabled()) {
    const ad_targeting::model::TextClassification text_classification_model;
    segments.text_classification_segments =
        text_classification_model.GetSegments();
  }

  if (features::IsPurchaseIntentEnabled()) {
    const ad_targeting::model::PurchaseIntent purchase_intent_model;
    segments.purchase_intent_segments = purchase_intent_model.GetSegments();
  }

  if (features::IsEpsilonGreedyBanditEnabled()) {
    const ad_targeting::model::EpsilonGreedyBandit epsilon_greedy_bandit_model;
    segments.epsilon_greedy_bandit_segments =
        epsilon_greedy_bandit_model.GetSegments();
  }

  return segments;
}

SegmentList GetTopParentChildSegments(const SegmentsInfo& segments) {
  return GetTopSegments(segments, /* parent */ false);
}

SegmentList GetTopParentSegments(const SegmentsInfo& segments) {
  return GetTopSegments(segments, /* parent */ true);
}

}  // namespace ad_targeting
}  // namespace ads
