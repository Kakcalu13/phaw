// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_adblock/adblock_throttle.h"

#include <string>
#include <string_view>
#include <vector>

#include "base/functional/bind.h"
#include "components/phaw_adblock/adblock_engine.h"
#include "services/network/public/cpp/resource_request.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace phaw {
namespace {

// Blocks exactly requests to "ads.example.com".
class FakeEngine : public AdblockEngine {
 public:
  AdblockMatch Match(const GURL& url,
                     const url::Origin&,
                     const std::string&) const override {
    AdblockMatch match;
    match.blocked = url.host() == "ads.example.com";
    return match;
  }
  std::vector<std::string> CosmeticSelectorsFor(const GURL&) const override {
    return {};
  }
};

// Minimal blink::URLLoaderThrottle::Delegate that records cancellation.
// confirm-in-tree: cover the full pure-virtual set (and CancelWithError's exact
// signature) for the pinned milestone; unlisted methods are no-ops.
class FakeDelegate : public blink::URLLoaderThrottle::Delegate {
 public:
  void CancelWithError(int error, std::string_view reason) override {
    cancelled = true;
    last_error = error;
  }
  void Resume() override {}
  void SetPriority(net::RequestPriority) override {}
  void PauseReadingBodyFromNet() override {}
  void ResumeReadingBodyFromNet() override {}

  bool cancelled = false;
  int last_error = 0;
};

network::ResourceRequest MakeRequest(const char* url, const char* initiator) {
  network::ResourceRequest request;
  request.url = GURL(url);
  request.request_initiator = url::Origin::Create(GURL(initiator));
  return request;
}

TEST(AdblockThrottleTest, CancelsBlockedRequest) {
  FakeEngine engine;
  int reported = 0;
  AdblockThrottle throttle(
      &engine, /*tab_id=*/7,
      base::BindRepeating([](int* c, const GURL&, int) { ++*c; }, &reported));
  FakeDelegate delegate;
  throttle.set_delegate(&delegate);

  network::ResourceRequest request =
      MakeRequest("https://ads.example.com/a.js", "https://news.example.org");
  bool defer = false;
  throttle.WillStartRequest(&request, &defer);

  EXPECT_TRUE(delegate.cancelled);
  EXPECT_EQ(1, reported);
}

TEST(AdblockThrottleTest, AllowsUnmatchedRequest) {
  FakeEngine engine;
  int reported = 0;
  AdblockThrottle throttle(
      &engine, /*tab_id=*/7,
      base::BindRepeating([](int* c, const GURL&, int) { ++*c; }, &reported));
  FakeDelegate delegate;
  throttle.set_delegate(&delegate);

  network::ResourceRequest request = MakeRequest(
      "https://news.example.org/app.js", "https://news.example.org");
  bool defer = false;
  throttle.WillStartRequest(&request, &defer);

  EXPECT_FALSE(delegate.cancelled);
  EXPECT_EQ(0, reported);
}

}  // namespace
}  // namespace phaw
