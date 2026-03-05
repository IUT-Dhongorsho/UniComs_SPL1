#include "server.h"

bool logout(Database &db, const std::string &usrName) {
    auto result = db.query<User>("username", usrName);
    if (!result.has_value()) return false;
    
    User user = result.value();
    user.isActive = "false";
    db.update<User>(user.id, user);
    return true;
}