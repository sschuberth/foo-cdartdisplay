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

#include "helpers.h"

namespace helpers
{
    static bool match_menu_command(const pfc::string_base & path, const char * command, t_size command_len = ~0)
    {
        if (command_len == ~0)
            command_len = strlen(command);

        if (command_len == path.get_length())
        {
            if (_stricmp(command, path) == 0)
                return true;
        }
        else if (command_len < path.get_length())
        {
            if ((path[path.get_length() - command_len - 1] == '/') &&
                (_stricmp(path.get_ptr() + path.get_length() - command_len, command) == 0))
                return true;
        }

        return false;
    }

    // p_out must be NULL
    static bool find_context_command_recur(const char * p_command, pfc::string_base & p_path, contextmenu_node * p_parent, contextmenu_node *& p_out)
    {
        if (p_parent != NULL && p_parent->get_type() == contextmenu_item_node::TYPE_POPUP)
        {
            for (t_size child_id = 0; child_id < p_parent->get_num_children(); ++child_id)
            {
                pfc::string8_fast path;
                contextmenu_node * child = p_parent->get_child(child_id);

                if (child)
                {
                    path = p_path;
                    path += child->get_name();

                    switch (child->get_type())
                    {
                    case contextmenu_item_node::TYPE_POPUP:
                        path += "/";

                        if (find_context_command_recur(p_command, path, child, p_out))
                            return true;

                        break;

                    case contextmenu_item_node::TYPE_COMMAND:
                        if (match_menu_command(path, p_command))
                        {
                            p_out = child;
                            return true;
                        }
                        break;
                    }
                }
            }
        }

        return false;
    }

    extern bool execute_context_command_by_name(const char * p_name, metadb_handle_list_cref p_handles)
    {
        contextmenu_node * node = NULL;

        service_ptr_t<contextmenu_manager> cm;
        pfc::string8_fast dummy("");

        contextmenu_manager::g_create(cm);

        if (p_handles.get_count() > 0)
        {
            cm->init_context(p_handles, 0);
        }
        else
        {
            cm->init_context_now_playing(0);
        }

        if (!find_context_command_recur(p_name, dummy, cm->get_root(), node))
            return false;

        node->execute();
        return true;
    }
}
