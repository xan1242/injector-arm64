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
#include <cstdint>
#include <cstdio>

namespace injector
{
    unsigned long base_addr;

    void SetGameBaseAddress(unsigned long addr)
    {
        base_addr = addr;
    }

#if 1       // GVM and Address Translator, Not very interesting for the users, so skip reading those...
    
/*
 *  game_version_manager
 *      Detects the game, the game version and the game region
 *      This assumes the executable is decrypted, so, Silent's ASI Loader is recommended.
 */
class game_version_manager
{
public:
    bool Detect() { return true; }
};


/*
 *  address_manager
 *      Address translator from 1.0 executables to other executables offsets
 *      Inherits from game_version_manager ;)
 */
class address_manager : public game_version_manager
{
    private:
        address_manager()
        {
            this->Detect();
        }
        
        void* translator(void* p) { return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(p) + base_addr)); }

    public:
        // Translates address p to the running executable pointer
        void* translate(void* p)
        {
            return translator(p);
        }
        
        
    public:
        // Address manager singleton
        static address_manager& singleton()
        {
            static address_manager m;
            return m;
        }
        
        // Static version of translate()
        static void* translate_address(void* p)
        {
            return singleton().translate(p);
        }
        
        
    public:
        // Functors for memory translation:

        // Translates aslr translator
        //struct fn_mem_translator_aslr
        //{
        //    void* operator()(void* p) const
        //    {
        //        static uintptr_t  module = base_addr;
        //        return (void*)((uintptr_t)(p)-(module));
        //    }
        //};

        // Translates nothing translator
        struct fn_mem_translator_nop
        {
            void* operator()(void* p) const
            { return p; }
        };
        
        // Real translator
        struct fn_mem_translator
        {
            void* operator()(void* p) const
            { return translate_address(p); }
        };
};

#endif  // #if 1


}