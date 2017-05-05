/*
 * Copyright (c) 2013-2017, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <uvisor.h>
#include "context.h"
#include "debug.h"
#include "vmpu.h"

/* FIXME: Currently it is not possible to return to a regular execution flow
 *        after the execution of the debug box handler. */
void debug_deprivilege_and_return(void * debug_handler, void * return_handler,
                                  uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    /* Source box: Get the current stack pointer. */
    /* Note: The source stack pointer is only used to assess the stack
     *       alignment and to read the xpsr. */
    uint32_t src_sp = context_validate_exc_sf(__get_PSP());

    /* Destination box: The debug box. */
    uint8_t dst_id = g_debug_box.box_id;

    /* Copy the xPSR from the source exception stack frame. */
    uint32_t xpsr = vmpu_unpriv_uint32_read((uint32_t) &((uint32_t *) src_sp)[7]);

    /* Destination box: Forge the destination stack frame. */
    /* Note: We manually have to set the 4 parameters on the destination stack,
     *       so we will set the API to have nargs=0. */
    uint32_t dst_sp = context_forge_exc_sf(src_sp, dst_id, (uint32_t) debug_handler, (uint32_t) return_handler, xpsr, 0);
    ((uint32_t *) dst_sp)[0] = a0;
    ((uint32_t *) dst_sp)[1] = a1;
    ((uint32_t *) dst_sp)[2] = a2;
    ((uint32_t *) dst_sp)[3] = a3;

    /* Suspend the OS. */
    g_priv_sys_hooks.priv_os_suspend();

    context_switch_in(CONTEXT_SWITCH_FUNCTION_DEBUG, dst_id, src_sp, dst_sp);

    /* Upon execution return debug_handler will be executed. Upon return from
     * debug_handler, return_handler will be executed. */
    return;
}
