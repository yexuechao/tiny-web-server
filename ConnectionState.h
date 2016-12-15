//
// Created by yxc on 16-11-29.
//

#ifndef WEBSERVER_CONNECTIONSTATE_H
#define WEBSERVER_CONNECTIONSTATE_H

enum connection_state_t{
    CON_STATE_CONNECT,

    CON_STATE_REQUEST_START,

    CON_STATE_READ,

    CON_STATE_REQUEST_END,

    CON_STATE_READ_POST,

    CON_STATE_HANDLE_REQUEST,

    CON_STATE_RESPONSE_START,

    CON_STATE_WRITE,

    CON_STATE_RESPONSE_END,

    CON_STATE_ERROR,

    CON_STATE_CLOSE

};

#endif //WEBSERVER_CONNECTIONSTATE_H
