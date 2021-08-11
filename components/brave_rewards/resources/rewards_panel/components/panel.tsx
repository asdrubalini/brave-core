/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { HostContext, useHostListener } from '../lib/host_context'
import { WalletCard } from '../../shared/components/wallet_card'
import { RewardsOptInModal, RewardsTourModal } from '../../shared/components/onboarding'
import { NavBar } from './navbar'
import { NotificationOverlay } from './notification_overlay'
import { PublisherCard } from './publisher_card'

import * as styles from './panel.style'

type ActiveView = 'tip' | 'summary'

export function Panel () {
  const host = React.useContext(HostContext)

  const [rewardsEnabled, setRewardsEnabled] =
    React.useState(host.state.rewardsEnabled)
  const [balance, setBalance] = React.useState(host.state.balance)
  const [externalWallet, setExternalWallet] =
    React.useState(host.state.externalWallet)
  const [exchangeInfo, setExchangeInfo] =
    React.useState(host.state.exchangeInfo)
  const [earningsInfo, setEarningsInfo] =
    React.useState(host.state.earningsInfo)
  const [summaryData, setSummaryData] = React.useState(host.state.summaryData)
  const [publisherInfo, setPublisherInfo] =
    React.useState(host.state.publisherInfo)
  const [settings, setSettings] = React.useState(host.state.settings)
  const [options, setOptions] = React.useState(host.state.options)
  const [externalWalletProviders, setExternalWalletProviders] =
    React.useState(host.state.externalWalletProviders)

  const [activeView, setActiveView] = React.useState<ActiveView>(
    publisherInfo ? 'tip' : 'summary')

  const [showTour, setShowTour] = React.useState(false)

  useHostListener(host, (state) => {
    setRewardsEnabled(state.rewardsEnabled)
    setBalance(state.balance)
    setExternalWallet(state.externalWallet)
    setExchangeInfo(state.exchangeInfo)
    setEarningsInfo(state.earningsInfo)
    setSummaryData(state.summaryData)
    setPublisherInfo(state.publisherInfo)
    setSettings(state.settings)
    setOptions(state.options)
    setExternalWalletProviders(state.externalWalletProviders)
  })

  function toggleTour () {
    setShowTour(!showTour)
  }

  function onEnable () {
    host.enableRewards()
    setShowTour(true)
  }

  function renderOverlays () {
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

    return <NotificationOverlay />
  }

  return (
    <styles.root>
      <WalletCard
        balance={balance}
        externalWallet={externalWallet}
        earningsThisMonth={earningsInfo.earningsThisMonth}
        earningsLastMonth={earningsInfo.earningsLastMonth}
        nextPaymentDate={earningsInfo.nextPaymentDate}
        exchangeRate={exchangeInfo.rate}
        exchangeCurrency={exchangeInfo.currency}
        showSummary={activeView === 'summary'}
        summaryData={summaryData}
        onExternalWalletAction={host.handleExternalWalletAction}
      />
      {activeView === 'tip' && <PublisherCard />}
      <NavBar
        canTip={Boolean(publisherInfo)}
        activeView={activeView}
        onActiveViewChange={setActiveView}
        onSettingsClick={host.openRewardsSettings}
      />
      {renderOverlays()}
    </styles.root>
  )
}
