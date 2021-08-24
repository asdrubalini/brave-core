/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext } from '../../shared/lib/locale_context'
import { HostContext, useHostListener } from '../lib/host_context'
import { MonthlyTipAction } from '../lib/interfaces'
import { ToggleButton } from './toggle_button'
import { MonthlyTipView } from './monthly_tip_view'
import { VerifiedIcon } from './icons/verified_icon'
import { LoadingIcon } from './icons/loading_icon'

import * as styles from './publisher_card.style'

export function PublisherCard () {
  const { getString } = React.useContext(LocaleContext)
  const host = React.useContext(HostContext)

  const [publisherInfo, setPublisherInfo] =
    React.useState(host.state.publisherInfo)
  const [publisherRefreshing, setPublisherRefreshing] =
    React.useState(host.state.publisherRefreshing)

  const [showPublisherLoading, setShowPublisherLoading] = React.useState(false)

  useHostListener(host, (state) => {
    setPublisherInfo(state.publisherInfo)
    setPublisherRefreshing(state.publisherRefreshing)
  })

  if (!publisherInfo) {
    return null
  }

  function renderStatusMessage () {
    if (!publisherInfo) {
      return null
    }

    if (publisherInfo.registered) {
      return (
        <styles.verified>
          <VerifiedIcon />{getString('verifiedCreator')}
        </styles.verified>
      )
    }

    return (
      <styles.unverified>
        <VerifiedIcon />{getString('unverifiedCreator')}
      </styles.unverified>
    )
  }

  function onRefreshClick (evt: React.UIEvent) {
    evt.preventDefault()

    // Show the publisher loading state for a minimum amount of time in order
    // to indicate activity to the user.
    setShowPublisherLoading(true)
    setTimeout(() => { setShowPublisherLoading(false) }, 500)

    host.refreshPublisherStatus()
  }

  function monthlyTipHandler (action: MonthlyTipAction) {
    return () => {
      host.handleMonthlyTipAction(action)
    }
  }

  return (
    <styles.root>
      <styles.heading>
        {
          publisherInfo.icon &&
            <styles.icon>
              <img src={publisherInfo.icon} />
            </styles.icon>
        }
        <styles.name>
          {publisherInfo.name}
          <styles.status>
            {renderStatusMessage()}
            <styles.refreshStatus>
              {
                publisherRefreshing || showPublisherLoading
                  ? <LoadingIcon />
                  : <a href='#' onClick={onRefreshClick}>
                      {getString('refreshStatus')}
                    </a>
              }
            </styles.refreshStatus>
          </styles.status>
        </styles.name>
      </styles.heading>
      <styles.attention>
        <div>{getString('attention')}</div>
        <div className='value'>
          {(publisherInfo.attentionScore * 100).toFixed(0)}%
        </div>
      </styles.attention>
      <styles.contribution>
        <styles.autoContribution>
          <div>{getString('includeInAutoContribute')}</div>
          <div>
            <ToggleButton
              checked={publisherInfo.autoContributeEnabled}
              onChange={host.setIncludeInAutoContribute}
            />
          </div>
        </styles.autoContribution>
        <styles.monthlyContribution>
          <div>{getString('monthlyContribution')}</div>
          <div>
            <MonthlyTipView
              publisherInfo={publisherInfo}
              onUpdateClick={monthlyTipHandler('update')}
              onCancelClick={monthlyTipHandler('cancel')}
            />
          </div>
        </styles.monthlyContribution>
      </styles.contribution>
      <styles.tipAction>
        <button onClick={host.sendTip}>
          {getString('sendTip')}
        </button>
      </styles.tipAction>
    </styles.root>
  )
}
