#ifndef FIREMAIL_STATE_H
#define FIREMAIL_STATE_H

enum {
    FM_INIT,
    FM_OPEN_STREAM,
    FM_CONNECTED,
};

struct firemail_connection_state_t {
    uint8_t stage;
    uint8_t sent;
    char target[40];
    char outbuf[40];
};

#endif /* FIREMAIL_STATE_H */
