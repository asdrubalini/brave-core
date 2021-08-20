/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { Notification } from '../../shared/components/notifications'
import { NotificationCard } from './notification_card'

import * as styles from './notification_overlay.style'

interface Props {
  notifications: Notification[]
  onClose: () => void
}

export function NotificationOverlay (props: Props) {
  function onBackgroundClick (evt: React.UIEvent) {
    if (evt.target === evt.currentTarget) {
      props.onClose()
    }
  }

  if (props.notifications.length === 0) {
    return null
  }

  return (
    <styles.root onClick={onBackgroundClick}>
      <styles.card>
        <NotificationCard notification={props.notifications[0]} />
        {props.notifications.length > 1 && <styles.peek />}
      </styles.card>
    </styles.root>
  )
}
