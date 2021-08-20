#include "DeviceGroupHelperRK.h"

SerialLogHandler logHandler;

SYSTEM_THREAD(ENABLED);

PRODUCT_ID(7615);   // Change this to your product ID!
PRODUCT_VERSION(1);

void groupCallback(DeviceGroupHelper::NotificationType notificationType, const char *group);

void setup() {
    waitFor(Serial.isConnected, 10000);

    DeviceGroupHelper::instance()
        .withRetrievalModeAtStart()
        .withNotifyCallback(groupCallback)
        .setup();   
}

void loop() {
    DeviceGroupHelper::instance().loop();
}


void groupCallback(DeviceGroupHelper::NotificationType notificationType, const char *group) {
    if (notificationType == DeviceGroupHelper::NotificationType::UPDATED) {
        Log.info("deviceName=%s", DeviceGroupHelper::instance().getDeviceName());
        Log.info("deviceNotes=%s", DeviceGroupHelper::instance().getDeviceNotes());
        Log.info("productId=%d", DeviceGroupHelper::instance().getProductId());
        Log.info("development=%d", DeviceGroupHelper::instance().getIsDevelopment());
    }
}

/*
    auto groups = DeviceGroupHelper::instance().getGroups();
    for(auto it = groups.begin(); it != groups.end(); it++) {
        Log.info("group %s", (*it).c_str());
    }
*/

/*
Example output

0000010426 [app] INFO: deviceName=test3
0000010426 [app] INFO: deviceNotes=
0000010427 [app] INFO: productId=7615
0000010427 [app] INFO: development=1

*/
