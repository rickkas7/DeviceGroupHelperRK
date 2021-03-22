#include "DeviceGroupHelperRK.h"

SerialLogHandler logHandler;

SYSTEM_THREAD(ENABLED);

PRODUCT_ID(7615);   // Change this to your product ID!
PRODUCT_VERSION(1);

void groupCallback(DeviceGroupHelper::NotificationType notificationType, const char *group);

void setup() {
    DeviceGroupHelper::instance()
        .withRetrievalModePeriodic(5min)
        .withNotifyCallback(groupCallback)
        .setup();   
}

void loop() {
    DeviceGroupHelper::instance().loop();
}


void groupCallback(DeviceGroupHelper::NotificationType notificationType, const char *group) {
    switch(notificationType) {
    case DeviceGroupHelper::NotificationType::UPDATED:
        Log.info("updated groups");
        break;

    case DeviceGroupHelper::NotificationType::ADDED:
        Log.info("added %s", group);
        break;

    case DeviceGroupHelper::NotificationType::REMOVED:
        Log.info("removed %s", group);
        break;
    }
}


/*
    auto groups = DeviceGroupHelper::instance().getGroups();
    for(auto it = groups.begin(); it != groups.end(); it++) {
        Log.info("group %s", (*it).c_str());
    }
*/