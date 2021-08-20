/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext } from '../../shared/lib/locale_context'
import { GrantCaptchaInfo } from '../lib/interfaces'
import { GrantCaptchaChallenge } from './grant_captcha_challenge'
import { Modal, ModalCloseButton } from '../../shared/components/modal'
import { TokenAmount } from '../../shared/components/token_amount'

import * as styles from './grant_captcha_modal.style'

interface Props {
  grantCaptchaInfo: GrantCaptchaInfo
  onSolve: (solution: { x: number, y: number }) => void
  onClose: () => void
}

export function GrantCaptchaModal (props: Props) {
  const { getString } = React.useContext(LocaleContext)
  const { grantCaptchaInfo } = props
  const { grantInfo } = grantCaptchaInfo

  function renderChallenge (message: string) {
    return (
      <>
        <styles.header>{getString(message)}</styles.header>
        <GrantCaptchaChallenge
          grantCaptchaInfo={grantCaptchaInfo}
          onSolve={props.onSolve}
        />
      </>
    )
  }

  function renderPassed () {
    const dateFormatter = Intl.DateTimeFormat(undefined, {
      year: 'numeric',
      month: 'numeric',
      day: 'numeric'
    })

    function getStrings () {
      switch (grantInfo.source) {
        case 'ads':
          return {
            title: getString('grantCaptchaPassedTitleAds'),
            text: getString('grantCaptchaPassedTextAds'),
            amountLabel: getString('grantCaptchaAmountAds')
          }
        case 'ugp':
          return {
            title: getString('grantCaptchaPassedTitleUGP'),
            text: getString('grantCaptchaPassedTextUGP'),
            amountLabel: getString('grantCaptchaAmountUGP')
          }
      }
    }

    const { title, text, amountLabel } = getStrings()

    return (
      <>
        <styles.header>{title}</styles.header>
        <styles.text>{text}</styles.text>
        <styles.summary>
          <styles.summaryItem>
            {amountLabel}
            <styles.summaryValue>
              <TokenAmount amount={grantInfo.amount} />
            </styles.summaryValue>
          </styles.summaryItem>
          {
            grantInfo.expiresAt !== null &&
              <styles.summaryItem>
                {getString('grantCaptchaExpiration')}
                <styles.summaryValue>
                  {dateFormatter.format(grantInfo.expiresAt)}
                </styles.summaryValue>
              </styles.summaryItem>
          }
        </styles.summary>
        <styles.okButton>
          <button onClick={props.onClose}>{getString('ok')}</button>
        </styles.okButton>
      </>
    )
  }

  function renderError () {
    return (
      <>
        <styles.header>
          {getString('grantCaptchaErrorTitle')}
        </styles.header>
        <styles.text>
          {getString('grantCaptchaErrorText')}
        </styles.text>
        <styles.okButton>
          <button onClick={props.onClose}>{getString('ok')}</button>
        </styles.okButton>
      </>
    )
  }

  function renderContent (): React.ReactNode {
    switch (props.grantCaptchaInfo.status) {
      case 'pending': return renderChallenge('grantCaptchaTitle')
      case 'passed': return renderPassed()
      case 'failed': return renderChallenge('grantCaptchaFailedTitle')
      case 'error': return renderError()
    }
  }

  return (
    <Modal>
      <styles.root>
        <ModalCloseButton onClick={props.onClose} />
        {renderContent()}
      </styles.root>
    </Modal>
  )
}
