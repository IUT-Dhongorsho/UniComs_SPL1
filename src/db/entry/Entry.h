#ifndef SRC_DB_ENTRY_ENTRY_H_
#define SRC_DB_ENTRY_ENTRY_H_

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>

class Entry
{
public:
    using Map = std::map<std::string, std::string>;
    using Keys = std::vector<std::string>;
    using Values = std::vector<std::string>;

    // Constructors
    explicit Entry(size_t _sizee) : sizee(_sizee) {}    
    Entry(size_t _sizee, const Keys &keys);       
    Entry(size_t _sizee, const std::string &str); 
    Entry(const Entry &other);                 

    // Public methods
    void insert(const std::string &key, const std::string &value); 
    bool set(const std::string &key, const std::string &value);    
    std::string get(const std::string &key) const;                 
    Map get() const;                                               
    std::string serialize() const;                                 
    size_t size() const { return sizee; }                             

private:
    size_t sizee;
    Map m;
};

#endif