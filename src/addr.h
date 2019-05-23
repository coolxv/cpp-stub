#ifndef __ADDR_H__
#define __ADDR_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <map>
#include <cxxabi.h>
#include <elfio/elfio.hpp>

inline bool demangle(std::string s, std::string& res) {
    int status;
    char* name = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    if (status != 0)
    {
        switch(status)
        {
            case -1: res = "memory allocation error"; break;
            case -2: res = "invalid name given"; break;
            case -3: res = "internal error: __cxa_demangle: invalid argument"; break;
            default: res = "unknown error occured"; break;
        }
        return false;
    }
    res = name;
    free(name);
    return true;
}



inline bool get_exe_pathname( std::string& res)
{
    char                     line[512];
    FILE                    *fp;
    uintptr_t                base_addr;
    char                     perm[5];
    unsigned long            offset;
    int                      pathname_pos;
    char                    *pathname;
    size_t                   pathname_len;
    int                      match = 0;
    
    if(NULL == (fp = fopen("/proc/self/maps", "r")))
    {
        return false;
    }

    while(fgets(line, sizeof(line), fp))
    {
        if(sscanf(line, "%" PRIxPTR "-%*lx %4s %lx %*x:%*x %*d%n", &base_addr, perm, &offset, &pathname_pos) != 3) continue;

        if(0 != offset) continue;

        //get pathname
        while(isspace(line[pathname_pos]) && pathname_pos < (int)(sizeof(line) - 1))
            pathname_pos += 1;
        if(pathname_pos >= (int)(sizeof(line) - 1)) continue;
        pathname = line + pathname_pos;
        pathname_len = strlen(pathname);
        if(0 == pathname_len) continue;
        if(pathname[pathname_len - 1] == '\n')
        {
            pathname[pathname_len - 1] = '\0';
            pathname_len -= 1;
        }
        if(0 == pathname_len) continue;
        if('[' == pathname[0]) continue;

        res = pathname;
        match = 1;
        break;

    }
    fclose(fp);

    if(0 == match)
    {
        return false;
    }
    else
    {
        return true;
    }

}




inline bool get_lib_pathname_and_baseaddr(std::string pathname_regex_str, std::string& res, unsigned long& addr)
{
    char                     line[512];
    FILE                    *fp;
    uintptr_t                base_addr;
    char                     perm[5];
    unsigned long            offset;
    int                      pathname_pos;
    char                    *pathname;
    size_t                   pathname_len;
    int                      match;
    regex_t   pathname_regex;

    regcomp(&pathname_regex, pathname_regex_str.c_str(), 0);

    if(NULL == (fp = fopen("/proc/self/maps", "r")))
    {
        return false;
    }

    while(fgets(line, sizeof(line), fp))
    {
        if(sscanf(line, "%" PRIxPTR "-%*lx %4s %lx %*x:%*x %*d%n", &base_addr, perm, &offset, &pathname_pos) != 3) continue;

        //check permission
        if(perm[0] != 'r') continue;
        if(perm[3] != 'p') continue; //do not touch the shared memory

        //check offset
        //
        //We are trying to find ELF header in memory.
        //It can only be found at the beginning of a mapped memory regions
        //whose offset is 0.
        if(0 != offset) continue;

        //get pathname
        while(isspace(line[pathname_pos]) && pathname_pos < (int)(sizeof(line) - 1))
            pathname_pos += 1;
        if(pathname_pos >= (int)(sizeof(line) - 1)) continue;
        pathname = line + pathname_pos;
        pathname_len = strlen(pathname);
        if(0 == pathname_len) continue;
        if(pathname[pathname_len - 1] == '\n')
        {
            pathname[pathname_len - 1] = '\0';
            pathname_len -= 1;
        }
        if(0 == pathname_len) continue;
        if('[' == pathname[0]) continue;

        //check pathname
        //if we need to hook this elf?
        match = 0;
        if(0 == regexec(&pathname_regex, pathname, 0, NULL, 0))
        {
            match = 1;
            res = pathname;
            addr = (unsigned long)base_addr;
            break;
        }
        if(0 == match) continue;

    }
    fclose(fp);
    if(0 == match)
    {
        return false;
    }
    else
    {
        return true;
    }

}


inline int get_local_func_addr(std::string file_name, std::string func_name_regex_str, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    regex_t   pathname_regex;

    regcomp(&pathname_regex, func_name_regex_str.c_str(), 0);
    // Load ELF data
    if(!reader.load(file_name.c_str()))
    {
        return -1;
    }
    
    ELFIO::Elf_Half sec_num = reader.sections.size();
    for(int i = 0; i < sec_num; ++i)
    {
        ELFIO::section* psec = reader.sections[i];
        // Check section type
        if(psec->get_type() == SHT_SYMTAB)
        {
            const ELFIO::symbol_section_accessor symbols( reader, psec );
            for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j )
            {
                std::string   name;
                std::string   name_mangle;
                ELFIO::Elf64_Addr    value;
                ELFIO::Elf_Xword     size;
                unsigned char bind;
                unsigned char type;
                ELFIO::Elf_Half      section_index;
                unsigned char other;
                
                // Read symbol properties
                symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
                if(type == STT_FUNC && bind == STB_LOCAL)
                {
                    bool ret = demangle(name,name_mangle);
                    if(ret == true)
                    {
                        if (0 == regexec(&pathname_regex, name_mangle.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        if (0 == regexec(&pathname_regex, name.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name,value));
                              count++;
                        }
                    }
                }
            }
            break;
        }
    }
    
    return count;
}
inline int get_globle_func_addr(std::string file_name, std::string func_name_regex_str, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    regex_t   pathname_regex;

    regcomp(&pathname_regex, func_name_regex_str.c_str(), 0);
    // Load ELF data
    if(!reader.load(file_name.c_str()))
    {
        return -1;
    }
    
    ELFIO::Elf_Half sec_num = reader.sections.size();
    for(int i = 0; i < sec_num; ++i)
    {
        ELFIO::section* psec = reader.sections[i];
        // Check section type
        if(psec->get_type() == SHT_SYMTAB)
        {
            const ELFIO::symbol_section_accessor symbols( reader, psec );
            for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j )
            {
                std::string   name;
                std::string   name_mangle;
                ELFIO::Elf64_Addr    value;
                ELFIO::Elf_Xword     size;
                unsigned char bind;
                unsigned char type;
                ELFIO::Elf_Half      section_index;
                unsigned char other;
                
                // Read symbol properties
                symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
                if(type == STT_FUNC && bind == STB_GLOBAL)
                {
                    bool ret = demangle(name,name_mangle);
                    if(ret == true)
                    {
                        if (0 == regexec(&pathname_regex, name_mangle.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        if (0 == regexec(&pathname_regex, name.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name,value));
                              count++;
                        }
                    }
                }
            }
            break;
        }
    }
    
    return count;
}
inline int get_weak_func_addr(std::string file_name, std::string func_name_regex_str, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    regex_t   pathname_regex;

    regcomp(&pathname_regex, func_name_regex_str.c_str(), 0);
    // Load ELF data
    if(!reader.load(file_name.c_str()))
    {
        return -1;
    }
    
    ELFIO::Elf_Half sec_num = reader.sections.size();
    for(int i = 0; i < sec_num; ++i)
    {
        ELFIO::section* psec = reader.sections[i];
        // Check section type
        if(psec->get_type() == SHT_SYMTAB)
        {
            const ELFIO::symbol_section_accessor symbols( reader, psec );
            for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j )
            {
                std::string   name;
                std::string   name_mangle;
                ELFIO::Elf64_Addr    value;
                ELFIO::Elf_Xword     size;
                unsigned char bind;
                unsigned char type;
                ELFIO::Elf_Half      section_index;
                unsigned char other;
                
                // Read symbol properties
                symbols.get_symbol( j, name, value, size, bind, type, section_index, other );
                if(type == STT_FUNC && bind == STB_WEAK)
                {
                    bool ret = demangle(name,name_mangle);
                    if(ret == true)
                    {
                        if (0 == regexec(&pathname_regex, name_mangle.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        if (0 == regexec(&pathname_regex, name.c_str(), 0, NULL, 0))
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name,value));
                              count++;
                        }
                    }
                }
            }
            break;
        }
    }
    
    return count;
}

#endif
