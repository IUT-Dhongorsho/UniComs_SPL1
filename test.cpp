#include "src/server/server.h"
#include <iostream>

int main() {
    Database db;
    
    std::string result1 = login(db, "testuser", "pass123");
    std::cout << "Test 1 (Register): " << result1 << std::endl;
    
    std::string result2 = login(db, "testuser", "pass123");
    std::cout << "Test 2 (Already logged in): " << result2 << std::endl;
    
    bool result3 = logout(db, "testuser");
    std::cout << "Test 3 (Logout): " << (result3 ? "true" : "false") << std::endl;
    
    std::string result4 = login(db, "testuser", "pass123");
    std::cout << "Test 4 (Login again): " << result4 << std::endl;
    
    return 0;
}