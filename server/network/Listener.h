#ifndef LISTENER_H
#define LISTENER_H

class Listener {
public:
    explicit Listener(int port);
    int acceptClient();

private:
    int port;
    int serverFd;
};

#endif
