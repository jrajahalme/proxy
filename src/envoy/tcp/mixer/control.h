/* Copyright 2017 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "common/common/logger.h"
#include "envoy/event/dispatcher.h"
#include "envoy/local_info/local_info.h"
#include "envoy/runtime/runtime.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/upstream/cluster_manager.h"
#include "include/istio/control/tcp/controller.h"
#include "include/istio/utils/local_attributes.h"
#include "src/envoy/tcp/mixer/config.h"
#include "src/envoy/utils/stats.h"

namespace Envoy {
namespace Tcp {
namespace Mixer {

class ControlData {
 public:
  ControlData(std::unique_ptr<Config> config, Utils::MixerFilterStats stats,
              const std::string& uuid)
      : config_(std::move(config)), stats_(stats), uuid_(uuid) {}

  const Config& config() { return *config_; }
  Utils::MixerFilterStats& stats() { return stats_; }
  const std::string& uuid() { return uuid_; }

 private:
  std::unique_ptr<Config> config_;
  Utils::MixerFilterStats stats_;
  // UUID of the Envoy TCP mixer filter.
  const std::string uuid_;
};

typedef std::shared_ptr<ControlData> ControlDataSharedPtr;

class Control final : public ThreadLocal::ThreadLocalObject {
 public:
  // The constructor.
  Control(ControlDataSharedPtr control_data, Upstream::ClusterManager& cm,
          Event::Dispatcher& dispatcher, Runtime::RandomGenerator& random,
          Stats::Scope& scope, const LocalInfo::LocalInfo& local_info);

  ::istio::control::tcp::Controller* controller() { return controller_.get(); }

  Event::Dispatcher& dispatcher() { return dispatcher_; }

  const std::string& uuid() const { return control_data_->uuid(); }

  const Config& config() const { return control_data_->config(); }

 private:
  // Call controller to get statistics.
  bool GetStats(::istio::mixerclient::Statistics* stat);

  // The control data.
  ControlDataSharedPtr control_data_;

  // dispatcher.
  Event::Dispatcher& dispatcher_;

  // Pre-serialized attributes_for_mixer_proxy.
  std::string serialized_forward_attributes_;

  // async client factories
  Grpc::AsyncClientFactoryPtr check_client_factory_;
  Grpc::AsyncClientFactoryPtr report_client_factory_;

  // statistics
  Utils::MixerStatsObject stats_obj_;

  // The mixer control
  std::unique_ptr<::istio::control::tcp::Controller> controller_;
};

}  // namespace Mixer
}  // namespace Tcp
}  // namespace Envoy
