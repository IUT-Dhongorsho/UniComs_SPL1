#ifndef SRC_DB_DATABASE_H_
#define SRC_DB_DATABASE_H_

#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

template <class T>
class Database {
 public:
  // Constructor
  explicit Database(const std::string &filename);
  
  // Database operations
  int add(const T &entry) const;
  std::pair<T, bool> get(const std::string &primaryKeyValue) const;  // Added missing semicolon
  bool update(const T &newEntry) const;
  bool remove(const std::string &primaryKeyValue) const;
  std::vector<T> getActiveUsers() const;
  
  // Additional utility methods
  std::vector<T> getAll() const;
  bool exists(const std::string &primaryKeyValue) const;
  size_t count() const;
  size_t countActive() const;

 private:
  std::string dbFile;
  
  // Private helper methods
  std::vector<T> readAllEntries() const;
  bool writeAllEntries(const std::vector<T> &entries) const;
};

#endif  // SRC_DB_DATABASE_H_