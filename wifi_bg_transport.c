/* wifi_bg_transport.c
   ------------------------------------------------------------
   Stub for sending Morse/56k pulse packets over a high-latency
   802.11b/g-style link (e.g., long-range, satellite gateway).
*/

#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t *payload;
    uint32_t length;
} PulsePacket;

/* Prepare a packet from compressed PCF data */
PulsePacket make_pulse_packet(const uint8_t *data, uint32_t len) {
    PulsePacket p;
    p.payload = (uint8_t *)data;
    p.length  = len;
    return p;
}

/* Placeholder: send over wireless b/g (or satellite-backed link) */
void send_pulse_packet_over_wifi_bg(const PulsePacket *p) {
    (void)p;
    /* Real implementation:
       - wrap in UDP/TCP
       - send via WiFi driver
       - tolerate high RTT (hundreds of ms)
    */
}
