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
#include "VshlSupportApi.h"

#include <list>

#include "afb/AFBApiImpl.h"
#include "afb/AFBRequestImpl.h"
#include "appmanagement/AppController.h"
#include "capabilities/CapabilitiesFactory.h"
#include "capabilities/CapabilityMessagingService.h"
#include "utilities/events/EventRouter.h"
#include "utilities/logging/Logger.h"

#include "json.hpp"

using namespace std;

CTLP_CAPI_REGISTER("vshlsupport-api");

static std::string TAG = "vshl::plugins::VshlSupportAPI";

static std::string VA_JSON_ATTR_DEFAULT = "default";
static std::string VA_JSON_ATTR_AGENTS = "agents";
static std::string VA_JSON_ATTR_ID = "id";
static std::string VA_JSON_ATTR_NAME = "name";
static std::string VA_JSON_ATTR_API = "api";
static std::string VA_JSON_ATTR_ACTIVE = "active";
static std::string VA_JSON_ATTR_WWS = "wakewords";
static std::string VA_JSON_ATTR_ACTIVE_WW = "activewakeword";
static std::string VA_JSON_ATTR_DESCRIPTION = "description";
static std::string VA_JSON_ATTR_VENDOR = "vendor";

static std::string STARTLISTENING_JSON_ATTR_REQUEST = "request_id";

static std::string EVENTS_JSON_ATTR_VA_ID = "va_id";
static std::string EVENTS_JSON_ATTR_EVENTS = "events";

static std::string CAPABILITIES_JSON_ATTR_ACTION = "action";
static std::string CAPABILITIES_JSON_ATTR_ACTIONS = "actions";
static std::string CAPABILITIES_JSON_ATTR_PAYLOAD = "payload";

static std::shared_ptr<vshl::utilities::logging::Logger> sLogger;
static std::shared_ptr<vshl::common::interfaces::IAFBApi> sAfbApi;
static std::shared_ptr<vshl::appmanagement::AppController> sAppController;
static std::unique_ptr<vshl::capabilities::CapabilitiesFactory> sCapabilitiesFactory;
static std::unique_ptr<vshl::capabilities::CapabilityMessagingService> sCapabilityMessagingService;
static std::unique_ptr<vshl::utilities::events::EventRouter> sEventRouter;

using json = nlohmann::json;
using Level = vshl::utilities::logging::Logger::Level;

CTLP_ONLOAD(plugin, ret) {
    if (plugin->api == nullptr) {
        return -1;
    }

    // Logger
    sLogger = vshl::utilities::logging::Logger::create(plugin->api);
    // sLogger->log(Level::INFO, TAG, "Vshl plugin loaded & initialized.");

    // AFB Wrapper
    sAfbApi = vshl::afb::AFBApiImpl::create(plugin->api);

    // AppController
    sAppController = vshl::appmanagement::AppController::create(sLogger, sAfbApi);
    if (!sAppController) {
        sLogger->log(Level::ERROR, TAG, "Failed to create AppController");
        return -1;
    }

    // EventRouter
    sEventRouter = vshl::utilities::events::EventRouter::create(sLogger);
    if (!sEventRouter) {
        sLogger->log(Level::ERROR, TAG, "Failed to create EventRouter");
        return -1;
    }

    sCapabilitiesFactory = vshl::capabilities::CapabilitiesFactory::create(sAppController, sLogger);
    if (!sCapabilitiesFactory) {
        sLogger->log(Level::ERROR, TAG, "Failed to create CapabilitiesFactory");
        return -1;
    }

    sCapabilityMessagingService = vshl::capabilities::CapabilityMessagingService::create(sLogger, sAfbApi);
    if (!sCapabilityMessagingService) {
        sLogger->log(Level::ERROR, TAG, "Failed to create CapabilityMessagingService");
        return -1;
    }

    return 0;
}


CTLP_CAPI(guiMetadataSubscribe, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> guMetadataCapability = sCapabilitiesFactory->getGuiMetadata();
    if (!guMetadataCapability) {
        sLogger->log(
            Level::WARNING,
            TAG,
            "guimetadataSubscribe: Failed to "
            "fetch guimetadata capability "
            "object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "guimetadataSubscribe: No arguments supplied.");
        return -1;
    }

    json subscribeJson = json::parse(json_object_to_json_string(eventJ));
    if (subscribeJson.find(CAPABILITIES_JSON_ATTR_ACTIONS) == subscribeJson.end()) {
        sLogger->log(Level::ERROR, TAG, "guimetadataSubscribe: No events array found in subscribe json");
        return -1;
    }
    list<string> events(subscribeJson[CAPABILITIES_JSON_ATTR_ACTIONS].get<list<string>>());

    // SUbscribe this client for the guimetadata events.
    auto request = vshl::afb::AFBRequestImpl::create(source->request);
    for (auto event : events) {
        if (!sCapabilityMessagingService->subscribe(*request, guMetadataCapability, event)) {
            sLogger->log(Level::ERROR, TAG, "guimetadataSubscribe: Failed to subscribe to event: " + event);
            return -1;
        }
    }

    AFB_ReqSuccess(
        source->request, json_object_new_string("Subscription to guimetadata events successfully completed."), NULL);
    return 0;
}

CTLP_CAPI(guiMetadataPublish, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> guMetadataCapability = sCapabilitiesFactory->getGuiMetadata();
    if (!guMetadataCapability) {
        sLogger->log(
            Level::WARNING,
            TAG,
            "guimetadataPublish: Failed to fetch "
            "guimetadata capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "guimetadataPublish: No arguments supplied.");
        return -1;
    }

    json_object* actionJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_ACTION.c_str());
    if (actionJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "guimetadataPublish: No action found in publish json");
        return -1;        
    }

    std::string action = std::string(json_object_get_string(actionJ));
    if (action.empty()) {
        sLogger->log(Level::ERROR, TAG, "guimetadataPublish: Invalid action input found in publish json");
        return -1;        
    }

    json_object* payloadJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_PAYLOAD.c_str());
    if (payloadJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "guimetadataPublish: No playload found in publish json");
        return -1;        
    }

    if (!sCapabilityMessagingService->publish(guMetadataCapability, action, payloadJ)) {
        sLogger->log(Level::ERROR, TAG, "guimetadataPublish: Failed to publish message: " + action);
        return -1;
    }

    AFB_ReqSuccess(source->request, json_object_new_string("Successfully published guimetadata messages."), NULL);
    return 0;
}

CTLP_CAPI(phonecontrolSubscribe, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> phoneControlCapability = sCapabilitiesFactory->getPhoneControl();
    if (!phoneControlCapability) {
        sLogger->log(Level::WARNING, TAG, "phoneControlSubscribe: Failed to fetch phone control capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "phoneControlSubscribe: No arguments supplied.");
        return -1;
    }

    json subscribeJson = json::parse(json_object_to_json_string(eventJ));
    if (subscribeJson.find(CAPABILITIES_JSON_ATTR_ACTIONS) == subscribeJson.end()) {
        sLogger->log(Level::ERROR, TAG, "phoneControlSubscribe: No events array found in subscribe json");
        return -1;
    }
    list<string> events(subscribeJson[CAPABILITIES_JSON_ATTR_ACTIONS].get<list<string>>());

    // SUbscribe this client for the phone call control events.
    auto request = vshl::afb::AFBRequestImpl::create(source->request);
    for (auto event : events) {
        if (!sCapabilityMessagingService->subscribe(*request, phoneControlCapability, event)) {
            sLogger->log(Level::ERROR, TAG, "phoneControlSubscribe: Failed to subscribe to event: " + event);
            return -1;
        }
    }

    AFB_ReqSuccess(
        source->request, json_object_new_string("Subscription to phone control events successfully completed."), NULL);
    return 0;
}

CTLP_CAPI(phonecontrolPublish, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> phoneControlCapability = sCapabilitiesFactory->getPhoneControl();
    if (!phoneControlCapability) {
        sLogger->log(Level::WARNING, TAG, "phoneControlPublish: Failed to fetch navigation capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "phoneControlPublish: No arguments supplied.");
        return -1;
    }

    json_object* actionJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_ACTION.c_str());
    if (actionJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "phoneControlPublish: No action found in publish json");
        return -1;        
    }

    std::string action = std::string(json_object_get_string(actionJ));
    if (action.empty()) {
        sLogger->log(Level::ERROR, TAG, "phoneControlPublish: Invalid action input found in publish json");
        return -1;        
    }

    json_object* payloadJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_PAYLOAD.c_str());
    if (payloadJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "phoneControlPublish: No playload found in publish json");
        return -1;        
    }

    if (!sCapabilityMessagingService->publish(phoneControlCapability, action, payloadJ)) {
        sLogger->log(Level::ERROR, TAG, "phoneControlPublish: Failed to publish message: " + action);
        return -1;
    }

    AFB_ReqSuccess(source->request, json_object_new_string("Successfully published phone control messages."), NULL);
    return 0;
}

CTLP_CAPI(navigationSubscribe, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> navigationCapability = sCapabilitiesFactory->getNavigation();
    if (!navigationCapability) {
        sLogger->log(Level::WARNING, TAG, "navigationSubscribe: Failed to fetch navigation capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "navigationSubscribe: No arguments supplied.");
        return -1;
    }

    json subscribeJson = json::parse(json_object_to_json_string(eventJ));
    if (subscribeJson.find(CAPABILITIES_JSON_ATTR_ACTIONS) == subscribeJson.end()) {
        sLogger->log(Level::ERROR, TAG, "navigationSubscribe: No events array found in subscribe json");
        return -1;
    }
    list<string> events(subscribeJson[CAPABILITIES_JSON_ATTR_ACTIONS].get<list<string>>());

    // SUbscribe this client for the navigation events.
    auto request = vshl::afb::AFBRequestImpl::create(source->request);
    for (auto event : events) {
        if (!sCapabilityMessagingService->subscribe(*request, navigationCapability, event)) {
            sLogger->log(Level::ERROR, TAG, "navigationSubscribe: Failed to subscribe to event: " + event);
            return -1;
        }
    }

    AFB_ReqSuccess(
        source->request, json_object_new_string("Subscription to navigation events successfully completed."), NULL);
    return 0;
}

CTLP_CAPI(navigationPublish, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> navigationCapability = sCapabilitiesFactory->getNavigation();
    if (!navigationCapability) {
        sLogger->log(Level::WARNING, TAG, "navigationPublish: Failed to fetch navigation capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "navigationPublish: No arguments supplied.");
        return -1;
    }

    json_object* actionJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_ACTION.c_str());
    if (actionJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "navigationPublish: No action found in publish json");
        return -1;        
    }

    std::string action = std::string(json_object_get_string(actionJ));
    if (action.empty()) {
        sLogger->log(Level::ERROR, TAG, "navigationPublish: Invalid action input found in publish json");
        return -1;        
    }

    json_object* payloadJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_PAYLOAD.c_str());
    if (payloadJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "navigationPublish: No playload found in publish json");
        return -1;        
    }

    if (!sCapabilityMessagingService->publish(navigationCapability, action, payloadJ)) {
        sLogger->log(Level::ERROR, TAG, "navigationPublish: Failed to publish message: " + action);
        return -1;
    }

    AFB_ReqSuccess(source->request, json_object_new_string("Successfully published navigation messages."), NULL);
    return 0;
}

CTLP_CAPI(playbackControllerSubscribe, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> playbackcontrollerCapability = sCapabilitiesFactory->getPlaybackController();
    if (!playbackcontrollerCapability) {
        sLogger->log(Level::WARNING, TAG, "playbackControllerSubscribe: Failed to fetch playbackcontroller capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "playbackControllerSubscribe: No arguments supplied.");
        return -1;
    }

    json subscribeJson = json::parse(json_object_to_json_string(eventJ));
    if (subscribeJson.find(CAPABILITIES_JSON_ATTR_ACTIONS) == subscribeJson.end()) {
        sLogger->log(Level::ERROR, TAG, "playbackControllerSubscribe: No events array found in subscribe json");
        return -1;
    }
    list<string> events(subscribeJson[CAPABILITIES_JSON_ATTR_ACTIONS].get<list<string>>());

    // SUbscribe this client for the navigation events.
    auto request = vshl::afb::AFBRequestImpl::create(source->request);
    for (auto event : events) {
        if (!sCapabilityMessagingService->subscribe(*request, playbackcontrollerCapability, event)) {
            sLogger->log(Level::ERROR, TAG, "playbackControllerSubscribe: Failed to subscribe to event: " + event);
            return -1;
        }
    }

    AFB_ReqSuccess(
        source->request, json_object_new_string("Subscription to playbackcontroller events successfully completed."), NULL);
    return 0;
}

CTLP_CAPI(playbackControllerPublish, source, argsJ, eventJ) {
    if (sCapabilitiesFactory == nullptr || sCapabilityMessagingService == nullptr) {
        return -1;
    }

    shared_ptr<vshl::common::interfaces::ICapability> playbackcontrollerCapability = sCapabilitiesFactory->getPlaybackController();
    if (!playbackcontrollerCapability) {
        sLogger->log(Level::WARNING, TAG, "playbackControllerPublish: Failed to fetch playbackcontroller capability object.");
        return -1;
    }

    if (eventJ == nullptr) {
        sLogger->log(Level::WARNING, TAG, "playbackControllerPublish: No arguments supplied.");
        return -1;
    }

    json_object* actionJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_ACTION.c_str());
    if (actionJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "playbackControllerPublish: No action found in publish json");
        return -1;        
    }

    std::string action = std::string(json_object_get_string(actionJ));
    if (action.empty()) {
        sLogger->log(Level::ERROR, TAG, "playbackControllerPublish: Invalid action input found in publish json");
        return -1;        
    }

    json_object* payloadJ = json_object_object_get(eventJ , CAPABILITIES_JSON_ATTR_PAYLOAD.c_str());
    if (payloadJ == nullptr) {
        sLogger->log(Level::ERROR, TAG, "playbackControllerPublish: No playload found in publish json");
        return -1;        
    }

    if (!sCapabilityMessagingService->publish(playbackcontrollerCapability, action, payloadJ)) {
        sLogger->log(Level::ERROR, TAG, "playbackControllerPublish: Failed to publish message: " + action);
        return -1;
    }
     
    AFB_ReqSuccess(source->request, json_object_new_string("Successfully published playbackcontroller messages."), NULL);
    return 0;
}