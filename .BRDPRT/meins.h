#ifndef MEINS_H
#define MEINS_H

void send_ranging(void);
void receive_ranging(void);
void waitforsysstatus(uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask);
uint64_t timestamp_to_64bit(uint8_t *timestamp);

#endif