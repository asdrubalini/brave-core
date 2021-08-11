/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { ExternalWallet, ExternalWalletProvider } from '../../shared/lib/external_wallet'
import { ExternalWalletAction, RewardsSummaryData } from '../../shared/components/wallet_card'
import { Notification, NotificationAction } from '../../shared/components/notifications'

interface ExchangeInfo {
  currency: string
  rate: number
}

interface EarningsInfo {
  nextPaymentDate: Date
  earningsThisMonth: number
  earningsLastMonth: number
}

export interface PublisherInfo {
  name: string
  icon: string
  registered: boolean
  attentionScore: number
  autoContributeEnabled: boolean
  monthlyContribution: number
  supportedWalletProviders: ExternalWalletProvider[]
}

interface Settings {
  adsPerHour: number
  autoContributeAmount: number
}

interface Options {
  autoContributeAmounts: number[]
}

export interface HostState {
  rewardsEnabled: boolean
  balance: number
  settings: Settings
  options: Options
  exchangeInfo: ExchangeInfo
  earningsInfo: EarningsInfo
  publisherInfo: PublisherInfo | null
  externalWalletProviders: ExternalWalletProvider[]
  externalWallet: ExternalWallet | null
  summaryData: RewardsSummaryData
  notifications: Notification[]
  notificationsLastViewed: number
  hidePublisherUnverifiedNote: boolean
}

export type HostListener = (state: HostState) => void

export type MonthlyTipAction = 'update' | 'cancel'

export interface Host {
  state: HostState
  addListener: (callback: HostListener) => () => void
  getString: (key: string) => string
  enableRewards: () => void
  openRewardsSettings: () => void
  refreshPublisherStatus: () => void
  setIncludeInAutoContribute: (include: boolean) => void
  setAutoContributeAmount: (amount: number) => void
  setAdsPerHour: (adsPerHour: number) => void
  hidePublisherUnverifiedNote: () => void
  handleMonthlyTipAction: (action: MonthlyTipAction) => void
  handleExternalWalletAction: (action: ExternalWalletAction) => void
  handleNotificationAction: (action: NotificationAction) => void
  dismissNotification: (notification: Notification) => void
  clearNotifications: () => void
  setNotificationsViewed: () => void
}
