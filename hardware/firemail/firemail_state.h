#ifndef FIREMAIL_STATE_H
#define FIREMAIL_STATE_H

struct firemail_connection_state_t {
    uint8_t stage;
    uint8_t sent;
    char outbuf[40];
    //uint8_t processing; /* interrupt must read that */
};


#endif /* FIREMAIL_STATE_H */
