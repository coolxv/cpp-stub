#ifndef __ADDR_H__
#define __ADDR_H__

#include <map>
#include <cxxabi.h>
#include <elfio/elfio.hpp>


bool demangle(std::string s, std::string& res) {
    int status;
    char* name = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    if (status != 0) {
        std::string err;
        switch(status) {
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


int get_exe_local_func_addr(std::string file_name, std::string func_name, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    
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
                        std::size_t found = name_mangle.find(func_name);
                        if (found != std::string::npos)
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        std::size_t found = name.find(func_name);
                        if (found != std::string::npos)
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
int get_exe_globle_func_addr(std::string file_name, std::string func_name, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    
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
                        std::size_t found = name_mangle.find(func_name);
                        if (found != std::string::npos)
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        std::size_t found = name.find(func_name);
                        if (found != std::string::npos)
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
int get_exe_weak_func_addr(std::string file_name, std::string func_name, std::map<std::string,ELFIO::Elf64_Addr>& result)
{
    // Create an elfio reader
    ELFIO::elfio reader;
    int count = 0;
    
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
                        std::size_t found = name_mangle.find(func_name);
                        if (found != std::string::npos)
                        {
                              result.insert ( std::pair<std::string,ELFIO::Elf64_Addr>(name_mangle,value));
                              count++;
                        }
                    }
                    else
                    {
                        std::size_t found = name.find(func_name);
                        if (found != std::string::npos)
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
