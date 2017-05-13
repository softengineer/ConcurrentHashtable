//
//  StringUtils.h
//

#ifndef StringUtils_h
#define StringUtils_h

#include <iostream>
#include <vector>
#include <regex>

namespace mt {
    
    /**
      split a string with seperator sep, return a vector contained with splitted result
     */
    std::vector<std::string> split(const std::string & val, const std::string & sep)
    {
        std::vector<std::string> ret;
        
        std::string::size_type current = 0;
        std::string::size_type last = -1;
        std::string::size_type pos;
        
        while ((pos = val.find(sep, current)) != std::string::npos)
        {
            ret.push_back(val.substr(last + 1, pos - last -1));
            last = pos;
            current = pos + 1;
        }
        
        ret.push_back(val.substr(last + 1, val.length() - last));
        
        return ret;
    }
    
    /**
     split a string with seperator sep, return a vector contained with splitted result
     sep support regular express
     */
    std::vector<std::string> splitEx(const std::string & val, const std::string & sep)
    {
        std::smatch m;
        std::vector<std::string> ret;
        std::regex e(sep);
        
        auto pos = val.begin();
        auto end = val.end();
        std::string::size_type last = 0;
        
        for ( ; regex_search (pos,end,m,e); pos = m.suffix().first) {
            ret.push_back(val.substr(last, m.position()));
            last = last + m.position() + m.length();
        }
        
        if (last == 0)
            ret.push_back(val);
        else
            ret.push_back(val.substr(last, val.size() - last));
        return ret;
    }
}

#endif /* StringUtils_h */
