/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext } from '../../shared/lib/locale_context'
import { ManageWalletIcon } from './icons/manage_wallet_icon'

import * as styles from './manage_wallet_button.style'

interface Props {
  onClick: () => void
}

export function ManageWalletButton (props: Props) {
  const { getString } = React.useContext(LocaleContext)
  return (
    <styles.root>
      <button onClick={props.onClick}>
        <ManageWalletIcon />{getString('manageWallet')}
      </button>
    </styles.root>
  )
}
