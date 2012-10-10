@
@  Copyright 2011-12 ARM Limited
@
@  Licensed under the Apache License, Version 2.0 (the "License");
@  you may not use this file except in compliance with the License.
@  You may obtain a copy of the License at
@
@      http://www.apache.org/licenses/LICENSE-2.0
@
@  Unless required by applicable law or agreed to in writing, software
@  distributed under the License is distributed on an "AS IS" BASIS,
@  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@  See the License for the specific language governing permissions and
@  limitations under the License.
@

@
@ NE10 Library : source/NE10_len.asm.s
@

        .text
        .syntax   unified

.include "Ne10/headers/NE10header.s"

        .balign   4
        .global   len_vec2f_asm
        .thumb
        .thumb_func

len_vec2f_asm:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @
        @ arm_result_t len_vec2f(arm_float_t * dst,
        @                 arm_vec2f_t * src, unsigned int count)
        @
        @  r0: *dst and current destination item's address
        @  r1: *src and current source item's address
        @  r2: int count
        @
        @  r2: loop counter
        @
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        cbz        r2, .LoopEndVec2F
        add        r0, r0, r2, lsl #2
        add        r1, r1, r2, lsl #3        @ r1 = r1 + count * 8

.LoopBeginVec2F:
        vldmdb     r1!, {s10-s11}
        vmul.f32   s14, s10, s10             @ s14 = x*x
        vmla.f32   s14, s11, s11             @ s14 = x*x + y*y
        vsqrt.f32  s15, s14                  @ s15 = sqrt( s14 )
        vstmdb     r0!, {s15}                @ store s15 in dst[ i ]=s15 and move dst to the next entry (4 bytes)
        subs       r2, r2, #1                @ decrement the loop counter
        bne        .LoopBeginVec2F           @ loop if r4 is still positive or zero
.LoopEndVec2F:
        mov        r0, NE10_OK             @ Return NE10_OK
        bx         lr




        .balign   4
        .global   len_vec3f_asm
        .thumb
        .thumb_func

len_vec3f_asm:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @
        @ arm_result_t len_vec3f(arm_float_t * dst,
        @                 arm_vec3f_t * src, unsigned int count)
        @
        @  r0: *dst and current destination item's address
        @  r1: *src and current source item's address
        @  r2: int count
        @
        @  r2: loop counter
        @
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        cbz        r2, .LoopEndVec3F
        add        r0, r0, r2, lsl #2
        add        r1, r1, r2, lsl #3        @ ...
        add        r1, r1, r2, lsl #2        @ r1 = r1 + count * 12

.LoopBeginVec3F:
        vldmdb     r1!, {s10-s12}
        vmul.f32   s14, s10, s10             @ s14 = x*x
        vmla.f32   s14, s11, s11             @ s14 = x*x + y*y
        vmla.f32   s14, s12, s12             @ s14 = x*x + y*y + z*z
        vsqrt.f32  s15, s14                  @ s15 = sqrt( s14 )
        vstmdb     r0!, {s15}                @ store s15 in dst[ i ]=s15 and move dst to the next entry (4 bytes)
        subs       r2, r2, #1                @ decrement the loop counter
        bne        .LoopBeginVec3F           @ loop if r4 is still positive or zero
.LoopEndVec3F:
        mov        r0, NE10_OK             @ Return NE10_OK
        bx         lr




        .balign   4
        .global   len_vec4f_asm
        .thumb
        .thumb_func

len_vec4f_asm:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @
        @ arm_result_t len_vec4f(arm_float_t * dst,
        @                 arm_vec4f_t * src, unsigned int count)
        @
        @  r0: *dst and current destination item's address
        @  r1: *src and current source item's address
        @  r2: int count
        @
        @  r2: loop counter
        @
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        cbz        r2, .LoopEndVec4F
        add        r0, r0, r2, lsl #2
        add        r1, r1, r2, lsl #4        @ r1 = r1 + count * 16

.LoopBeginVec4F:
        vldmdb     r1!, {s10-s13}
        vmul.f32   s14, s10, s10             @ s14 = x*x
        vmla.f32   s14, s11, s11             @ s14 = x*x + y*y
        vmla.f32   s14, s12, s12             @ s14 = x*x + y*y + z*z
        vmla.f32   s14, s13, s13             @ s14 = x*x + y*y + z*z + w*w
        vsqrt.f32  s15, s14                  @ s15 = sqrt( s14 )
        vstmdb     r0!, {s15}                @ store s15 in dst[ i ]=s15 and move dst to the next entry (4 bytes)
        subs       r2, r2, #1                @ decrement the loop counter
        bne        .LoopBeginVec4F           @ loop if r4 is still positive or zero
.LoopEndVec4F:
        mov        r0, NE10_OK             @ Return NE10_OK
        bx         lr
