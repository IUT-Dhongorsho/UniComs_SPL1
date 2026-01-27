#include "DatabaseUtils.h"
#include "User.h"
#include "Database.h"

namespace fs = std::filesystem;

namespace DatabaseUtils {
    
bool initializeDatabase(const std::string &filename, 
                       const std::string &adminName,
                       const std::string &adminPassword) {
    Database<User> db(filename);
    
    // Check if database is empty
    if (db.countActive() == 0) {
        // Create admin user
        User admin({adminName, adminPassword, "active"});
        
        if (db.add(admin) == 0) {
            return true;
        }
    }
    
    return false;
}

bool backupDatabase(const std::string &sourceFile, 
                   const std::string &backupDir) {
    try {
        // Create backup directory if it doesn't exist
        if (!fs::exists(backupDir)) {
            fs::create_directories(backupDir);
        }
        
        // Generate timestamp
        std::time_t now = std::time(nullptr);
        std::tm tm = *std::localtime(&now);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y%m%d_%H%M%S");
        
        // Create backup filename
        std::string backupFile = backupDir + "/backup_" + ss.str() + ".db";
        
        // Copy file
        fs::copy_file(sourceFile, backupFile, fs::copy_options::overwrite_existing);
        
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

bool restoreDatabase(const std::string &backupFile, 
                    const std::string &targetFile) {
    try {
        if (!fs::exists(backupFile)) {
            return false;
        }
        
        // Copy backup to target
        fs::copy_file(backupFile, targetFile, fs::copy_options::overwrite_existing);
        
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

bool validateCredentials(const std::string &filename,
                        const std::string &username,
                        const std::string &password) {
    Database<User> db(filename);
    
    auto [user, found] = db.get(username);
    if (!found) return false;
    
    return user.getPassword() == password && user.getStatus() != "deleted";
}

bool changePassword(const std::string &filename,
                   const std::string &username,
                   const std::string &oldPassword,
                   const std::string &newPassword) {
    Database<User> db(filename);
    
    auto [user, found] = db.get(username);
    if (!found) return false;
    
    // Verify old password
    if (user.getPassword() != oldPassword) {
        return false;
    }
    
    // Update password
    user.setPassword(newPassword);
    
    return db.update(user);
}

}