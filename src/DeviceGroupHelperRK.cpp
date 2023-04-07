#include "DeviceGroupHelperRK.h"

static Logger _log("app.devgrp");

DeviceGroupHelper *DeviceGroupHelper::_instance = 0;

// [static] 
DeviceGroupHelper &DeviceGroupHelper::instance() {
    if (!_instance) {
        _instance = new DeviceGroupHelper();
    }
    return *_instance;
}


void DeviceGroupHelper::setup() {
    String subscriptionName = System.deviceID() + String("/hook-response/") + eventName;
    Particle.subscribe(subscriptionName, &DeviceGroupHelper::subscriptionHandler, this);

    if (retrievalMode == RetrievalMode::AT_START || retrievalMode == RetrievalMode::PERIODIC) {
        stateHandler = &DeviceGroupHelper::stateWaitConnected;
    }

    if (functionName.length() > 0) {
        Particle.function(functionName, &DeviceGroupHelper::functionHandler, this);
    }
}

void DeviceGroupHelper::loop() {
    if (stateHandler) {
        stateHandler(*this);
    }
}

void DeviceGroupHelper::update() {
    if (isIdle) {
        stateHandler = &DeviceGroupHelper::stateWaitConnected;
    }
}


DeviceGroupHelper::DeviceGroupHelper() {

}

DeviceGroupHelper::~DeviceGroupHelper() {

}

void DeviceGroupHelper::stateWaitConnected() {
    if (!Particle.connected()) {
        return;
    }

    isIdle = false;

    Particle.publish(eventName, "");
    groupUpdateTime = 0;
    stateTime = millis();
    stateHandler = &DeviceGroupHelper::stateWaitResponse;
}

void DeviceGroupHelper::stateWaitResponse() {
    if (groupUpdateTime) {
        isIdle = true;
        stateTime = millis();
        stateHandler = &DeviceGroupHelper::stateWaitPeriodic;
        return;
    }
    if (millis() - stateTime >= groupResponseTimeout.count()) {
        // Timeout
        isIdle = true;
        stateTime = millis();
        stateHandler = &DeviceGroupHelper::stateWaitRetry;
        return;
    }
}


void DeviceGroupHelper::stateWaitPeriodic() {
    if (retrievalMode != RetrievalMode::PERIODIC || periodicTimeMs == 0) {
        stateHandler = 0;
        return;
    }

    if (millis() - stateTime >= periodicTimeMs) {
        stateHandler = &DeviceGroupHelper::stateWaitConnected;
    }
}


void DeviceGroupHelper::stateWaitRetry() {

    if (millis() - stateTime >= retryTimeout.count()) {
        // Retry time completed, try to get values again
        stateHandler = &DeviceGroupHelper::stateWaitConnected;
    }
}


int DeviceGroupHelper::functionHandler(String cmd) {
    subscriptionHandler("", cmd.c_str());
    return 0;
}

void DeviceGroupHelper::subscriptionHandler(const char *event, const char *data) {
//    _log.info("event %s data %s", event, data);

    std::unordered_set<std::string> newGroups;

    JSONValue outerObj = JSONValue::parseCopy(data);

    JSONObjectIterator iter(outerObj);
    while(iter.next()) {
        if (iter.name() == "groups") {
            JSONArrayIterator iter2(iter.value());
            for(size_t ii = 0; iter2.next(); ii++) {
                const char *group = iter2.value().toString().data();
                newGroups.emplace(group);
                _log.trace("in group %s", group);
            }
        }
        else 
        if (iter.name() == "name") {
            name = iter.value().toString().data();
        }
        else 
        if (iter.name() == "product_id") {
            product_id = iter.value().toInt();
        }
        else 
        if (iter.name() == "notes") {
            notes = iter.value().toString().data();
        }
        else 
        if (iter.name() == "development") {
            development = iter.value().toBool();
        }
    }

    if (notifyCallback) {
        for(auto it = groups.begin(); it != groups.end(); it++) {
            std::string curGroup(*it);

            if (newGroups.count(curGroup) == 0) {
                notifyCallback(NotificationType::REMOVED, curGroup.data());
            }
        }
        for(auto it = newGroups.begin(); it != newGroups.end(); it++) {
            std::string curGroup(*it);

            if (groups.count(curGroup) == 0) {
                notifyCallback(NotificationType::ADDED, curGroup.data());
            }
        }
    }

    groups = newGroups;
    groupUpdateTime = millis();
    _log.trace("updated groups");

    if (notifyCallback) {
        notifyCallback(NotificationType::UPDATED, NULL);
    }
}
