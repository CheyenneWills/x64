; Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
; 
; This file is part of Macro SPITBOL.
; 
;     Macro SPITBOL is free software: you can redistribute it and/or modify
;     it under the terms of the GNU General Public License as published by
;     the Free Software Foundation, either version 3 of the License, or
;     (at your option) any later version.
; 
;     Macro SPITBOL is distributed in the hope that it will be useful,
;     but WITHOUT ANY WARRANTY; without even the implied warranty of
;     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;     GNU General Public License for more details.
; 
;     You should have received a copy of the GNU General Public License
;     along with Macro SPITBOL.  If not, see <http://www.gnu.org/licenses/>.
;
; First segment in program.  Contains serial number string.
; If external functions are included, a call to the external
; function will appear in this segment as well, placed here
; by the code in load.asm.
;
        %include        "systype.nh"

        segment	.data
        align         4
	global	hasfpu
hasfpu:	dd	0
	global	cprtmsg
cprtmsg: db " Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer."
