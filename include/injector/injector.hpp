/*
 *  Injectors - Base Header
 *
 *  Copyright (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

/*
*   Injectors - arm64-v8a + Linux port by Xan/Tenjoin
*/

#pragma once
#define INJECTOR_HAS_INJECTOR_HPP
#include <cstdint>
#include <cstdio>
#include <sys/mman.h>
#include "gvm/gvm.hpp"
//#include "../../hook_main.h"

namespace injector
{

/*
 *  auto_pointer 
 *      Casts itself to another pointer type in the lhs
 */
union auto_pointer
{
    protected:
        friend union memory_pointer_tr;
        template<class T> friend union basic_memory_pointer;
        
        void*	 p;
        uintptr_t a;

    public:
        auto_pointer() : p(0)                          {}
        auto_pointer(const auto_pointer& x) : p(x.p)  {}
        explicit auto_pointer(void* x)    : p(x)       {}
        explicit auto_pointer(uint32_t x) : a(x)       {}

        bool is_null() const { return this->p != nullptr; }

    #if __cplusplus >= 201103L || _MSC_VER >= 1800
        explicit operator bool() const { return is_null(); }
    #endif

        auto_pointer get() const               { return *this; }
        template<class T> T* get() const       { return (T*) this->p; }
        template<class T> T* get_raw() const   { return (T*) this->p; }

        template<class T>
        operator T*() const { return reinterpret_cast<T*>(p); }
};

/*
 *  basic_memory_pointer
 *      A memory pointer class that is capable of many operations, including address translation
 *      MemTranslator is the translator functor
 */
template<class MemTranslator>
union basic_memory_pointer
{
    protected:
        void*	  p;
        uintptr_t a;
        
        // Translates address p to the running executable pointer
        static auto_pointer memory_translate(void* p)
        {
            return auto_pointer(MemTranslator()(p));
        }

    public:
        basic_memory_pointer()                      : p(nullptr)    {}
        basic_memory_pointer(std::nullptr_t)        : p(nullptr)    {}
        basic_memory_pointer(uintptr_t x)           : a(x)          {}
        basic_memory_pointer(const auto_pointer& x) : p(x.p)        {}
        basic_memory_pointer(const basic_memory_pointer& rhs) : p(rhs.p)  {}

        template<class T>
        basic_memory_pointer(T* x) : p((void*)x) {}

        

        
        // Gets the translated pointer (plus automatic casting to lhs)
        auto_pointer get() const               { return memory_translate(p); }
        
        // Gets the translated pointer (casted to T*)
        template<class T> T* get() const        { return get(); }
        
        // Gets the raw pointer, without translation (casted to T*)
        template<class T> T* get_raw() const    { return auto_pointer(p); }
        
        // This type can get assigned from void* and uintptr_t
        basic_memory_pointer& operator=(void* x)		{ return p = x, *this; }
        basic_memory_pointer& operator=(uintptr_t x)	{ return a = x, *this; }
        
        /* Arithmetic */
        basic_memory_pointer operator+(const basic_memory_pointer& rhs) const
        { return basic_memory_pointer(this->a + rhs.a); }
        
        basic_memory_pointer operator-(const basic_memory_pointer& rhs) const
        { return basic_memory_pointer(this->a - rhs.a); }
        
        basic_memory_pointer operator*(const basic_memory_pointer& rhs) const
        { return basic_memory_pointer(this->a * rhs.a); }
        
        basic_memory_pointer operator/(const basic_memory_pointer& rhs) const
        { return basic_memory_pointer(this->a / rhs.a); }
        
        
        /* Comparision */
        bool operator==(const basic_memory_pointer& rhs) const
        { return this->a == rhs.a; }
        
        bool operator!=(const basic_memory_pointer& rhs) const
        { return this->a != rhs.a; }
        
        bool operator<(const basic_memory_pointer& rhs) const
        { return this->a < rhs.a; }
        
        bool operator<=(const basic_memory_pointer& rhs) const
        { return this->a <= rhs.a; }
        
        bool operator>(const basic_memory_pointer& rhs) const
        { return this->a > rhs.a; }
        
        bool operator>=(const basic_memory_pointer& rhs) const
        { return this->a >=rhs.a; }
        
        bool is_null() const      { return this->p == nullptr; }
        uintptr_t as_int() const  { return this->a; }	// does not perform translation



#if __cplusplus >= 201103L || _MSC_VER >= 1800  // MSVC 2013
        /* Conversion to other types */
        explicit operator uintptr_t() const
        { return this->a; }	// does not perform translation
        explicit operator bool() const
        { return this->p != nullptr; }
#else
        //operator bool() -------------- Causes casting problems because of implicitness, use !is_null()
        //{ return this->p != nullptr; }
#endif

};

 // Typedefs including memory translator for the above type
typedef basic_memory_pointer<address_manager::fn_mem_translator>       memory_pointer;
typedef basic_memory_pointer<address_manager::fn_mem_translator_nop>   memory_pointer_raw;
//typedef basic_memory_pointer<address_manager::fn_mem_translator_aslr>  memory_pointer_aslr;



/*
 *  memory_pointer_tr
 *      Stores a basic_memory_pointer<Tr> as a raw pointer from translated pointer
 */
union memory_pointer_tr
{
    protected:
        void*     p;
        uintptr_t a;
    
    public:
        template<class Tr>
        memory_pointer_tr(const basic_memory_pointer<Tr>& ptr)
            : p(ptr.get())
        {}      // Constructs from a basic_memory_pointer
      
        memory_pointer_tr(const auto_pointer& ptr)
            : p(ptr.p)
        {}  // Constructs from a auto_pointer, probably comming from basic_memory_pointer::get
        
        memory_pointer_tr(const memory_pointer_tr& rhs)
            : p(rhs.p)
        {}  // Constructs from my own type, copy constructor
        
        memory_pointer_tr(uintptr_t x)
            : p(memory_pointer(x).get())
        {}  // Constructs from a integer, translating the address
      
        memory_pointer_tr(void* x)
            : p(memory_pointer(x).get())
        {}  // Constructs from a void pointer, translating the address
        
        // Just to be method-compatible with basic_memory_pointer ...
        auto_pointer         get()      { return auto_pointer(p);     }
        template<class T> T* get()      { return get(); }
        template<class T> T* get_raw()  { return get(); }

        memory_pointer_tr operator+(const uintptr_t& rhs) const
        { return memory_pointer_raw(this->a + rhs); }
        
        memory_pointer_tr operator-(const uintptr_t& rhs) const
        { return memory_pointer_raw(this->a - rhs); }
        
        memory_pointer_tr operator*(const uintptr_t& rhs) const
        { return memory_pointer_raw(this->a * rhs); }
        
        memory_pointer_tr operator/(const uintptr_t& rhs) const
        { return memory_pointer_raw(this->a / rhs); }

        bool is_null() const      { return this->p == nullptr; }
        uintptr_t as_int() const  { return this->a; }
        
#if __cplusplus >= 201103L
       explicit operator uintptr_t() const
       { return this->a; }
#else
#endif

};


/*
 *  ProtectMemory
 *      Makes the address @addr have a protection of @protection
 */
inline int ProtectMemory(memory_pointer_tr addr, unsigned int protection)
{
    //return VirtualProtect(addr.get(), size, protection, &protection) != 0;
    //return true;
    uintptr_t calcaddr = (uintptr_t)addr.get<void>();

    void* page_start = (void*)(calcaddr - calcaddr % PAGE_SIZE);
    return mprotect(page_start, PAGE_SIZE, protection) == 0;

}

/*
 *  UnprotectMemory
 *      Unprotect the memory at @addr with size @size so it have all accesses (execute, read and write)
 *      Returns the old protection to out_oldprotect
 */
inline int UnprotectMemory(memory_pointer_tr addr, bool bExecutable, unsigned int& out_oldprotect)
{
    //return VirtualProtect(addr.get(), size, PAGE_EXECUTE_READWRITE, &out_oldprotect) != 0;
    //return true;
    uintptr_t calcaddr = (uintptr_t)addr.get<void>();

    void* page_start = (void*)(calcaddr - calcaddr % PAGE_SIZE);
    out_oldprotect = PROT_READ; // TODO -- find an easy way to get the current memory page status, this is a HACK
    if (bExecutable) out_oldprotect |= PROT_EXEC;
    //LOGD("unprotect addr: 0x%lX\n", (unsigned long)calcaddr);
    return mprotect(page_start, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

/*
 *  scoped_unprotect
 *      RAII wrapper for UnprotectMemory
 *      On construction unprotects the memory, on destruction reprotects the memory
 */
struct scoped_unprotect
{
    memory_pointer_raw  addr;
    size_t              size;
    unsigned int        dwOldProtect;
    bool                bUnprotected;

    scoped_unprotect(memory_pointer_tr addr, size_t size, bool bExecutable)
    {
        if(size == 0) bUnprotected = false;
        else          bUnprotected = UnprotectMemory(this->addr = addr.get<void>(), bExecutable, dwOldProtect);
    }
    
    ~scoped_unprotect()
    {
        if(bUnprotected) ProtectMemory(this->addr.get(), this->dwOldProtect);
    }
};








/*
 *  WriteMemoryRaw 
 *      Writes into memory @addr the content of @value with a sizeof @size
 *      Does memory unprotection if @vp is true
 */
inline void WriteMemoryRaw(memory_pointer_tr addr, void* value, size_t size, bool vp, bool exec)
{
    scoped_unprotect xprotect(addr, vp? size : 0, exec);
    memcpy(addr.get(), value, size);
}

/*
 *  ReadMemoryRaw 
 *      Reads the memory at @addr with a sizeof @size into address @ret
 *      Does memory unprotection if @vp is true
 */
inline void ReadMemoryRaw(memory_pointer_tr addr, void* ret, size_t size, bool vp, bool exec)
{
    scoped_unprotect xprotect(addr, vp? size : 0, exec);
    memcpy(ret, addr.get(), size);
}

/*
 *  MemoryFill 
 *      Fills the memory at @addr with the byte @value doing it @size times
 *      Does memory unprotection if @vp is true
 */
inline void MemoryFill(memory_pointer_tr addr, uint8_t value, size_t size, bool vp, bool exec)
{
    scoped_unprotect xprotect(addr, vp? size : 0, exec);
    memset(addr.get(), value, size);
}

/*
 *  WriteObject
 *      Assigns the object @value into the same object type at @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline T& WriteObject(memory_pointer_tr addr, const T& value, bool vp = false, bool exec = false)
{
    scoped_unprotect xprotect(addr, vp? sizeof(value) : 0, exec);
    return (*addr.get<T>() = value);
}

/*
 *  ReadObject
 *      Assigns the object @value with the value of the same object type at @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline T& ReadObject(memory_pointer_tr addr, T& value, bool vp = false, bool exec = false)
{
    scoped_unprotect xprotect(addr, vp? sizeof(value) : 0, exec);
    return (value = *addr.get<T>());
}


/*
 *  WriteMemory
 *      Writes the object of type T into the address @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline void WriteMemory(memory_pointer_tr addr, T value, bool vp = false, bool exec = false)
{
    WriteObject(addr, value, vp, exec);
}

/*
 *  ReadMemory
 *      Reads the object type T at address @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline T ReadMemory(memory_pointer_tr addr, bool vp = false, bool exec = false)
{
    T value;
    return ReadObject(addr, value, vp, exec);
}

/*
 *  WriteMemoryNoTr
 *      Writes the object of type T into the address @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline void WriteMemoryNoTr(memory_pointer_raw addr, T value, bool vp = false, bool exec = false)
{
    WriteObject(addr, value, vp, exec);
}

/*
 *  ReadMemoryNoTr
 *      Reads the object type T at address @addr
 *      Does memory unprotection if @vp is true
 */
template<class T>
inline T ReadMemoryNoTr(memory_pointer_raw addr, bool vp = false, bool exec = false)
{
    T value;
    return ReadObject(addr, value, vp, exec);
}

/*
 *  AdjustPointer 
 *      Searches in the range [@addr, @addr + @max_search] for a pointer in the range [@default_base, @default_end] and replaces
 *      it with the proper offset in the pointer @replacement_base.
 *      Does memory unprotection if @vp is true.
 */
 inline memory_pointer_raw AdjustPointer(memory_pointer_tr addr,
                                         memory_pointer_raw replacement_base, memory_pointer_tr default_base, memory_pointer_tr default_end,
                                         size_t max_search = 8, bool vp = true, bool exec = false)
 {
    scoped_unprotect xprotect(addr, vp? max_search + sizeof(void*) : 0, exec);
    for(size_t i = 0; i < max_search; ++i)
    {
        memory_pointer_raw ptr = ReadMemory<void*>(addr + i);
        if(ptr >= default_base.get() && ptr <= default_end.get())
        {
            auto result = replacement_base + (ptr - default_base.get());
            WriteMemory<void*>(addr + i, result.get());
            return result;
        }
    }
    return nullptr;
 }






/*
 *  GetAbsoluteOffset
 *      Gets absolute address based on relative offset @rel_value from instruction that ends at @end_of_instruction
 */
inline memory_pointer_raw GetAbsoluteOffset(int rel_value, memory_pointer_tr end_of_instruction)
{
    return end_of_instruction.get<char>() + rel_value;
}

/*
 *  GetRelativeOffset
 *      Gets relative offset based on absolute address @abs_value for instruction that ends at @end_of_instruction
 */
inline int GetRelativeOffset(memory_pointer_tr abs_value, memory_pointer_tr end_of_instruction)
{
    return uintptr_t(abs_value.get<char>() - end_of_instruction.get<char>());
}

/*
 *  ReadRelativeOffset
 *      Reads relative offset from address @at
 */
inline memory_pointer_raw ReadRelativeOffset(memory_pointer_tr at, size_t sizeof_addr = 4, bool vp = true)
{
    switch(sizeof_addr)
    {
        case 1: return (GetAbsoluteOffset(ReadMemory<int8_t> (at, vp), at+sizeof_addr));
        case 2: return (GetAbsoluteOffset(ReadMemory<int16_t>(at, vp), at+sizeof_addr));
        case 4: return (GetAbsoluteOffset(ReadMemory<int32_t>(at, vp), at+sizeof_addr));
    }
    return nullptr;
}

/*
 *  MakeRelativeOffset
 *      Writes relative offset into @at based on absolute destination @dest
 */
inline void MakeRelativeOffset(memory_pointer_tr at, memory_pointer_tr dest, size_t sizeof_addr = 4, bool vp = true)
{
    switch(sizeof_addr)
    {
        case 1: WriteMemory<int8_t> (at, static_cast<int8_t> (GetRelativeOffset(dest, at+sizeof_addr)), vp);
        case 2: WriteMemory<int16_t>(at, static_cast<int16_t>(GetRelativeOffset(dest, at+sizeof_addr)), vp);
        case 4: WriteMemory<int32_t>(at, static_cast<int32_t>(GetRelativeOffset(dest, at+sizeof_addr)), vp);
    }
}

/*
 *  GetBranchDestination
 *      Gets the destination of a branch instruction at address @at
 *      *** Works only with JMP and CALL for now ***
 */
inline memory_pointer_raw GetBranchDestination(memory_pointer_tr at, bool vp = true)
{
    uint32_t ins = ReadMemory<uint32_t>(at, vp, true);

    if (ins & 0x14000000) // branch arm64 -- TODO
    {
        bool sign = (ins & 0x2000000) != 0;
        int32_t off = ins & 0x1FFFFFF;
        if (sign)
            off = -off;
        off *= 4;
        
        uintptr_t out = off + (uintptr_t)(at.get<void>());
        return out;
    }

    //switch(ReadMemory<uint8_t>(at, vp))
    //{
    //    // We need to handle other instructions (and prefixes) later...
    //    case 0xE8:	// call rel
    //    case 0xE9:	// jmp rel
    //        return ReadRelativeOffset(at + 1, 4, vp);
    //
    //    case 0xFF: 
    //        switch(ReadMemory<uint8_t>(at + 1, vp))
    //        {
    //            case 0x15:  // call dword ptr [addr]
    //            case 0x25:  // jmp dword ptr [addr]
    //                return *(ReadMemory<uintptr_t*>(at + 2, vp));
    //        }
    //        break;
    //}
    return nullptr;
}

// /*
//  *  MakeJMP
//  *      Creates a JMP instruction at address @at that jumps into address @dest
//  *      If there was already a branch instruction there, returns the previosly destination of the branch
//  */
// inline memory_pointer_raw MakeJMP(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
// {
//     auto p = GetBranchDestination(at, vp);
//     WriteMemory<uint8_t>(at, 0xE9, vp);
//     MakeRelativeOffset(at+1, dest, 4, vp);
//     return p;
// }
// 
// /*
//  *  MakeCALL
//  *      Creates a CALL instruction at address @at that jumps into address @dest
//  *      If there was already a branch instruction there, returns the previosly destination of the branch
//  */
// inline memory_pointer_raw MakeCALL(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
// {
//     auto p = GetBranchDestination(at, vp);
//     WriteMemory<uint8_t>(at, 0xE8, vp);
//     MakeRelativeOffset(at+1, dest, 4, vp);
//     return p;
// }

// /*
//  *  MakeJA
//  *      Creates a JA instruction at address @at that jumps if above into address @dest
//  *      If there was already a branch instruction there, returns the previosly destination of the branch
//  */
// inline void MakeJA(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
// {
//     WriteMemory<uint16_t>(at, 0x87F0, vp);
//     MakeRelativeOffset(at+2, dest, 4, vp);
// }

/*
 *  MakeB
 *      Creates a B instruction at address @at that jumps into address @dest
 *      TODO: If there was already a branch instruction there, returns the previosly destination of the branch
 */
inline memory_pointer_raw MakeB(memory_pointer_tr at, memory_pointer_tr dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return nullptr;
    
    if ((uintptr_t)dest.get<void>() % 4)
        return nullptr;

    auto p = GetBranchDestination(at, vp);

    // construct branch arm64
    intptr_t off = (uintptr_t)(dest.get<void>()) - (uintptr_t)(at.get<void>());
    uint32_t ins = 0x14000000;

    if (off < 0)
    {
        if (off > -4)
            return nullptr;



        ins = (off / 4) & 0x17FFFFFF;
        ins |= 0x2000000;
    }
    else
    {
        if ((off > 0x7fffffc))
            return nullptr;

        ins |= ((off / 4) & 0x1FFFFFF);
    }

    //LOGD("off: 0x%X\tins: 0x%X\tat: 0x%lX\tdest: 0x%lX\n", (uint32_t)off, ins, (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    WriteMemory<uint32_t>(at, ins, true, true);

    return p;
}

/*
 *  MakeBRaw
 *      Creates a B instruction at address @at that jumps into address (raw) @dest
 *      TODO: If there was already a branch instruction there, returns the previosly destination of the branch
 */
inline memory_pointer_raw MakeBRaw(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    //LOGD("at: 0x%lX\tdest: 0x%lX\n", (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    if ((uintptr_t)at.get<void>() % 4)
        return nullptr;

    if ((uintptr_t)dest.get<void>() % 4)
        return nullptr;

    auto p = GetBranchDestination(at, vp);

    // construct branch arm64
    intptr_t off = (uintptr_t)(dest.get<void>()) - (uintptr_t)(at.get<void>());
    uint32_t ins = 0x14000000;

    if (off < 0)
    {
        if (off > -4)
            return nullptr;



        ins = (off / 4) & 0x17FFFFFF;
        ins |= 0x2000000;
    }
    else
    {
        if ((off > 0x7fffffc))
            return nullptr;

        ins |= ((off / 4) & 0x1FFFFFF);
    }

    //LOGD("off: 0x%X\tins: 0x%X\tat: 0x%lX\tdest: 0x%lX\n", (uint32_t)off, ins, (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    WriteMemory<uint32_t>(at, ins, true, true);

    return p;
}

/*
 *  MakeBL
 *      Creates a BL instruction at address @at that jumps into address @dest
 *      TODO: If there was already a branch instruction there, returns the previosly destination of the branch
 */
inline memory_pointer_raw MakeBL(memory_pointer_tr at, memory_pointer_tr dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return nullptr;

    if ((uintptr_t)dest.get<void>() % 4)
        return nullptr;

    auto p = GetBranchDestination(at, vp);

    // construct branch with link arm64
    intptr_t off = (uintptr_t)(dest.get<void>()) - (uintptr_t)(at.get<void>());
    uint32_t ins = 0x94000000;

    if (off < 0)
    {
        if (off > -4)
            return nullptr;



        ins = (off / 4) & 0x17FFFFFF;
        ins |= 0x2000000;
    }
    else
    {
        if ((off > 0x7fffffc))
            return nullptr;

        ins |= ((off / 4) & 0x1FFFFFF);
    }

    //LOGD("off: 0x%X\tins: 0x%X\tat: 0x%lX\tdest: 0x%lX\n", (uint32_t)off, ins, (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    WriteMemory<uint32_t>(at, ins, true, true);

    return p;
}

/*
 *  MakeBLRaw
 *      Creates a BL instruction at address @at that jumps into address (raw) @dest
 *      TODO: If there was already a branch instruction there, returns the previosly destination of the branch
 */
inline memory_pointer_raw MakeBLRaw(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    //LOGD("at: 0x%lX\tdest: 0x%lX\n", (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    if ((uintptr_t)at.get<void>() % 4)
        return nullptr;

    if ((uintptr_t)dest.get<void>() % 4)
        return nullptr;

    auto p = GetBranchDestination(at, vp);

    // construct branch with link arm64
    intptr_t off = (uintptr_t)(dest.get<void>()) - (uintptr_t)(at.get<void>());
    uint32_t ins = 0x94000000;

    if (off < 0)
    {
        if (off > -4)
            return nullptr;



        ins = (off / 4) & 0x97FFFFFF;
        ins |= 0x2000000;
    }
    else
    {
        if ((off > 0x7fffffc))
            return nullptr;

        ins |= ((off / 4) & 0x1FFFFFF);
    }

    //LOGD("off: 0x%X\tins: 0x%X\tat: 0x%lX\tdest: 0x%lX\n", (uint32_t)off, ins, (unsigned long)(at.get<void>()), (unsigned long)(dest.get<void>()));

    WriteMemory<uint32_t>(at, ins, true, true);

    return p;
}

/*
 *  MakeBR
 *      Creates BR instructions at address @at that jumps into address @dest with register X16
 */
inline void MakeBR(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return;

    if ((uintptr_t)dest.get<void>() % 4)
        return;

    // assemble ADRP X16, addr & 0xF000
    intptr_t off = ((uintptr_t)(dest.get<void>()) & 0xFFFFFFFFFFFFF000) - ((uintptr_t)(at.get<void>()) & 0xFFFFFFFFFFFFF000);
    uint32_t ins = 0x90000010;
    uintptr_t cursor = 0;

    if (off < 0)
    {
        intptr_t coff = -off;
        int32_t count1000 = ((coff >> 12) % 4) & 3;
        int32_t count4000 = (((coff >> 12) / 5) & 0x3FFFF) + 1;

        count1000 = (-count1000) & 3;
        count4000 = (-count4000) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5) | 0x800000;
    }
    else
    {
        int32_t count1000 = ((off >> 12) % 4) & 3;
        int32_t count4000 = ((off >> 12) / 4) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5);
    }

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    unsigned int oldprotect = 0;
    UnprotectMemory(at, true, oldprotect);
    WriteMemory<uint32_t>(at, ins, false, false);
    cursor += sizeof(uint32_t);

    // assemble ADD X16, X16, #(addr & 0xFFF)
    ins = 0x91000210;
    off = (uintptr_t)(dest.get<void>()) & 0xFFF;
    ins |= (off << 10);

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    // BR X16
    ins = 0xD61F0200;
    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    ProtectMemory(at, oldprotect);

    return;
}

/*
 *  MakeBRPointer
 *      Creates BR instructions at address @at that jumps into address @@dest with registers X16 and X17
 */
inline void MakeBRPointer(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return;

    if ((uintptr_t)dest.get<void>() % 4)
        return;

    // assemble ADRP X16, addr & 0xF000
    intptr_t off = ((uintptr_t)(dest.get<void>()) & 0xFFFFFFFFFFFFF000) - ((uintptr_t)(at.get<void>()) & 0xFFFFFFFFFFFFF000);
    uint32_t ins = 0x90000010;
    uintptr_t cursor = 0;

    if (off < 0)
    {
        intptr_t coff = -off;
        int32_t count1000 = ((coff >> 12) % 4) & 3;
        int32_t count4000 = (((coff >> 12) / 5) & 0x3FFFF) + 1;

        count1000 = (-count1000) & 3;
        count4000 = (-count4000) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5) | 0x800000;
    }
    else
    {
        int32_t count1000 = ((off >> 12) % 4) & 3;
        int32_t count4000 = ((off >> 12) / 4) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5);
    }

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    unsigned int oldprotect = 0;
    UnprotectMemory(at, true, oldprotect);
    WriteMemory<uint32_t>(at, ins, false, false);
    cursor += sizeof(uint32_t);

    // assemble LDR X17, [X16, #(addr & 0xFFF)]
    ins = 0xF9400211;
    off = (uintptr_t)(dest.get<void>()) & 0xFFF;
    off = off >> 3;
    ins |= (off << 10);

    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    // BR X17
    ins = 0xD61F0220;
    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    ProtectMemory(at, oldprotect);

    return;
}

/*
 *  MakeBLR
 *      Creates BLR instructions at address @at that jumps into address @dest with register X16
 */
inline void MakeBLR(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return;

    if ((uintptr_t)dest.get<void>() % 4)
        return;

    // assemble ADRP X16, addr & 0xF000
    intptr_t off = ((uintptr_t)(dest.get<void>()) & 0xFFFFFFFFFFFFF000) - ((uintptr_t)(at.get<void>()) & 0xFFFFFFFFFFFFF000);
    uint32_t ins = 0x90000010;
    uintptr_t cursor = 0;

    if (off < 0)
    {
        intptr_t coff = -off;
        int32_t count1000 = ((coff >> 12) % 4) & 3;
        int32_t count4000 = (((coff >> 12) / 5) & 0x3FFFF) + 1;

        count1000 = (-count1000) & 3;
        count4000 = (-count4000) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5) | 0x800000;
    }
    else
    {
        int32_t count1000 = ((off >> 12) % 4) & 3;
        int32_t count4000 = ((off >> 12) / 4) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5);
    }

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    unsigned int oldprotect = 0;
    UnprotectMemory(at, true, oldprotect);
    WriteMemory<uint32_t>(at, ins, false, false);
    cursor += sizeof(uint32_t);

    // assemble ADD X16, X16, #(addr & 0xFFF)
    ins = 0x91000210;
    off = (uintptr_t)(dest.get<void>()) & 0xFFF;
    ins |= (off << 10);

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    // BLR X16
    ins = 0xD63F0200;
    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    ProtectMemory(at, oldprotect);

    return;
}

/*
 *  MakeBLRPointer
 *      Creates BLR instructions at address @at that jumps into address @@dest with registers X16 and X17
 */
inline void MakeBLRPointer(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true)
{
    if ((uintptr_t)at.get<void>() % 4)
        return;

    if ((uintptr_t)dest.get<void>() % 4)
        return;

    // assemble ADRP X16, addr & 0xF000
    intptr_t off = ((uintptr_t)(dest.get<void>()) & 0xFFFFFFFFFFFFF000) - ((uintptr_t)(at.get<void>()) & 0xFFFFFFFFFFFFF000);
    uint32_t ins = 0x90000010;
    uintptr_t cursor = 0;

    if (off < 0)
    {
        intptr_t coff = -off;
        int32_t count1000 = ((coff >> 12) % 4) & 3;
        int32_t count4000 = (((coff >> 12) / 5) & 0x3FFFF) + 1;

        count1000 = (-count1000) & 3;
        count4000 = (-count4000) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5) | 0x800000;
    }
    else
    {
        int32_t count1000 = ((off >> 12) % 4) & 3;
        int32_t count4000 = ((off >> 12) / 4) & 0x3FFFF;

        ins |= (count1000 << 29) | (count4000 << 5);
    }

    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    unsigned int oldprotect = 0;
    UnprotectMemory(at, true, oldprotect);
    WriteMemory<uint32_t>(at, ins, false, false);
    cursor += sizeof(uint32_t);

    // assemble LDR X17, [X16, #(addr & 0xFFF)]
    ins = 0xF9400211;
    off = (uintptr_t)(dest.get<void>()) & 0xFFF;
    off = off >> 3;
    ins |= (off << 10);

    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    // BLR X17
    ins = 0xD63F0220;
    //LOGD("writing: off: 0x%llX ins: 0x%X\n", (unsigned long long)(at.get<void>()) + cursor, ins);
    WriteMemoryNoTr<uint32_t>((uintptr_t)(at.get<void>()) + cursor, ins, false, false);
    cursor += sizeof(uint32_t);

    ProtectMemory(at, oldprotect);

    return;
}

/*
 *  MakeNOP
 *      Creates a bunch of NOP instructions at address @at
 */
inline void MakeNOP(memory_pointer_tr at, size_t count = 1, bool vp = true, bool exec = true)
{
    uintptr_t calcaddr = (uintptr_t)at.get<void>();
    for (int i = 0; i < count; i++)
    {
        WriteMemory<uint32_t>(calcaddr, 0xD503201F, vp, exec);
        calcaddr += sizeof(uint32_t);
    }
}


/*
 *  MakeRET
 *      Creates a RET instruction at address @at
 */
inline void MakeRET(memory_pointer_tr at, bool vp = true, bool exec = true)
{
    WriteMemory<uint32_t>(at, 0xD65F03C0, vp, exec);
}





/*
  *  lazy_pointer
  *      Lazy pointer, where it's final value will get evaluated only once when finally needed.
  */
 template<uintptr_t addr>
 struct lazy_pointer
 {
    public:
         // Returns the final raw pointer
         static auto_pointer get()
         {
             return xget().get();
         }

         template<class T>
         static T* get()
         {
             return get().template get<T>();
         }

    private:
        // Returns the final pointer
        static memory_pointer_raw xget()
        {
            static void* ptr = nullptr;
            if(!ptr) ptr = memory_pointer(addr).get();
            return memory_pointer_raw(ptr);
        }
};

 /*
  *  lazy_object
  *      Lazy object, where it's final object will get evaluated only once when finally needed.
  */
 template<uintptr_t addr, class T>
 struct lazy_object
 {
     static T& get()
     {
         static T data;
         static bool has_data = false;
         if(!has_data)
         {
             ReadObject<T>(addr, data, true);
             has_data = true;
         }
         return data;
     }
 };


 /*
    Helpers
 */

 template<class T>
inline memory_pointer  mem_ptr(T p)
{
    return memory_pointer(p);
}

template<class T>
inline memory_pointer_raw  raw_ptr(T p)
{
    return memory_pointer_raw(p);
}

template<class Tr>
inline memory_pointer_raw  raw_ptr(basic_memory_pointer<Tr> p)
{
    return raw_ptr(p.get());
}

template<uintptr_t addr>
inline memory_pointer_raw  lazy_ptr()
{
    return lazy_pointer<addr>::get();
}

} // namespace 

