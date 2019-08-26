#ifndef AWARE_SUBSTRING_H
#define AWARE_SUBSTRING_H

class Substring
{
  public:
    Substring(const char *string, size_t offset, size_t length) : _string(string), _offset(offset), _length(length) {}

    bool operator==(const char *other) const
    {
        const char *ch = _string + _offset;
        const char *och = other;
        const char *end = ch + _length;
        bool match = true;
        while (ch < end && match)
        {
            match = (*ch == *och);
            ++ch;
            ++och;
        }
        match = match && !(*och); // Match if strings match and the next character of other is zero.
        return match;
    }

    inline bool operator!=(const char *other) const
    {
        return !(this->operator==(other));
    }

    char operator[] (const int index) const {
        return *(_string + _offset + index);
    }

    inline size_t length() const {
        return _length; 
    }

    inline int copy_to_c_string(char * const buffer, size_t buffer_size, bool truncate = false) {
        if (!truncate && (_length + 1 > buffer_size)) {
            return -1;
        }
        size_t length = std::min(buffer_size - 1, _length);
        memcpy(buffer, _string + _offset, length);
        *(buffer + _length) = 0;
        return 0;
    }

  private:
    const char *_string;
    size_t _offset;
    size_t _length;
};

#endif