#ifndef NetworkUtils_hpp
#define NetworkUtils_hpp

#include <iostream>

// #define INADDR_ANY (u_long)0x00000000

namespace HDE{
    // Check if system is little-endian
    inline bool isLittleEndian(){
        unsigned short num = 1;
        return (*(unsigned char*)&num == 1);
    }

    // Convert 16-bit host to network byte order
    inline unsigned short htonsCustom(unsigned short hostshort){
        if(isLittleEndian()){
            return ((hostshort & 0xFF00) >> 8) | ((hostshort & 0x00FF) << 8);
        }
        return hostshort;
    }

    // Convert 32-bit host to network byte order
    inline unsigned long htonlCustom(unsigned long hostlong){
        if(isLittleEndian()){
            return ((hostlong & 0xFF000000) >> 24) |
                   ((hostlong & 0x00FF0000) >> 8)  |
                   ((hostlong & 0x0000FF00) << 8)  |
                   ((hostlong & 0x000000FF) << 24);
        }
        return hostlong;
    }

    // Print error message
    inline void printError(const char* message){
        std::cerr << "Error: " << message << std::endl;
    }
}

#endif