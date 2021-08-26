/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/toolbar/brave_vpn_button.h"

#include <memory>
#include <utility>

#include "base/notreached.h"
#include "brave/app/vector_icons/vector_icons.h"
#include "brave/browser/brave_vpn/brave_vpn_service_factory.h"
#include "brave/browser/themes/theme_properties.h"
#include "brave/common/webui_url_constants.h"
#include "brave/components/brave_vpn/brave_vpn_service.h"
#include "brave/grit/brave_generated_resources.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/themes/theme_properties.h"
#include "chrome/browser/ui/layout_constants.h"
#include "third_party/blink/public/mojom/frame/user_activation_notification_type.mojom-shared.h"
#include "content/public/browser/storage_partition.h"
#include "chrome/browser/ui/views/toolbar/toolbar_ink_drop_util.h"
#include "content/public/browser/navigation_controller.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/rrect_f.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/highlight_path_generator.h"

VpnLoginStatusDelegate::VpnLoginStatusDelegate() = default;
VpnLoginStatusDelegate::~VpnLoginStatusDelegate() = default;

void VpnLoginStatusDelegate::PassiveInsecureContentFound(const GURL& resource_url) {
  LOG(ERROR) << "BSC]] WHOOPS ] " << resource_url.spec();
}

bool VpnLoginStatusDelegate::ShouldAllowLazyLoad() {
  return false;
}

bool VpnLoginStatusDelegate::ShouldAllowRunningInsecureContent(
    content::WebContents* web_contents,
    bool allowed_per_prefs,
    const url::Origin& origin,
    const GURL& resource_url) {
  return true;
}

void VpnLoginStatusDelegate::OnDidBlockNavigation(
      content::WebContents* web_contents,
      const GURL& blocked_url,
      const GURL& initiator_url,
      blink::mojom::NavigationBlockedReason reason) {
  LOG(ERROR) << "BSC]] WHOOPS2 ] " << blocked_url;
}

void VpnLoginStatusDelegate::UpdateTargetURL(content::WebContents* source,
    const GURL& url) {
  LOG(ERROR) << "BSC]] UpdateTargetURL\nurl=" << url;
}

bool VpnLoginStatusDelegate::ShouldSuppressDialogs(content::WebContents* source){
  return true;
}

void VpnLoginStatusDelegate::LoadingStateChanged(content::WebContents* source,
                                                 bool to_different_document) {
  LOG(ERROR) << "BSC]] LoadingStateChanged\nto_different_document="
             << to_different_document << "\nIsLoading=" << source->IsLoading();
  if (!source->IsLoading()) {
    LOG(ERROR) << "BSC]] FINISHED LOADING";

    // NOTES:
    // Before you can properly test, you need to open a tab to account.brave.software
    // and login. There, you can purchase Talk (DM me if you need information).
    // After that, you can close the tab and the browser. Local storage has been
    // written to :)
    //
    // PROBLEM I'M TRYING TO SOLVE:
    // When you open a fresh instance of Brave, use brave://flags to enable VPN
    // and then click the VPN button, this code IS executing.
    // The page does indeed load (first load takes about 10 seconds or so).
    // As part of loading, it DOES NOT write anything to console.
    // ex: There are no calls to DidAddMessageToConsole().
    //
    // If you open a tab and manually navigate to https://account.brave.software/skus/
    // It will suddenly ACTUALLY finish loading the code that this tries to load
    // and messages are being logged via DidAddMessageToConsole.
    // Specifically the `rewards sdk initialized` one.
    //
    // Why does this need one user-opened instance to be visited first??!


    // open questions
    // - is something storage related being locked?
    // - is the URL redirecting?
    // - does the web contents need to be marked as being user opened?
    auto* main_frame = source->GetMainFrame();

    // Storage path is accessible
    auto* storage = main_frame->GetStoragePartition();
    LOG(ERROR) << "BSC]] storage GetPath() " << storage->GetPath();

    // I thought maybe this would help "activate" the web contents, but it doesn't do anything
    main_frame->NotifyUserActivation(blink::mojom::UserActivationNotificationType::kInteraction);
  }
}

bool VpnLoginStatusDelegate::DidAddMessageToConsole(
    content::WebContents* source,
    blink::mojom::ConsoleMessageLevel log_level,
    const std::u16string& message,
    int32_t line_no,
    const std::u16string& source_id) {
  LOG(ERROR) << "BSC]] DidAddMessageToConsole\nmessage=" << message;

  std::size_t found = message.find(u"rewards sdk initialized");
  if (found != std::u16string::npos) {
    LOG(ERROR) << "SKU SDK is initialized! Try to get reference to "
                  "`navigator.brave.skus`";

    const char16_t kGetTheCookie[] =
        uR"(
let retries = 10;
let wait_for_sdk_id = window.setInterval(() => {
  let sku_sdk = navigator.brave.skus;
  if (sku_sdk) {
    sku_sdk.prepare_credentials_presentation('talk.brave.software', '*').then((response) => {
      console.log(response);
    });
    window.clearInterval(wait_for_sdk_id);
  } else {
    retries--;
    if (retries <= 0) {
      console.log('BSC]] giving up')
      window.clearInterval(wait_for_sdk_id);
    }
  }
}, 1000);
)";

    std::u16string get_my_cookie(kGetTheCookie);
    auto* main_frame = source->GetMainFrame();
    main_frame->ExecuteJavaScript(get_my_cookie, base::NullCallback());
    return false;
  }
  found = message.find(u"__Secure-sku#");
  if (found != std::u16string::npos) {
    LOG(ERROR) << "GOT THE CREDENTIAL! " << message;
  }

  return true;
}

namespace {

constexpr int kButtonRadius = 47;

class BraveVPNButtonHighlightPathGenerator
    : public views::HighlightPathGenerator {
 public:
  explicit BraveVPNButtonHighlightPathGenerator(const gfx::Insets& insets)
      : HighlightPathGenerator(insets) {}

  BraveVPNButtonHighlightPathGenerator(
      const BraveVPNButtonHighlightPathGenerator&) = delete;
  BraveVPNButtonHighlightPathGenerator& operator=(
      const BraveVPNButtonHighlightPathGenerator&) = delete;

  // views::HighlightPathGenerator overrides:
  absl::optional<gfx::RRectF> GetRoundRect(const gfx::RectF& rect) override {
    return gfx::RRectF(rect, kButtonRadius);
  }
};

}  // namespace

BraveVPNButton::BraveVPNButton(Profile* profile)
    : ToolbarButton(base::BindRepeating(&BraveVPNButton::OnButtonPressed,
                                        base::Unretained(this))),
      service_(BraveVpnServiceFactory::GetForProfile(profile)),
      webui_bubble_manager_(this, profile, GURL(kVPNPanelURL), 1, true) {
  DCHECK(service_);
  observation_.Observe(service_);

  // Replace ToolbarButton's highlight path generator.
  views::HighlightPathGenerator::Install(
      this, std::make_unique<BraveVPNButtonHighlightPathGenerator>(
                GetToolbarInkDropInsets(this)));

  label()->SetText(
      l10n_util::GetStringUTF16(IDS_BRAVE_VPN_TOOLBAR_BUTTON_TEXT));
  gfx::FontList font_list = views::Label::GetDefaultFontList();
  constexpr int kFontSize = 12;
  label()->SetFontList(
      font_list.DeriveWithSizeDelta(kFontSize - font_list.GetFontSize()));

  // Set image positions first. then label.
  SetHorizontalAlignment(gfx::ALIGN_LEFT);

  UpdateButtonState();

  // USED TO CHECK IF THEY ARE LOGGED IN LOL
  content::WebContents::CreateParams params(profile);
  contents_ = content::WebContents::Create(params);
  contents_delegate_.reset(new VpnLoginStatusDelegate);
  contents_->SetDelegate(contents_delegate_.get());
}
// TODO(bsclifton): clean up contents
BraveVPNButton::~BraveVPNButton() = default;

void BraveVPNButton::OnConnectionStateChanged(bool connected) {
  UpdateButtonState();
}

void BraveVPNButton::OnConnectionCreated() {
  // Do nothing.
}

void BraveVPNButton::OnConnectionRemoved() {
  // Do nothing.
}

void BraveVPNButton::UpdateColorsAndInsets() {
  if (const auto* tp = GetThemeProvider()) {
    const gfx::Insets paint_insets =
        gfx::Insets((height() - GetLayoutConstant(LOCATION_BAR_HEIGHT)) / 2);
    SetBackground(views::CreateBackgroundFromPainter(
        views::Painter::CreateSolidRoundRectPainter(
            tp->GetColor(ThemeProperties::COLOR_TOOLBAR), kButtonRadius,
            paint_insets)));

    SetEnabledTextColors(tp->GetColor(
        IsConnected()
            ? BraveThemeProperties::COLOR_BRAVE_VPN_BUTTON_TEXT_CONNECTED
            : BraveThemeProperties::COLOR_BRAVE_VPN_BUTTON_TEXT_DISCONNECTED));

    std::unique_ptr<views::Border> border = views::CreateRoundedRectBorder(
        1, kButtonRadius, gfx::Insets(),
        tp->GetColor(BraveThemeProperties::COLOR_BRAVE_VPN_BUTTON_BORDER));
    constexpr gfx::Insets kTargetInsets{4, 6};
    const gfx::Insets extra_insets = kTargetInsets - border->GetInsets();
    SetBorder(views::CreatePaddedBorder(std::move(border), extra_insets));
  }

  constexpr int kBraveAvatarImageLabelSpacing = 4;
  SetImageLabelSpacing(kBraveAvatarImageLabelSpacing);
}

void BraveVPNButton::UpdateButtonState() {
  constexpr SkColor kColorConnected = SkColorSetRGB(0x51, 0xCF, 0x66);
  constexpr SkColor kColorDisconnected = SkColorSetRGB(0xAE, 0xB1, 0xC2);
  SetImage(views::Button::STATE_NORMAL,
           gfx::CreateVectorIcon(kVpnIndicatorIcon, IsConnected()
                                                        ? kColorConnected
                                                        : kColorDisconnected));
}

bool BraveVPNButton::IsConnected() {
  return service_->IsConnected();
}

void BraveVPNButton::OnButtonPressed(const ui::Event& event) {
  ShowBraveVPNPanel();

  if (contents_) {
    content::RenderFrameHost::AllowInjectingJavaScript();
    GURL url = GURL("https://account.brave.software/skus/");
    std::string extra_headers =
        "Authorization: Basic BASE64_ENCODED_USER:PASSWORD_HERE";
    contents_->GetController().LoadURL(url, content::Referrer(),
                                       ui::PAGE_TRANSITION_TYPED,
                                       extra_headers);
  }
}

void BraveVPNButton::ShowBraveVPNPanel() {
  if (webui_bubble_manager_.GetBubbleWidget()) {
    webui_bubble_manager_.CloseBubble();
    return;
  }

  webui_bubble_manager_.ShowBubble();
}

BEGIN_METADATA(BraveVPNButton, LabelButton)
END_METADATA
