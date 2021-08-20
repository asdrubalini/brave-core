/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { HostContext, useHostListener } from '../lib/host_context'
import { RewardsOptInModal, RewardsTourModal } from '../../shared/components/onboarding'
import { GrantCaptchaModal } from './grant_captcha_modal'
import { NotificationOverlay } from './notification_overlay'

export function PanelOverlays () {
  const host = React.useContext(HostContext)

  const [rewardsEnabled, setRewardsEnabled] =
    React.useState(host.state.rewardsEnabled)
  const [settings, setSettings] = React.useState(host.state.settings)
  const [options, setOptions] = React.useState(host.state.options)
  const [externalWalletProviders, setExternalWalletProviders] =
    React.useState(host.state.externalWalletProviders)
  const [grantCaptchaInfo, setGrantCaptchaInfo] =
    React.useState(host.state.grantCaptchaInfo)
  const [notifications, setNotifications] =
    React.useState(host.state.notifications)
  const [notificationsLastViewed, setNotificationsLastViewed ] =
    React.useState(host.state.notificationsLastViewed)

  const [showTour, setShowTour] = React.useState(false)

  useHostListener(host, (state) => {
    setRewardsEnabled(state.rewardsEnabled)
    setSettings(state.settings)
    setOptions(state.options)
    setExternalWalletProviders(state.externalWalletProviders)
    setGrantCaptchaInfo(state.grantCaptchaInfo)
    setNotifications(state.notifications)
    setNotificationsLastViewed(state.notificationsLastViewed)
  })

  function toggleTour () {
    setShowTour(!showTour)
  }

  function onEnable () {
    host.enableRewards()
    setShowTour(true)
  }

  if (showTour) {
    const onVerifyWalletClick = () => {
      host.handleExternalWalletAction('verify')
    }

    return (
      <RewardsTourModal
        firstTimeSetup={rewardsEnabled}
        adsPerHour={settings.adsPerHour}
        externalWalletProvider={externalWalletProviders[0]}
        autoContributeAmount={settings.autoContributeAmount}
        autoContributeAmountOptions={options.autoContributeAmounts}
        onAdsPerHourChanged={host.setAdsPerHour}
        onAutoContributeAmountChanged={host.setAutoContributeAmount}
        onVerifyWalletClick={onVerifyWalletClick}
        onDone={toggleTour}
        onClose={toggleTour}
      />
    )
  }

  if (!rewardsEnabled) {
    return <RewardsOptInModal onEnable={onEnable} onTakeTour={toggleTour} />
  }

  if (grantCaptchaInfo) {

    return (
      <GrantCaptchaModal
        grantCaptchaInfo={grantCaptchaInfo}
        onSolve={host.solveGrantCaptcha}
        onClose={host.clearGrantCaptcha}
      />
    )
  }

  const activeNotifications = Array.from(notifications)
    .sort((a, b) => b.timeStamp - a.timeStamp)
    .filter(n => n.timeStamp > notificationsLastViewed)

  if (activeNotifications.length > 0) {
    return (
      <NotificationOverlay
        notifications={activeNotifications}
        onClose={host.setNotificationsViewed}
      />
    )
  }

  return null
}
