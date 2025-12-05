# MQTTComm

MQTTComm manages a 3-topic contract:
- `cfg`: incoming configuration (JSON) → your callback handles it.
- `cmd`: incoming commands (JSON) → your callback handles them.
- `data`: outgoing telemetry (JSON) → your callback composes it.

Main (or another coordinator) provides the callbacks; MQTTComm builds topics and client ID from Kconfig.

## Kconfig options
- Broker:
  - `BROKER_URL` (string), `BROKER_URL_FROM_STDIN` (dev helper).
- Topic segments:
  - `MQTT_TOPIC_ORGANIZATION` (default `cBots`)
  - `MQTT_TOPIC_APIVERSION` (default `v1`)
  - `MQTT_TOPIC_SITE` (default `MyHome`)
  - Device segment:
    - `MQTT_TOPIC_DEVICE_USE_WIFI_MAC` (default on): use Wi-Fi MAC (no separators).
    - `MQTT_TOPIC_DEVICE_NAME`: device name when MAC is not used.
- Client ID:
  - `MQTT_CLIENT_ID_USE_DEVICE` (default on): client ID = device segment.
  - `MQTT_CLIENT_ID_CUSTOM`: free-form client ID when the above is off (empty falls back to device).
- Threading:
  - `MQTTCOMM_USE_THREAD`, `MQTTCOMM_PERIOD_MS`, `MQTTCOMM_MINIMIZE_JITTER`, `MQTTCOMM_PIN_CORE`, `MQTTCOMM_TASK_STACK`, `MQTTCOMM_TASK_PRIO`.

Derived topics:
- `cfg`: `<org>/<api>/<site>/<device>/cfg`
- `cmd`: `<org>/<api>/<site>/<device>/cmd`
- `data`: `<org>/<api>/<site>/<device>/data`

## API (simplified)
- `MQTTComm_setup(mqtt_comm_cfg_t *cfg)` — register callbacks, build topics/client ID from Kconfig, connect to broker.
- `MQTTComm_enable/disable()` — gate publishing/handling.
- Threaded mode (`CONFIG_MQTTCOMM_USE_THREAD=y`):
  - `MQTTComm_start/stop()` — manage the internal task that calls `MQTTComm_spin()`.
  - `MQTTComm_set_period_ms()` — adjust publish period at runtime.
- Spin mode (`CONFIG_MQTTCOMM_USE_THREAD=n`):
  - Call `MQTTComm_spin()` from your scheduler.

### Callbacks
```c
typedef struct {
  void (*f_cfg_cb)(const char *payload, int len);
  void (*f_req_cb)(const char *payload, int len);
  void (*f_data_cb)(char *out, int *len);
} mqtt_comm_cfg_t;
```
- `f_cfg_cb`: handle config JSON from `cfg` topic.
- `f_req_cb`: handle command JSON from `cmd` topic.
- `f_data_cb`: produce telemetry JSON for `data` topic (`out` buffer, write length to `len`).

## Usage pattern
1) Provide callbacks (usually in `main`) and call `MQTTComm_setup(&cfg)` with the function pointers filled.
2) If threaded mode is off, call `MQTTComm_enable()` and then periodically `MQTTComm_spin()` from your scheduler.
   If threaded mode is on, call `MQTTComm_enable()` and `MQTTComm_start()`; the component will publish on its own cadence.
3) Inspect computed topics/client ID at runtime (e.g., `MQTTComm_dre.*`).

## Notes
- When using MAC-based device IDs, MQTTComm reads the Wi-Fi MAC. If it cannot, it falls back to `MQTT_TOPIC_DEVICE_NAME`.
- The component only builds/publishes JSON; each project decides how to merge component payloads inside the callbacks.
