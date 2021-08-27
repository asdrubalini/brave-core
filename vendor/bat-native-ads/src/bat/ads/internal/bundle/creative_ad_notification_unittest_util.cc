/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/bundle/creative_ad_notification_unittest_util.h"

#include "base/guid.h"

namespace ads {

namespace {

int64_t kDistantPastAsTimestamp = 0;
int64_t kDistantFutureAsTimestamp = 4102444799;

}  // namespace

CreativeAdNotificationInfo GetCreativeAdNotificationDefaultValues() {
  CreativeAdNotificationInfo creative_ad_notification;

  creative_ad_notification.creative_instance_id = base::GenerateGUID();
  creative_ad_notification.creative_set_id = base::GenerateGUID();
  creative_ad_notification.campaign_id = base::GenerateGUID();
  creative_ad_notification.start_at_timestamp = kDistantPastAsTimestamp;
  creative_ad_notification.end_at_timestamp = kDistantFutureAsTimestamp;
  creative_ad_notification.daily_cap = 1;
  creative_ad_notification.advertiser_id = base::GenerateGUID();
  creative_ad_notification.priority = 1;
  creative_ad_notification.ptr = 1.0;
  creative_ad_notification.per_day = 1;
  creative_ad_notification.per_week = 1;
  creative_ad_notification.per_month = 1;
  creative_ad_notification.total_max = 1;
  creative_ad_notification.segment = "untargeted";
  creative_ad_notification.geo_targets = {"US"};
  creative_ad_notification.target_url = "https://brave.com";
  CreativeDaypartInfo daypart;
  creative_ad_notification.dayparts = {daypart};
  creative_ad_notification.title = "Test Ad Title";
  creative_ad_notification.body = "Test Ad Body";

  return creative_ad_notification;
}

CreativeAdNotificationInfo GetCreativeAdNotification() {
  return GetCreativeAdNotificationDefaultValues();
}

CreativeAdNotificationInfo GetCreativeAdNotification(const std::string& segment,
                                                     const double ptr,
                                                     const int priority) {
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationDefaultValues();

  creative_ad_notification.priority = priority;
  creative_ad_notification.ptr = ptr;
  creative_ad_notification.segment = segment;

  return creative_ad_notification;
}

CreativeAdNotificationInfo GetCreativeAdNotification(
    const std::string& advertiser_id) {
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationDefaultValues();

  creative_ad_notification.advertiser_id = advertiser_id;

  return creative_ad_notification;
}

CreativeAdNotificationInfo GetCreativeAdNotification(
    const std::string& creative_instance_id,
    const std::string& segment) {
  CreativeAdNotificationInfo creative_ad_notification =
      GetCreativeAdNotificationDefaultValues();

  creative_ad_notification.creative_instance_id = creative_instance_id;
  creative_ad_notification.segment = segment;

  return creative_ad_notification;
}

}  // namespace ads
