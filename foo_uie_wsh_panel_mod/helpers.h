/*
 * This file was taken from the foo_uie_wsh_panel component which is licensed
 * under the New BSD License, see <http://code.google.com/p/foo-wsh-panel-mod/>.
 *
 * Copyright (c) 2009-2011, ColdNeverEnd <ColdNeverEnd_AT_gmail_DOT_com>
 * All rights reserved.
 *
 * Modified by Sebastian Schuberth <sschuberth_AT_gmail_DOT_com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef HELPERS_H
#define HELPERS_H

// Get rid of tons of warnings.
#pragma warning(push,1)
#include <foobar2000/SDK/foobar2000.h>
#pragma warning(pop)

namespace helpers
{
    extern bool execute_context_command_by_name(const char * p_name, metadb_handle_list_cref p_handles);

    __declspec(noinline) static bool execute_context_command_by_name_SEH(const char * p_name, metadb_handle_list_cref p_handles)
    {
        bool ret = false;

        __try 
        {
            ret = execute_context_command_by_name(p_name, p_handles);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            ret = false;
        }

        return ret;
    }
}

#endif // HELPERS_H
