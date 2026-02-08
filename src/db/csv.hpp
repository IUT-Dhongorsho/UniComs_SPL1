#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>

std::string toCSV(std::vector<std::string> &strings)
{
    std::string csvLine;
    for (std::string &s : strings)
    {
        csvLine.append(s);
        csvLine.append(",");
    }
    csvLine.pop_back();
    csvLine.append("\n");
    return csvLine;
}

#endif
