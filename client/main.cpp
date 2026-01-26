#include "core/Client.h"

int main() {
    Client client("127.0.0.1", 5000);

    if (!client.connectToServer()) return 1;
    client.start();

    return 0;
}
