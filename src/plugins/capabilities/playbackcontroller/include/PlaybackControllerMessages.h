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
#ifndef VSHL_CAPABILITIES_PLAYBACKCONTROLLER_MESSAGES_H_
#define VSHL_CAPABILITIES_PLAYBACKCONTROLLER_MESSAGES_H_

#include <list>
#include <string>

using namespace std;

namespace vshl {
namespace capabilities {
namespace playbackcontroller {

static string NAME = "playbackcontroller";

/*
 ******************************************************************************************************
 *  Supported actions from APPS -> VA
 ******************************************************************************************************
 */
// List of actions that are delivered from VA -> Apps
static list<string> PLAYBACKCONTROLLER_UPSTREAM_ACTIONS = {

};


/*
 ******************************************************************************************************
 *  Supported actions from VA -> APPS
 ******************************************************************************************************
 */
// List of actions that are delivered from Apps -> VA
/* 
 *  payload :
 *  {
 *    "button" : "{{STRING}}" // play, pause, next, previous, skip-forward, skip-backward
 *  }
 * @param [in] button Button pressed by the user.
 * 
 */
static string PLAYBACKCONTROLLER_BUTTONPRESSED = "button_pressed";

static list<string> PLAYBACKCONTROLLER_DOWNSTREAM_ACTIONS = {
    PLAYBACKCONTROLLER_BUTTONPRESSED
};

}  // namespace playbackcontroller
}  // namespace capabilities
}  // namespace vshl

#endif  // VSHL_CAPABILITIES_PLAYBACKCONTROLLER_MESSAGES_H_
