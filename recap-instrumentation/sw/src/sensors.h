#ifndef RECAP_INSTRUMENT_SENSORS_H
#define RECAP_INSTRUMENT_SENSORS_H

#include <sdp810.h>
#include <sfm3xxx.h>

#define SENSORS_SAMPLE_RATE_KHZ 20

struct sensors_sample {
    bool audio_only;

    uint32_t millis;
    sfm3xxx_flow_t flow;
    sdp810_pressure_t pressure;
    uint16_t ear;
    uint16_t mask;
    bool event;
    int32_t h2s;
    int8_t h2s_temp;
    uint8_t h2s_humidity;
};

// Blocks until sensors are ready
void sensors_init(void);

void sensors_start_recording(void);
void sensors_stop_recording(void);

// Return true if data contains sample, false on timeout
bool sensors_read(struct sensors_sample *data, int timeout);

void sensors_clear_read_buffer(void);

#endif // RECAP_INSTRUMENT_SENSORS_H
