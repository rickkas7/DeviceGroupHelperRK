#include "DeviceGroupHelperRK.h"

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

SYSTEM_THREAD(ENABLED);

PRODUCT_ID(7615);   // Change this to your product ID!
PRODUCT_VERSION(1);


void setup() {
    DeviceGroupHelper::instance()
        .withRetrievalModeAtStart()
        .setup();   
}

void loop() {
    DeviceGroupHelper::instance().loop();

    if (DeviceGroupHelper::instance().isInGroup("dev")) {
        static bool notified = false;
        if (!notified) {
            Log.info("is in group dev!");
            notified = true;
        }
    }
}

