#ifndef FIREMAIL_STATE_H
#define FIREMAIL_STATE_H

struct firemail_connection_state_t {
    uint8_t stage;
    uint8_t sent;
    char target[40];
    char outbuf[40];
    uint8_t processing;
};


#endif /* FIREMAIL_STATE_H */
