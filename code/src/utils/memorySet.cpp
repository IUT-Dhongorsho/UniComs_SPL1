void* memorySet(void* dest, int ch, unsigned long long count) {
    unsigned char* p = static_cast<unsigned char*>(dest);
    unsigned char c = static_cast<unsigned char>(ch);
    
    for (unsigned long long i = 0; i < count; ++i) {
        p[i] = c;
    }
    
    return dest;
}