#include "server.h"

bool logout(const Database<User> &db, const std::string &usrName) {
    auto [user, found] = db.get(usrName);
    if (!found) return false;
    
    user.setStatus("0");
    return db.update(user);
}