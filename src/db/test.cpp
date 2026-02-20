#include "Database.hpp"
#include "Schema.hpp"
#include "./models/User.hpp"

int main()
{
    Database db;
    db.init("src/db/ddl.csv");

    // Create User table
    db.create<User>();

    // Insert a user
    User u{"1", "Alice", "alice@example.com", "pass123", true, false};
    db.insert(u);

    // Query user
    auto result = db.query<User>("id", "1");
    if (result.has_value())
        std::cout << "Found user: " << result->name << std::endl;

    return 0;
}
