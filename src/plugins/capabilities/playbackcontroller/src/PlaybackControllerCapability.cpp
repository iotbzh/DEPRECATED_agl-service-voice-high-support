/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#include "capabilities/playbackcontroller/include/PlaybackControllerCapability.h"
#include "capabilities/playbackcontroller/include/PlaybackControllerMessages.h"

const string TAG = "vshl::capabilities::playbackcontroller";

using Level = vshl::common::interfaces::ILogger::Level;

namespace vshl {
namespace capabilities {
namespace playbackcontroller {

// Create a playbackcontroller.
shared_ptr<PlaybackController> PlaybackController::create(
    shared_ptr<vshl::common::interfaces::ILogger> logger) {
    auto playbackcontroller = std::shared_ptr<PlaybackController>(new PlaybackController(logger));
    return playbackcontroller;
}

PlaybackController::PlaybackController(
    shared_ptr<vshl::common::interfaces::ILogger> logger) {
    mLogger = logger;
}

string PlaybackController::getName() const {
    return NAME;
}

list<string> PlaybackController::getUpstreamMessages() const {
    return PLAYBACKCONTROLLER_UPSTREAM_ACTIONS;
}

list<string> PlaybackController::getDownstreamMessages() const {
    return PLAYBACKCONTROLLER_DOWNSTREAM_ACTIONS;
}

void PlaybackController::onMessagePublished(const string action) {

}

}  // namespace playbackcontroller
}  // namespace capabilities
}  // namespace vshl
