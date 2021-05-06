/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/base64url.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/test/scoped_feature_list.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/extensions/brave_base_local_data_files_browsertest.h"
#include "brave/common/brave_paths.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "brave/components/debounce/browser/debounce_component_installer.h"
#include "brave/components/debounce/common/features.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/url_util.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/default_handlers.h"

namespace {
const char kTestDataDirectory[] = "debounce-data";
}

namespace debounce {

class DebounceComponentInstallerWaiter
    : public DebounceComponentInstaller::Observer {
 public:
  explicit DebounceComponentInstallerWaiter(
      DebounceComponentInstaller* component_installer)
      : component_installer_(component_installer), scoped_observer_(this) {
    scoped_observer_.Add(component_installer_);
  }
  DebounceComponentInstallerWaiter(const DebounceComponentInstallerWaiter&) =
      delete;
  DebounceComponentInstallerWaiter& operator=(
      const DebounceComponentInstallerWaiter&) = delete;
  ~DebounceComponentInstallerWaiter() override = default;

  void Wait() { run_loop_.Run(); }

 private:
  // DebounceComponentInstaller::Observer:
  void OnRulesReady(DebounceComponentInstaller* component_installer) override {
    run_loop_.QuitWhenIdle();
  }

  DebounceComponentInstaller* const component_installer_;
  base::RunLoop run_loop_;
  ScopedObserver<DebounceComponentInstaller,
                 DebounceComponentInstaller::Observer>
      scoped_observer_;
};

class DebounceBrowserTest : public BaseLocalDataFilesBrowserTest {
 public:
  void SetUp() override {
    scoped_feature_list_.InitAndEnableFeature(
        debounce::features::kBraveDebounce);
    BaseLocalDataFilesBrowserTest::SetUp();
  }

  // BaseLocalDataFilesBrowserTest overrides
  const char* test_data_directory() override { return kTestDataDirectory; }
  const char* embedded_test_server_directory() override { return ""; }
  LocalDataFilesObserver* service() override {
    return g_brave_browser_process->debounce_component_installer();
  }

  void WaitForService() override {
    // Wait for debounce download service to load and parse its
    // configuration file.
    debounce::DebounceComponentInstaller* component_installer =
        g_brave_browser_process->debounce_component_installer();
    DebounceComponentInstallerWaiter(component_installer).Wait();
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    InProcessBrowserTest::SetUpCommandLine(command_line);
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  GURL add_redirect_param(const GURL& original_url, const GURL& landing_url) {
    return net::AppendOrReplaceQueryParameter(original_url, "url",
                                              landing_url.spec());
  }

  GURL add_base64_redirect_param(const GURL& original_url,
                                 const GURL& landing_url) {
    std::string encoded_destination;
    base::Base64UrlEncode(landing_url.spec(),
                          base::Base64UrlEncodePolicy::OMIT_PADDING,
                          &encoded_destination);
    const std::string query =
        base::StringPrintf("url=%s", encoded_destination.c_str());
    GURL::Replacements replacement;
    replacement.SetQueryStr(query);
    return original_url.ReplaceComponents(replacement);
  }

  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  void NavigateToURLAndWaitForRedirects(const GURL& original_url,
                                        const GURL& landing_url) {
    ui_test_utils::UrlLoadObserver load_complete(
        landing_url, content::NotificationService::AllSources());
    ui_test_utils::NavigateToURL(browser(), original_url);
    load_complete.Wait();
    EXPECT_EQ(web_contents()->GetLastCommittedURL(), landing_url);
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

// Test simple redirection by query parameter.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, Redirect) {
  ASSERT_TRUE(InstallMockExtension());
  GURL base_url = GURL("http://simple.a.com/");
  GURL landing_url = GURL("http://simple.b.com/");
  GURL original_url = add_redirect_param(base_url, landing_url);
  NavigateToURLAndWaitForRedirects(original_url, landing_url);
}

// Test base64-encoded redirection by query parameter.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, Base64Redirect) {
  ASSERT_TRUE(InstallMockExtension());
  GURL base_url = GURL("http://base64.a.com/");
  GURL landing_url = GURL("http://base64.b.com/");
  GURL original_url = add_base64_redirect_param(base_url, landing_url);
  NavigateToURLAndWaitForRedirects(original_url, landing_url);
}

// Test that debounce rules continue to be processed in order
// by constructing a URL that should be debounced to a second
// URL that should then be debounced to a third URL.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, DoubleRedirect) {
  ASSERT_TRUE(InstallMockExtension());
  GURL final_url = GURL("http://z.com/");
  GURL intermediate_url =
      add_redirect_param(GURL("http://double.b.com/"), final_url);
  GURL start_url =
      add_redirect_param(GURL("http://double.a.com/"), intermediate_url);
  NavigateToURLAndWaitForRedirects(start_url, final_url);
}

// Test that debounce rules are not processed twice by constructing
// a URL that should be debounced to a second URL that would be
// debounced to a third URL except that that rule has already been
// processed, so it won't.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, NotDoubleRedirect) {
  ASSERT_TRUE(InstallMockExtension());
  GURL final_url = GURL("http://z.com/");
  GURL intermediate_url =
      add_redirect_param(GURL("http://double.a.com/"), final_url);
  GURL start_url =
      add_redirect_param(GURL("http://double.b.com/"), intermediate_url);
  NavigateToURLAndWaitForRedirects(start_url, intermediate_url);
}

// Test wildcard URL patterns by constructing a URL that should be
// debounced because it matches a wildcard include pattern.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, WildcardInclude) {
  ASSERT_TRUE(InstallMockExtension());
  GURL landing_url = GURL("http://z.com/");
  GURL start_url =
      add_redirect_param(GURL("http://included.c.com/"), landing_url);
  NavigateToURLAndWaitForRedirects(start_url, landing_url);
}

// Test that unknown actions are ignored.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, UnknownAction) {
  ASSERT_TRUE(InstallMockExtension());
  GURL landing_url = GURL("http://z.com/");
  GURL start_url =
      add_redirect_param(GURL("http://included.d.com/"), landing_url);
  NavigateToURLAndWaitForRedirects(start_url, start_url);
}

// Test URL exclude patterns by constructing a URL that should be debounced
// because it matches a wildcard include pattern, then a second one
// that should not be debounced because it matches an exclude pattern.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, ExcludeOverridesWildcardInclude) {
  ASSERT_TRUE(InstallMockExtension());
  GURL landing_url = GURL("http://z.com/");
  GURL start_url_1 =
      add_redirect_param(GURL("http://included.e.com/"), landing_url);
  NavigateToURLAndWaitForRedirects(start_url_1, landing_url);
  GURL start_url_2 =
      add_redirect_param(GURL("http://excluded.e.com/"), landing_url);
  NavigateToURLAndWaitForRedirects(start_url_2, start_url_2);
}

// Test that debouncing rules only apply if the query parameter matches
// exactly.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, NoParamMatch) {
  ASSERT_TRUE(InstallMockExtension());
  GURL landing_url = GURL("http://z.com/");
  GURL start_url =
      add_redirect_param(GURL("http://included.f.com/"), landing_url);
  NavigateToURLAndWaitForRedirects(start_url, start_url);
}

// Test that extra keys in a rule are ignored and the rule is still
// processed and applied.
IN_PROC_BROWSER_TEST_F(DebounceBrowserTest, IgnoreExtraKeys) {
  ASSERT_TRUE(InstallMockExtension());
  GURL base_url = GURL("http://simple.g.com/");
  GURL landing_url = GURL("http://z.com/");
  GURL original_url = add_redirect_param(base_url, landing_url);
  NavigateToURLAndWaitForRedirects(original_url, landing_url);
}

}  // namespace debounce
