#ifndef __DEVICEGROUPHELPERRK_H
#define __DEVICEGROUPHELPERRK_H

// Github repository: https://github.com/rickkas7/DeviceGroupHelperRK
// License: MIT

#include "Particle.h"

#include <unordered_set>

/**
 * @brief Helper class to use device groups on a Particle device
 * 
 * This is useful when you have a product and are using the device groups feature. Normally, use
 * this to group related devices and control firmware releases. However, you can use this technique
 * to read the device groups on-device, which would allow you to make decisions in device firmware
 * based on group membership.
 * 
 * You can choose when to update groups (manually, at startup, or periodically).
 * 
 * You can then either query whether the device is in a specific group using the previously cached
 * group list. This is fast and does not require network access so you can use it in your code liberally.
 * 
 * Or, if you prefer, you can register a notification function that will call your function with
 * an indication that the list was updated, and when individual groups are added or removed from the
 * previous retrieval.
 */
class DeviceGroupHelper {
public:
    /**
     * @brief How often to retrieve device groups
     */
    enum class RetrievalMode {
        MANUAL,     //!< Manually (default)
        AT_START,   //!< At startup (once)
        PERIODIC    //!< At startup, then periodically thereafter
    };

    /**
     * @brief Used for the notify callback to specify what is being notified of
     */
    enum class NotificationType {
        UPDATED,    //!< The groups were updated. Use getGroups() to get a set of all group names.
        ADDED,      //!< This group was added.
        REMOVED     //!< This group was removed.
    };

    /**
     * @brief Get the singleton instance of this class
     */
    static DeviceGroupHelper &instance();

    /**
     * @brief Specify the name of the event used to get the device group. Must match integration.
     * 
     * Default is "G52ES20Q_DeviceGroup".
     */
    DeviceGroupHelper &withEventName(const char *eventName) { this->eventName = eventName; return *this; };

    /**
     * @brief Gets the event name to use for the webhook
     */
    const char *getEventName() const { return eventName; };

    /**
     * @brief Sets manual retrieval mode. This is the default mode. 
     * 
     * Normally called before setup() but you can change it later using this method for example
     * if you want to stop periodically checking at runtime.
     */
    DeviceGroupHelper &withRetrievalModeManual() { return withRetrievalMode(RetrievalMode::MANUAL); };

    /**
     * @brief Sets retrieve groups at start mode. This should be done before setup().
     */
    DeviceGroupHelper &withRetrievalModeAtStart() { return withRetrievalMode(RetrievalMode::AT_START); };

    /**
     * @brief Sets retrieve groups at start mode and periodically mode. This should be done before setup().
     * 
     * @param ms The number of milliseconds between retrievals.
     * 
     * You don't want to set this to be too short as each retrieve uses two data operations.
     */
    DeviceGroupHelper &withRetrievalModePeriodic(unsigned long ms) { periodicTimeMs = ms; return withRetrievalMode(RetrievalMode::PERIODIC); };

    /**
     * @brief Sets retrieve groups at start mode and periodically mode. This should be done before setup().
     * 
     * @param chronoLiteral Time between retrievals as a chrono literal, for example 15min or 24h.
     * 
     * You don't want to set this to be too short as each retrieve uses two data operations.
     */
    DeviceGroupHelper &withRetrievalModePeriodic(std::chrono::milliseconds chronoLiteral) { return withRetrievalModePeriodic(chronoLiteral.count()); };

    /**
     * @brief Sets the retrieve mode using RetrievalMode value
     */
    DeviceGroupHelper &withRetrievalMode(RetrievalMode retrievalMode) { this->retrievalMode = retrievalMode; return *this; };

    /**
     * @brief Gets the current retrival mode (how often to fetch group membership)
     */
    RetrievalMode getRetrievalMode() const { return retrievalMode; };

    /**
     * @brief Sets the periodic time to check when in RetrivalMode::PERIODIC
     * 
     * @param ms Check period in milliseconds
     * 
     * Using withRetrievalModePeriodic() may be more convenient than using withRetrievalMode(RetrivalMode::PERIODIC)
     * and withPeriodicTimeMs().
     */
    DeviceGroupHelper &withPeriodicTimeMs(unsigned long ms) { periodicTimeMs = ms; return *this; };

    /**
     * @brief Gets the periodic time to check in milliseconds
     */
    unsigned long getPeriodicTimeMs() const { return periodicTimeMs; };

    /**
     * @brief Sets a function to be called when the group list is updated.
     * 
     * @param notifyCallback The function to call
     * 
     * You can only have one notifyCallback. To remove it, pass NULL.
     * 
     * The function has the prototype:
     * 
     * ```
     * void callback(NotificationType notificationType, const char *group)
     * ```
     * 
     * The notificationTypes is one of the constants:
     * 
     * - NotificationType::ADDED A group was added to this device
     * - NotificationType::REMOVED A group was removed from this device 
     * - NotificationType::UPDATED The list was updated
     * 
     * The UPDATED notification is made after ADDED and REMOVED, and occurs
     * even if no changes occurred. You can use getGroups or isInGroup() in
     * the UPDATED callback. The group parameter is always NULL for the UPDATED
     * notification.
     * 
     * The notifyCallback parameter is a std::function so it can also be a C++11 lambda
     * instead of a plain function. This is an easy way to make the callback a member
     * function of your class instead of a static member function.
     */
    DeviceGroupHelper &withNotifyCallback(std::function<void(NotificationType, const char *)> notifyCallback) { this->notifyCallback = notifyCallback; return *this; };


    /**
     * @brief You must call setup() from global application setup()!
     * 
     * It's normally done like:
     * 
     * ```
     * DeviceGroupHelper::instance().setup();
     * ```
     * 
     * Failure to call setup will prevent the library from doing anything.
     */
    void setup();

    /**
     * @brief You must call loop() from global application loop()!
     * 
     * It's normally done like:
     * 
     * ```cpp
     * DeviceGroupHelper::instance().loop();
     * ```
     * 
     * Failure to call loop will prevent the library from doing anything.
     */
    void loop();

    /**
     * @brief Gets the set of all groups the device current belongs to
     * 
     * This uses the previously retrieved list. This is fast and does not use the network.
     * 
     * To iterate the group list, you can use something like this:
     * 
     * ```cpp
     * auto groups = DeviceGroupHelper::instance().getGroups();
     * for(auto it = groups.begin(); it != groups.end(); it++) {
     *     Log.info("group %s", (*it).c_str());
     * }
     * ```
     */
    const std::unordered_set<std::string> getGroups() const { return groups; };

    /**
     * @brief Returns true if the group list has been retrieved
     */
    bool retrievedGroups() const { return (groupUpdateTime != 0); };

    /**
     * @brief Returns true if this device belongs to the specified group
     * 
     * @param group The group name to check.
     * 
     * You can use the notifyCallback to be notified of group changes instead of 
     * polling for it using this function. 
     * 
     * This uses the previously retrieved list. This is fast and does not use the network.
     */
    bool isInGroup(const char *group) const { return groups.count(group) == 1; };

    /**
     * @brief Requests an update of the group list
     * 
     * If the retrieval mode is PERIODIC, then this retrieves now and shifts the next
     * check to be periodicTimeMs from now instead of the originally scheduled time.
     * 
     * Updates usually only take a second or so, but if there is poor network connectivity
     * it could take longer, up to 30 seconds. It could also fail. 
     * 
     * retrievedGroups() will return false immediately after calling update() and then
     * will become try again if the operation succeeds.
     * 
     * If the notification callback is used, that will notify of any changes in group
     * membership.
     */
    void update();


    /**
     * @brief Get the device Name (if sent by the webhook)
     */
    const char *getDeviceName() { return name.c_str(); };

    /**
     * @brief Get the Product ID (if sent by the webhook)
     */
    int getProductId() const { return product_id; };

    /**
     * @brief Device Notes (if sent by the webhook)
     */
    const char *getDeviceNotes() const { return notes.c_str(); };

    /**
     * @brief Developent device (if sent by the webhook)
     */
    bool getIsDevelopment() const { return development; };

protected:
    /**
     * @brief Constructor (protected)
     * 
     * You never construct one of these - use the singleton instance using `DeviceGroupHelper::instance()`.
     */
    DeviceGroupHelper();

    /**
     * @brief Destructor - never used
     * 
     * The singleton cannot be deleted.
     */
    virtual ~DeviceGroupHelper();

    /**
     * @brief This class is not copyable
     */
    DeviceGroupHelper(const DeviceGroupHelper&) = delete;

    /**
     * @brief This class is not copyable
     */
    DeviceGroupHelper& operator=(const DeviceGroupHelper&) = delete;

    /**
     * @brief State handler, waits for Particle.connected() then publishes the request event.
     * 
     * Previous state: 0, stateWaitRetry, stateWaitPeriodic
     * Next state: stateWaitResponse
     */
    void stateWaitConnected();

    /**
     * @brief State handler, waits for the subscription handler to be called.
     * 
     * Previous state: stateWaitConnected
     * Next state: stateWaitRetry, stateWaitPeriodic 
     */
    void stateWaitResponse();

    /**
     * @brief State handler to wait for the next periodic check, or go into idle state (0)
     * 
     * Previous state: stateWaitResponse
     * Next state: 0 or stateWaitConnected
     */
    void stateWaitPeriodic();

    /**
     * @brief State handler for waiting to retry after the subscribe handler was not called
     * 
     * Previous state: stateWaitResponse
     * Next state: stateWaitConnected
     */
    void stateWaitRetry();

    /**
     * @brief Subscription handler to receive webhook responses
     */
    void subscriptionHandler(const char *event, const char *data);

    /**
     * @brief Event name, set with withEventName(). This must match the webhook.
     */
    String eventName = "G52ES20Q_DeviceGroup";

    /**
     * @brief How often to retrieve the group membership data
     */
    RetrievalMode retrievalMode = RetrievalMode::MANUAL;

    /**
     * @brief When in RetrievalMode::PERIODIC, how often to check in milliseconds
     */
    unsigned long periodicTimeMs = 0;

    /**
     * @brief State handler function. Can be NULL.
     */
    std::function<void(DeviceGroupHelper&)> stateHandler = 0;

    /**
     * @brief Millis value for timing in some states
     */
    unsigned long stateTime = 0;

    /**
     * @brief Groups this device belongs to
     */
    std::unordered_set<std::string> groups;

    /**
     * @brief Device Name (if sent by the webhook)
     */
    String name;

    /**
     * @brief Product ID (if sent by the webhook)
     */
    int product_id;

    /**
     * @brief Device Notes (if sent by the webhook)
     */
    String notes;

    /**
     * @brief Developent device (if sent by the webhook)
     */
    bool development = false;

    /**
     * @brief Millis value when the group list was last updated, or 0 for not updated.
     */
    unsigned long groupUpdateTime = 0;

    /**
     * @brief stateHandler is idle and update() can push it into stateWaitConnected to update list
     */
    bool isIdle = true;

    /**
     * @brief How long to wait for the webhook response
     */
    std::chrono::milliseconds groupResponseTimeout = 30s;

    /**
     * @brief How long to wait to retry after a failed webhook response
     */
    std::chrono::milliseconds retryTimeout = 2min;

    /**
     * @brief Callback function for when group membership is updated.
     */
    std::function<void(NotificationType, const char *)> notifyCallback = 0;

    /**
     * @brief Singleton instance of this class
     */
    static DeviceGroupHelper *_instance;
};

#endif /* __DEVICEGROUPHELPERRK_H */
