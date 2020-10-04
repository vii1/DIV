_SMALL_CODE     =       00h
_BIG_CODE       =       01h
_SMALL_DATA     =       00h
_BIG_DATA       =       02h
_HUGE_DATA      =       04h

_TINY           =       08h
_SMALL          =       ( _SMALL_CODE or _SMALL_DATA )
_COMPACT        =       ( _SMALL_CODE or _BIG_DATA   )
_MEDIUM         =       ( _BIG_CODE   or _SMALL_DATA )
_LARGE          =       ( _BIG_CODE   or _BIG_DATA   )
_HUGE           =       ( _BIG_CODE   or _HUGE_DATA  )

ifdef __TINY__
    _MODEL = _TINY
endif
ifdef __SMALL__
    _MODEL = _SMALL
endif
ifdef __COMPACT__
    _MODEL = _COMPACT
endif
ifdef __MEDIUM__
    _MODEL = _MEDIUM
endif
ifdef __LARGE__
    _MODEL = _LARGE
endif
ifdef __HUGE__
    _MODEL = _HUGE
endif

if _MODEL and _BIG_CODE

modstart	macro	modname
		name	modname
_TEXT		segment word public 'CODE'
		assume	cs:_TEXT
		endm

codeptr 	macro	name
		extrn	name : dword
		endm

docall		macro	dest
		call	far ptr dest
		endm

xdefp		macro	xsym
		public	xsym
		endm

defp		macro	dsym
	 dsym	proc	far
		endm

xref		macro	dsym
		extrn	dsym : far
		endm

endproc 	macro	dsym
	 dsym	endp
		endm

else

modstart	macro	modname
		name	modname
_TEXT		segment word public 'CODE'
		assume	cs:_TEXT
		endm

codeptr 	macro	name
		extrn	name : word
		endm

docall		macro	dest
		call	dest
		endm

xdefp		macro	xsym
		public	xsym
		endm

defp		macro	dsym
	 dsym	proc	near
		endm

xref		macro	dsym
		extrn	dsym : near
		endm

endproc 	macro	dsym
	 dsym	endp
		endm

endif

endmod		macro
_TEXT		ends
		endm

datasegment	macro
_DATA		segment word public 'DATA'
DGROUP		group _DATA
		assume	ds:DGROUP
		endm

enddata 	macro
_DATA		ends
		endm
bss_segment	macro
_BSS		segment word public 'BSS'
DGROUP		group _BSS
		assume	ds:DGROUP
		endm

endbss		macro
_BSS		ends
		endm
