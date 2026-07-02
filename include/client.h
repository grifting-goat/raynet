#ifndef CLIENT_H
#define CLIENT_H


#include "disp.h"
#include "render.h"

typedef struct {
    Display disp;
    Camera cam;

} Client;


void client_init(Client* c);

void client_run(Client* c);

void client_close(Client* c);


#endif // CLIENT_H
