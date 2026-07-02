#include "client.h"


void client_init(Client* c) {

    init_camera(&c->cam, 103.0, 20, 1280);

}

void client_run(Client* c) {

}

void client_close(Client* c);