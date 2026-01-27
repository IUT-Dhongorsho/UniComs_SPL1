#include "Entry.h"

// Constructor from string (deserialize)
Entry::Entry(size_t _sizee, const std::string &str) {
    using std::getline;
    using std::map;
    using std::string;
    using std::stringstream;

    sizee = _sizee;
    stringstream in(str);
    string token, key, value;
    
    while ((getline(in, token, ',') >> std::ws) && !token.empty()) {
        stringstream tk(token);
        getline(getline(tk, key, ':') >> std::ws, value);
        m[key] = value;
    }
}

// Copy constructor
Entry::Entry(const Entry &other) {
    sizee = other.sizee;
    m = other.m;  // Simple copy
}

// Constructor with keys
Entry::Entry(size_t _sizee, const Keys &keys) {
    sizee = _sizee;
    for (const auto &key : keys) {
        m[key] = "undefined";
    }
}

// Get value for key
std::string Entry::get(const std::string &key) const {
    auto it = m.find(key);
    if (it == m.end()) return "undefined";
    return it->second;
}

// Get all key-value pairs
Entry::Map Entry::get() const {
    return m;
}

// Serialize to string
std::string Entry::serialize() const {
    std::string s;
    s.reserve(sizee);
    
    for (const auto &p : m) {
        s += p.first + ":" + p.second + ",";
    }
    
    // Remove trailing comma if exists
    if (!s.empty()) {
        s.pop_back();
    }
    
    // Pad with spaces to reach exact size
    if (s.size() > sizee) {
        throw std::length_error("Cannot serialize: data exceeds allocated size");
    }
    
    std::string pad(sizee - s.size(), ' ');
    s.append(pad);
    return s;
}

// Set existing key to new value
bool Entry::set(const std::string &key, const std::string &value) {
    if (m.find(key) == m.end()) return false;
    m[key] = value;
    return true;
}

// Insert new key-value pair
void Entry::insert(const std::string &key, const std::string &value) {
    m[key] = value;
}