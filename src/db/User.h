#ifndef SRC_DB_USER_H_
#define SRC_DB_USER_H_

#include <string>
#include <vector>
#include "entry/Entry.h"

class User : public Entry {
 public:
  // Static constants
  static const std::vector<std::string> Keys;
  static const size_t SerializedSize;
  
  // Constructors
  User();
  explicit User(const Entry::Values &values);
  explicit User(const std::string &str);
  
  // Convenience getters
  std::string getName() const;
  std::string getPassword() const;
  std::string getStatus() const;
  
  // Convenience setters
  bool setName(const std::string &name);
  bool setPassword(const std::string &password);
  bool setStatus(const std::string &status);
};

#endif  // SRC_DB_USER_H_