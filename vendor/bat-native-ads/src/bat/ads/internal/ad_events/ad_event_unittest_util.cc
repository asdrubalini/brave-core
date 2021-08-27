/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ad_events/ad_event_unittest_util.h"

#include "base/guid.h"

namespace ads {

AdEventInfo GetAdEvent(
    const CreativeAdNotificationInfo& creative_ad_notification,
    const ConfirmationType confirmation_type,
    const base::Time& timestamp) {
  AdEventInfo ad_event;
  ad_event.uuid = base::GenerateGUID();
  ad_event.confirmation_type = confirmation_type;
  ad_event.creative_instance_id = creative_ad_notification.creative_instance_id;
  ad_event.advertiser_id = creative_ad_notification.advertiser_id;
  ad_event.timestamp = static_cast<int64_t>(timestamp.ToDoubleT());

  return ad_event;
}

}  // namespace ads
