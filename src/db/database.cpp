#include "Database.h"
#include "User.h"

// Constructor
template<class T>
Database<T>::Database(const std::string &filename) : dbFile(filename) {
    // Create file if it doesn't exist
    std::ofstream create(dbFile, std::ios::app);
    // File is created automatically by opening it
}

// Add new entry
template<class T>
int Database<T>::add(const T &entry) const {
    std::vector<T> allEntries = readAllEntries();
    
    // Check if entry already exists
    for (const auto &e : allEntries) {
        if (e.get("name") == entry.get("name")) {
            return 1; // Entry already exists
        }
    }
    
    // Append to file
    std::ofstream file(dbFile, std::ios::app | std::ios::binary);
    if (!file) return -1;
    
    file << entry.serialize() << "\n";
    return 0;
}

// Get entry by primary key
template<class T>
std::pair<T, bool> Database<T>::get(const std::string &primaryKeyValue) const {
    std::vector<T> allEntries = readAllEntries();
    
    for (const auto &entry : allEntries) {
        if (entry.get("name") == primaryKeyValue) {
            return {entry, true};
        }
    }
    
    return {T(), false};
}

// Update existing entry
template<class T>
bool Database<T>::update(const T &newEntry) const {
    std::vector<T> allEntries = readAllEntries();
    bool found = false;
    
    // Update the entry if found
    for (auto &entry : allEntries) {
        if (entry.get("name") == newEntry.get("name")) {
            entry = newEntry;
            found = true;
            break;
        }
    }
    
    if (!found) return false;
    
    return writeAllEntries(allEntries);
}

// Remove entry
template<class T>
bool Database<T>::remove(const std::string &primaryKeyValue) const {
    std::vector<T> allEntries = readAllEntries();
    bool found = false;
    
    // Remove the entry
    for (auto it = allEntries.begin(); it != allEntries.end(); ++it) {
        if (it->get("name") == primaryKeyValue) {
            allEntries.erase(it);
            found = true;
            break;
        }
    }
    
    if (!found) return false;
    
    return writeAllEntries(allEntries);
}

// Get all active users (simplified - all users are considered active)
template<class T>
std::vector<T> Database<T>::getActiveUsers() const {
    return readAllEntries();
}

// Get all entries
template<class T>
std::vector<T> Database<T>::getAll() const {
    return readAllEntries();
}

// Check if entry exists
template<class T>
bool Database<T>::exists(const std::string &primaryKeyValue) const {
    auto result = get(primaryKeyValue);
    return result.second;
}

// Count all entries
template<class T>
size_t Database<T>::count() const {
    return readAllEntries().size();
}

// Count active entries
template<class T>
size_t Database<T>::countActive() const {
    return readAllEntries().size();
}

// Private helper: Read all entries from file
template<class T>
std::vector<T> Database<T>::readAllEntries() const {
    std::vector<T> entries;
    std::ifstream file(dbFile, std::ios::binary);
    
    if (!file) return entries;
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                entries.emplace_back(line);
            } catch (...) {
                // Skip malformed entries
                continue;
            }
        }
    }
    
    return entries;
}

// Private helper: Write all entries to file
template<class T>
bool Database<T>::writeAllEntries(const std::vector<T> &entries) const {
    std::ofstream file(dbFile, std::ios::trunc | std::ios::binary);
    if (!file) return false;
    
    for (const auto &entry : entries) {
        file << entry.serialize() << "\n";
    }
    
    return true;
}

// Explicit template instantiation
template class Database<User>;