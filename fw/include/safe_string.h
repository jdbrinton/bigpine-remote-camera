#ifndef AWARE_SAFE_STRING_H
#define AWARE_SAFE_STRING_H

namespace aware
{

inline int safe_strcpy(char *const destination, size_t destination_size, const char *const source)
{
    size_t len = strlen(source);
    if (destination_size <= len)
    {
        // Destination buffer too short
        if (destination_size > 0)
        {
            // Null terminate string regardless
            destination[0] = 0;
        }
        return -1;
    }
    // Copy string without null terminator
    memcpy(destination, source, len);

    // Add null terminator for safety
    destination[len] = 0;

    return 0;
}

inline int safe_strcat(char *const destination, size_t destination_size, const char *const source)
{
    size_t len = strlen(source);
    size_t dest_len = strlen(destination);
    if (destination_size - dest_len <= len)
    {
        // Destination buffer too short
        return -1;
    }

    if (dest_len + 1 > destination_size)
    {
        // Destination length larger than buffer size
        return -2;
    }

    // Copy string without null terminator
    memcpy(destination + dest_len, source, len);

    // Add null terminator for safety
    destination[dest_len + len] = 0;

    return 0;
}

} // namespace aware

#endif