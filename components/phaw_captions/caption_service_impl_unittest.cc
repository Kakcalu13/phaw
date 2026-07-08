// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/caption_service_impl.h"

#include <memory>
#include <string>
#include <utility>

#include "components/phaw_captions/caption_backend.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace phaw {
namespace {

// Backend whose install completion the test drives explicitly.
class FakeBackend : public CaptionBackend {
 public:
  void Install(const std::string& lang,
               base::OnceCallback<void(bool)> on_ready) override {
    ++install_calls;
    last_lang = lang;
    pending_ = std::move(on_ready);
  }
  void Start() override { ++start_calls; }
  void Stop() override { ++stop_calls; }

  void FinishInstall(bool ready) { std::move(pending_).Run(ready); }

  int install_calls = 0;
  int start_calls = 0;
  int stop_calls = 0;
  std::string last_lang;

 private:
  base::OnceCallback<void(bool)> pending_;
};

class CaptionServiceImplTest : public testing::Test {
 protected:
  CaptionServiceImplTest() {
    CaptionServiceImpl::RegisterProfilePrefs(prefs_.registry());
  }
  TestingPrefServiceSimple prefs_;
};

TEST_F(CaptionServiceImplTest, ReadyWhenSodaLoads) {
  auto backend = std::make_unique<FakeBackend>();
  FakeBackend* backend_ptr = backend.get();
  CaptionServiceImpl service(&prefs_, std::move(backend));

  service.EnsureModel("en-US");
  EXPECT_EQ(CaptionService::Status::kDownloading, service.status());
  EXPECT_EQ(1, backend_ptr->install_calls);

  backend_ptr->FinishInstall(/*ready=*/true);
  EXPECT_EQ(CaptionService::Status::kReady, service.status());
}

TEST_F(CaptionServiceImplTest, UnsupportedWhenSodaFailsToLoad) {
  auto backend = std::make_unique<FakeBackend>();
  FakeBackend* backend_ptr = backend.get();
  CaptionServiceImpl service(&prefs_, std::move(backend));

  service.EnsureModel("en-US");
  backend_ptr->FinishInstall(/*ready=*/false);
  EXPECT_EQ(CaptionService::Status::kUnsupportedOnOS, service.status());
}

TEST_F(CaptionServiceImplTest, StartsWhenEnabledAndReady) {
  auto backend = std::make_unique<FakeBackend>();
  FakeBackend* backend_ptr = backend.get();
  CaptionServiceImpl service(&prefs_, std::move(backend));

  service.SetEnabled(true);  // triggers install
  EXPECT_EQ(1, backend_ptr->install_calls);

  backend_ptr->FinishInstall(/*ready=*/true);
  EXPECT_EQ(CaptionService::Status::kReady, service.status());
  EXPECT_EQ(1, backend_ptr->start_calls);
}

}  // namespace
}  // namespace phaw
