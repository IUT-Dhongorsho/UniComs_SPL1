#include "server.h"
#include <string>

std::string login(
    const Database<User> &db,
    const std::string &usrName,
    const std::string &password)
{
    auto [user, found] = db.get(usrName);

    if (!found)
    {
        db.add(User({usrName, password, "active"}));
        std::cout << "Client not found. Registering as new user" << "\n";
        return "SUCCESS";
    }

    if (user.getPassword() == password)
    {
        if (user.getStatus() == "active")
        {
            return "Already logged in! Logout first to start new session";
        }
        user.setStatus("active");
        db.update(user);
        return "SUCCESS";
    }

    return "Invalid credentials";
}