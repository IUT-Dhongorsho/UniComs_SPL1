#ifndef SRC_DB_DATABASE_UTILS_H_
#define SRC_DB_DATABASE_UTILS_H_

#include <string>
#include <vector>
#include "User.h"

// Utility functions for database operations
namespace DatabaseUtils {
  // Initialize database with admin user if empty
  bool initializeDatabase(const std::string &filename, 
                         const std::string &adminName = "admin",
                         const std::string &adminPassword = "admin123");
  
  // Backup database
  bool backupDatabase(const std::string &sourceFile, 
                     const std::string &backupDir = "./backups");
  
  // Restore database from backup
  bool restoreDatabase(const std::string &backupFile, 
                      const std::string &targetFile);
  
  // Validate user credentials
  bool validateCredentials(const std::string &filename,
                          const std::string &username,
                          const std::string &password);
  
  // Change user password
  bool changePassword(const std::string &filename,
                     const std::string &username,
                     const std::string &oldPassword,
                     const std::string &newPassword);
}

#endif  // SRC_DB_DATABASE_UTILS_H_