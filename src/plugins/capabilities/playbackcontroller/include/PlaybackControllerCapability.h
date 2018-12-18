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

#ifndef VSHL_CAPABILITIES_PLAYBACKCONTROLLER_CAPABILITY_H_
#define VSHL_CAPABILITIES_PLAYBACKCONTROLLER_CAPABILITY_H_

#include <memory>

#include "interfaces/capabilities/ICapability.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace vshl {
namespace capabilities {
namespace playbackcontroller {

/*
 * PlaybackController capability. Calls are initiated in the endpoint.
 */
class PlaybackController : public common::interfaces::ICapability {
public:
    // Create a PlaybackController.
    static std::shared_ptr<PlaybackController> create(
        shared_ptr<vshl::common::interfaces::ILogger> logger);

    ~PlaybackController() = default;

protected:
    string getName() const override;

    list<string> getUpstreamMessages() const override;

    list<string> getDownstreamMessages() const override;

    void onMessagePublished(const string action) override;

private:
    PlaybackController(
        shared_ptr<vshl::common::interfaces::ILogger> logger);

    shared_ptr<vshl::common::interfaces::ILogger> mLogger;
};

}  // namespace playbackcontroller
}  // namespace capabilities
}  // namespace vshl

#endif  // VSHL_CAPABILITIES_PLAYBACKCONTROLLER_CAPABILITY_H_
