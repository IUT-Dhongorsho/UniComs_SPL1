#include "Database.hpp"
#include "Schema.hpp"
#include <iostream>

/* ---------------- User Schema ---------------- */

/* ---------------- Main ---------------- */

int main()
{
    Database db;
    db.create<User>("User");
    struct User user1;
    user1.id = "12";
    user1.name = "Nafis Ahnaf Jamil";
    user1.email = "nafisahnaf@iut-dhaka.edu";
    user1.password = "Nafis@123";
    user1.isDeleted = false;
    user1.isActive = true;
    // std::cout << "Hello World";
    db.insert<User>(user1);
    return 0;
}
