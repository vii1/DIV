;ÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜ
;
;                          ßş MaLiCe VeSa LiBrARy şß
;
;
;                            by MaLiCe / WitchCraft
;
;                                 August 1998 
;
;ÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜ


		CPU	386

		SEGMENT	STACK 	ALIGN=16 USE32
			resd	200h


; ú ÄÄÍÍ´ şDATAş ÃÍÍÍÍÄÄÄÄÄÄÄ ú   ú
		SEGMENT	DATA	ALIGN=2 USE32

		GLOBAL	_VbeInfoBlock, _ModeInfoBlock

        struc	VBEINFOBLOCK

        .VbeSignature    resb 4 ;db 'VBE2'       ; VBE Signature
        .VbeVersion:	resw	1        ; VBE Version
        .OemStringPtr:	resd	1            ; Pointer to OEM String
        .Capabilities:	resb	4     ; Capabilities of graphics cont.
        .VideoModePtr:	resd	1            ; Pointer to Video Mode List
        .TotalMemory:	resw	1            ; Number of 64kb memory blocks

        ; Added for VBE 2.0

        .OemSoftwareRev:	resw	1    ; VBE implementation Software revision
        .OemVendorNamePtr:	resd	1    ; Pointer to Vendor Name String
        .OemProductNamePtr:	resd	1    ; Pointer to Product Name String
        .OemProductRevPtr:	resd	1    ; Pointer to Product Revision String
        .Reserved:	resb	222   ; Reserved for VBE implementation

        .OemData:	resb	256   ; Data Area for OEM Strings

        endstruc	;VBEINFOBLOCK

        ALIGN 4
        _VbeInfoBlock:   ;VBEINFOBLOCK struct
        istruc	VBEINFOBLOCK
            at VBEINFOBLOCK.VbeSignature, db "VBE2"
        iend
        VbeTmpBlock     resd	1            ;Linear adr. for temporary block
        VbeTmpSel    	resw	1		;Selector for temporary block
        VbeDataBuffer	resb 1024	;Buffer for video modes, etc...

        VBEINFOBLOCK_SIZE   equ VbeTmpBlock-_VbeInfoBlock


        struc	MODEINFOBLOCK

        ; Mandatory information for all VBE revisions:

        .ModeAttributes:	resw	1    ; mode attributes
        .WinAAttributes:	resb	1    ; window A attributes
        .WinBAttributes:	resb	1    ; window B attributes
        .WinGranularity:	resw	1    ; window granularity
        .WinSize:	resw	1    ; window size
        .WinASegment:	resw	1    ; window A start segment
        .WinBSegment:	resw	1    ; window B start segment
        .WinFuncPtr:	resd	1    ; pointer to window function
        .BytesPerScanLine:	resw	1    ; bytes per scan line

        ; Mandatory information for VBE 1.2 and above:

        .XResolution:	resw	1    ; horizontal resolution in pixels or chars
        .YResolution:	resw	1    ; vertical resolution in pixels or chars
        .XCharSize:	resb	1    ; character cell width in pixels
        .YCharSize:	resb	1    ; character cell height in pixels
        .NumberOfPlanes:	resb	1    ; number of memory planes
        .BitsPerPixel:	resb	1    ; bits per pixel
        .NumberOfBanks:	resb	1    ; number of banks
        .MemoryModel:	resb	1    ; memory model type
        .BankSize:	resb	1    ; bank size in KB
        .NumberOfImages:	resb	1    ; number of images
        ._Reserved:	resb	1    ; reserved for page function

        ; Direct Color fields (required for direct/6 and YUV/7 memory models)

        .RedMaskSize:	resb	1    ; size of direct color red mask (bits)
        .RedFieldPosition:	resb	1    ; bit position of lsb of red mask
        .GreenMaskSize:	resb	1    ; size of direct color green mask
        .GreenFieldPosition:	resb	1    ; bit position of lsb of green mask
        .BlueMaskSize:	resb	1    ; size of direct color blue mask
        .BlueFieldPosition:	resb	1    ; bit position of lsb of blue mask
        .RsvdMaskSize:	resb	1    ; size of direct color reserved mask
        .RsvdFieldPosition:	resb	1    ; bit position of lsb of reserved mask
        .DirectColorModeInfo:	resb	1    ; direct color mode attributes

        ; Mandatory information for VBE 2.0 and above:

        .PhysBasePtr:	resd	1    ; physical address for LFB
        .OffScreenMemOffset:	resd	1    ; pointer to start of off screen memory
        .OffScreenMemSize:	resw	1    ; amount of off screen memory in K's
        .__Reserved:	resb	206

        endstruc	;MODEINFOBLOCK

        ALIGN 4
        _ModeInfoBlock: ;MODEINFOBLOCK struct
        istruc	MODEINFOBLOCK
        	resw 1
        iend
        ModeTmpBlock    resd	1            ;Pointer to temporary block
        ModeTmpSel	resw	1		;Selector of temporary block

        MODEINFOBLOCK_SIZE  equ ModeTmpBlock-_ModeInfoBlock


        struc	VBESCREEN

        .xres:	resw	1            ; screen width in pixels
        .yres:	resw	1		; screen heigth in pixels
        .ssize:	resd	1		; screen size in bytes
        .adr:	resd	1            ; address (of LFB, if not virtual)
        .handle:	resd	1		; mem handle (only for virtuals)

        endstruc	;VBESCREEN



        RMREGS:
         	rm_edi		resd	1
             	rm_esi		resd	1
        	rm_ebp		resd	1
        	rm_esp		resd	1
        	rm_ebx		resd	1
        	rm_edx		resd	1
        	rm_ecx		resd	1
        	rm_eax		resd	1

        	rm_flags	resw	1
        	rm_es		resw	1
        	rm_ds		resw	1
        	rm_fs		resw	1
        	rm_gs		resw	1
        	rm_ip		resw	1
        	rm_cs		resw	1
        	rm_sp		resw	1
        	rm_ss		resw	1

        	rm_spare_data	resd	20

        TmpVar	resw	1



; ú ÄÄÍÍ´ şCODEş ÃÍÍÍÍÄÄÄÄÄÄÄ ú   ú
		SEGMENT	CODE	ALIGN=2 USE32

		GLOBAL	vbeInit_,	vbeGetModeInfo_,	vbeSetMode_
		GLOBAL	vbeSetVirtual_,	vbeFreeVirtual_
		GLOBAL	vbeFlip_,	vbeClearScreen_
		GLOBAL	vbeSetVGAMode_,	vbeGetVGAMode_
		GLOBAL	vbePutPixel_
		GLOBAL	vbeSetScanWidth_,vbeGetScanWidth_
		GLOBAL	vbeSetStart_,	vbeGetStart_
		GLOBAL	vbeWR_

;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Seg2Linear:		; converts segmented address 2 linear address
 		push edx
 		mov edx,eax		; needs rm SEG:OFS pair in eax
 		and edx,0ffffh		; isolate offset part
 		shr eax,16
 		and eax,0ffffh		; and segment part
 		shl eax,4	 	; make it a 20 bit address
 		add eax,edx		; rets linear value in eax
 		pop edx
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        vbeInit_:   		;Checks VBE 2.0 availability
                pushad

                xor eax,eax
                mov ax,100h		; request low mem for the VbeTmpBlock
                mov bx,VBEINFOBLOCK_SIZE
                shr bx,4		; number of paragraphs
                int 31h			; call DPMI functions
                jnc @vbeInit1

                popad
                mov eax,2		; return 'Out of memory'
                retn

	@vbeInit1:
		mov [VbeTmpSel],dx	; save selector
                mov edx,eax	  	; ax - real mode seg.
                and edx,0ffffh
                shl edx,4	  	; edx - linear address
                mov [VbeTmpBlock],edx

                ; call VESA function 00h in real mode
                mov edi,RMREGS
                mov DWORD [rm_eax],4f00h	; VESA function 00h
                mov [rm_es],ax		; seg address of VbeTmpBlock
                mov DWORD [rm_edi],0

                mov eax,300h		; DPMI simulate rm int
                mov bl,10h		; interrupt number
                xor bh,bh
                xor ecx,ecx
                int 31h
                jnc @vbeInit2

                mov ax,101h
                mov dx,[VbeTmpSel]
                int 31h			; DPMI free mem
                popad
                mov eax,3		; return 'DPMI error'
                retn

	@vbeInit2:
		mov eax,[rm_eax]
		cmp al,4fh		; Check if VBE 2.0 exists
		je @vbeInit3

                mov ax,101h
                mov dx,[VbeTmpSel]
                int 31h			; DPMI free mem
		popad
		mov eax,1		; return 'VBE 2.0 not installed'
		retn

	@vbeInit3:
		mov esi,[VbeTmpBlock]
		cmp dword [esi+VBEINFOBLOCK.VbeSignature],'ASEV'
		je @vbeInit4

                mov ax,101h
                mov dx,[VbeTmpSel]
                int 31h	 		; DPMI free mem
	       	popad
		mov eax,1
		retn

	@vbeInit4:
		mov edi,_VbeInfoBlock   	; copy data to our struct
		mov ecx,VBEINFOBLOCK_SIZE
		rep movsb

		; now copy data such as video modes to a local buffer
		mov edi,VbeDataBuffer

		mov esi,[VbeTmpBlock]
		mov eax,[esi+VBEINFOBLOCK.VideoModePtr]
		call Seg2Linear
		mov esi,eax			;esi - linear address

		mov [_VbeInfoBlock+VBEINFOBLOCK.VideoModePtr],edi	; set new address

	@vbeInit5:
		lodsw  		 	; copy list of supp'ed video modes
		cmp ax,-1
		je @vbeInit6
		stosw
	 	jmp @vbeInit5
	@vbeInit6:
		stosw

		; copy the OEM string
		mov esi,[VbeTmpBlock]
		mov eax,[esi+VBEINFOBLOCK.OemStringPtr]
		call Seg2Linear
		mov esi,eax

		mov [_VbeInfoBlock+VBEINFOBLOCK.OemStringPtr],edi	; set new address

	@vbeInit7:
		lodsb
		cmp al,0
		je @vbeInit8
		stosb
	 	jmp @vbeInit7
	@vbeInit8:
		stosb

		; now copy OEM vendor name
		mov esi,[VbeTmpBlock]
		mov eax,[esi+VBEINFOBLOCK.OemVendorNamePtr]
		call Seg2Linear
 		mov esi,eax

		mov [_VbeInfoBlock+VBEINFOBLOCK.OemVendorNamePtr],edi

	@vbeInit9:
		lodsb
		cmp al,0
		je @vbeInit10
		stosb
	 	jmp @vbeInit9
	@vbeInit10:
		stosb

		; copy OEM Product Name
		mov esi,[VbeTmpBlock]
		mov eax,[esi+VBEINFOBLOCK.OemProductNamePtr]
		call Seg2Linear
		mov esi,eax

		mov [_VbeInfoBlock+VBEINFOBLOCK.OemProductNamePtr],edi

	@vbeInit11:
		lodsb
		cmp al,0
		je @vbeInit12
		stosb
	 	jmp @vbeInit11
	@vbeInit12:
		stosb

		; and OEM Product revision
		mov esi,[VbeTmpBlock]
		mov eax,[esi+VBEINFOBLOCK.OemProductRevPtr]
		call Seg2Linear
		mov esi,eax

		mov [_VbeInfoBlock+VBEINFOBLOCK.OemProductRevPtr],edi

	@vbeInit13:
		lodsb
		cmp al,0
		je @vbeInit14
		stosb
	 	jmp @vbeInit13
	@vbeInit14:
		stosb

		; we should check if had overran the VbeDataBuffer

		; now free memory allocated for VbeTmpBlock
		mov ax,101h
 		mov dx,[VbeTmpSel]
 		int 31h

		popad
		xor eax,eax	 	; return success code
		retn			; and exit
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetModeInfo_:
		pushad

		push eax		; save mode number
		xor eax,eax
		mov ax,100h		; request low mem for ModeTmpBlock
		mov bx,MODEINFOBLOCK_SIZE
		shr bx,4		; number of paragraphs
		int 31h			; call DPMI functions
		jnc @vbeGMI1

		popad
		mov eax,2  		; return 'Out of memory'
		retn

	@vbeGMI1:
		mov [ModeTmpSel],dx	; save selector
		mov edx,eax	  	; ax - real mode seg.
		and edx,0ffffh
		shl edx,4	  	; edx - linear address
		mov [ModeTmpBlock],edx

		; call VESA function 01h in real mode
		mov edi,RMREGS
		mov DWORD [rm_eax],4f01h	; VESA function 01h
		mov [rm_es],ax   		; seg address of VbeTmpBlock
		mov DWORD [rm_edi],0
		mov DWORD [rm_ebx],0
		pop eax
		mov [rm_ecx],eax		; mode number

		mov eax,300h		; DPMI simulate rm int
		mov bl,10h  		; interrupt number
		xor bh,bh
		xor ecx,ecx
		int 31h
		jnc @vbeGMI2

		mov ax,101h
		mov dx,[ModeTmpSel]
		int 31h	   		; DPMI free mem
		popad
		mov eax,3  		; return 'DPMI error'
		retn

	@vbeGMI2:
		mov eax,[rm_eax]
		cmp al,4fh		; Check if VBE 2.0 exists
		je @vbeGMI3

		mov ax,101h
		mov dx,[ModeTmpSel]
		int 31h	  		; DPMI free mem
		popad
		mov eax,1	 	; return 'VBE 2.0 not installed'
		retn

	@vbeGMI3:
		mov esi,[ModeTmpBlock]	; copy data to local struct
		mov edi,_ModeInfoBlock
		mov ecx,MODEINFOBLOCK_SIZE
		rep movsb

		mov al,[_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel]	; check if 15bpp
		cmp al,16
		jne @vbeGMI4

		mov al,[_ModeInfoBlock+MODEINFOBLOCK.RsvdMaskSize]
		cmp al,1
		jne @vbeGMI4
		mov BYTE [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel],15

	@vbeGMI4:
		mov ax,101h
		mov dx,[ModeTmpSel]	; free mem
		int 31h
		popad
		xor eax,eax		; return success code
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetMode_:
		pushad

		xchg ebx,ecx			; ebx - pointer to vbeScreen
		mov [ebx+VBESCREEN.xres],ax
		mov [ebx+VBESCREEN.yres],dx

		mov esi,[_VbeInfoBlock+VBEINFOBLOCK.VideoModePtr]

	@vbeSetMode1:
		xor eax,eax
		lodsw
		cmp ax,-1
		jnz @vbeSetMode3

		popad
		mov eax,4		; return 'Mode not supported'
		retn

	@vbeSetMode3:
		mov [TmpVar],ax		; save mode number
		call vbeGetModeInfo_
		or eax,eax
		jne @vbeSetMode1 	; if not supp'ed, try next mode

	@vbeSetMode2:
		; check this is the mode we wanted
		mov ax,[ebx+VBESCREEN.xres]
		cmp [_ModeInfoBlock+MODEINFOBLOCK.XResolution],ax
		jne @vbeSetMode1
		mov ax,[ebx+VBESCREEN.yres]
		cmp [_ModeInfoBlock+MODEINFOBLOCK.YResolution],ax
		jne @vbeSetMode1
		cmp [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel],cl
		jne @vbeSetMode1
		mov ebp,ebx		; preserve address

		; this is the mode, try setting with LFB
		mov ax,4F02h		; function 02h - set video mode
		mov bx,[TmpVar]
		or bx,4000h
		int 10h

		or ah,ah
		jz @vbeSetMode4

		popad
		mov eax,4
		retn

	@vbeSetMode4:

		; map LFB to accessible mem
		mov ebx,[_ModeInfoBlock+MODEINFOBLOCK.PhysBasePtr]
		mov ecx,ebx
		shr ebx,16		; bx:cx - physical address
		movzx esi,WORD [_VbeInfoBlock+VBEINFOBLOCK.TotalMemory]
		shl esi,6
		shl esi,10		; TM * 64 * 1024 = Total video mem
		mov edi,esi
		shr esi,16		; si:di - region size

		mov ax,800h
		int 31h			; call DPMI

		jnc @vbeSetMode5
		popad
		mov eax,2		; return memory error
		retn

	@vbeSetMode5:

		shl ebx,16
		mov bx,cx
		mov [ebp+VBESCREEN.adr],ebx	; save new LFB address

		mov ax,[ebp+VBESCREEN.yres]
		xor edx,edx
		mov dx,[_ModeInfoBlock+MODEINFOBLOCK.BytesPerScanLine]
		mul dx
		shl edx,16
		mov dx,ax
		mov [ebp+VBESCREEN.ssize],edx

    call vbeGetScanWidth_
    or ax,ax
    je @vbeSetMode6
    mov bx,ax
		movzx eax,WORD [ebp+VBESCREEN.xres]
    cmp ax,bx
    je @vbeSetMode6

		call vbeSetScanWidth_

  @vbeSetMode6:

		popad
		xor eax,eax		; return success
		retn

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetVirtual_:
		pushad

		mov [ebx+VBESCREEN.xres],ax
		mov [ebx+VBESCREEN.yres],dx

		xor edx,edx
		mov dl,[_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel]
                inc dl			; just in case its 15bpp
		shr dl,3		; dl - bytes per pixel
		mul dx
		ror eax,16
		mov ax,dx
		ror eax,16		; ax - bytes per line

		mov dx,[ebx+VBESCREEN.yres]
		mul dx
		shl edx,16
		mov dx,ax      		; (xres * bytespp) * yres
		mov [ebx+VBESCREEN.ssize],edx

		push ebx
		shr edx,16
		mov bx,dx
		mov cx,ax		; bx:cx - size of screen
		mov ax,501h
		int 31h			; allocate linear memory

		jnc @vbeSV1
		pop ebx
		popad
		mov ax,2		; return 'Out of memory'
		retn

	@vbeSV1:
		shl ebx,16
		mov bx,cx		; ebx - screen address
		mov eax,ebx
		pop ebx
		mov [ebx+VBESCREEN.adr],eax

		shl esi,16
		mov si,di 		; esi - memory handle
		mov [ebx+VBESCREEN.handle],esi

		popad
		xor eax,eax
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeFreeVirtual_:
		pushad

		mov ebx,eax
		mov esi,[ebx+VBESCREEN.handle]
		mov di,si
		shr esi,16		; si:di - block handle

		mov ax,502h
		int 31h			; free linear memory
		jc @vbeFV_error

		mov WORD [ebx+VBESCREEN.xres],0
		mov WORD [ebx+VBESCREEN.yres],0
		mov DWORD [ebx+VBESCREEN.ssize],0
		mov DWORD [ebx+VBESCREEN.adr],0

		popad
		xor eax,eax
		retn

	@vbeFV_error:
		popad
		mov eax,5		; return 'Memory allocation error'
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeFlip_:
		pushad

		mov ebx,eax
		mov esi,[ebx+VBESCREEN.adr]
		mov ebx,edx
		mov edi,[ebx+VBESCREEN.adr]

		mov ecx,[ebx+VBESCREEN.ssize]
		shr ecx,2

		rep movsd
		popad
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbePutPixel_:
		pushad

		push eax
		mov ax,[_ModeInfoBlock+MODEINFOBLOCK.BytesPerScanLine]
		mul dx
		shl edx,16
		mov dx,ax		; edx - y * bpsl
		pop eax

		push edx
		movzx edx,BYTE [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel]
		inc dl	 		; just in case its 15bpp
		shr dl,3		; find out bytes per pixel
		mul dx
		shl edx,16
		mov dx,ax
		pop eax
		add eax,edx 		; eax - pixel offset
		push eax

		cmp BYTE [_ModeInfoBlock+MODEINFOBLOCK.MemoryModel],4	; 256 colors modes
		jne @vbePP1

		pop eax
		mov edi,[ebx+VBESCREEN.adr]
		add edi,eax
		mov [edi],cl
		jmp @vbePP_end

	@vbePP1:
		cmp BYTE [_ModeInfoBlock+MODEINFOBLOCK.MemoryModel],6	; Direct Color modes
		jne @vbePP_end

		mov ebp,ebx
		xor ebx,ebx
		push ecx

		mov eax,-1 			; build blue mask
		mov edx,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.BlueMaskSize]
		shl edx,cl
		xor eax,edx	      		; eax - blue mask
		and edi,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.BlueFieldPosition]
		shl edi,cl
		or ebx,edi

		mov eax,-1
		mov edx,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.GreenMaskSize]
		shl edx,cl
		xor eax,edx			; eax - green mask
		and esi,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.GreenFieldPosition]
		shl esi,cl
		or ebx,esi

		mov eax,-1
		mov edx,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.RedMaskSize]
		shl edx,cl
		xor eax,edx			; eax - red mask
		pop edx
		and edx,eax
		mov cl,[_ModeInfoBlock+MODEINFOBLOCK.RedFieldPosition]
		shl edx,cl
		or ebx,edx 			; ebx - composed pixel color

		mov eax,ebx
		mov ebx,[ebp+VBESCREEN.adr]
		pop edx
		add ebx,edx

		cmp BYTE [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel],16
		jnbe @vbePP2

		mov [ebx],ax
		jmp @vbePP_end

	@vbePP2:
		cmp BYTE [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel],24
		jne @vbePP3

		mov [ebx],ax
		shr eax,16
		mov [ebx+2],al
		jmp @vbePP_end

	@vbePP3:
		cmp BYTE [_ModeInfoBlock+MODEINFOBLOCK.BitsPerPixel],32
		jne @vbePP_end

		mov [ebx],eax

	@vbePP_end:
		popad
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeClearScreen_:
		pushad

		mov ebx,eax
		mov edi,[ebx+VBESCREEN.adr]

		mov ecx,[ebx+VBESCREEN.ssize]
		shr ecx,2
		xor eax,eax

		rep stosd

		popad
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetScanWidth_:
		pushad

		mov ecx,eax		; ecx - pixels per scan line
		xor ebx,ebx
		xor edx,edx

		mov ax,4f06h
		int 10h
		cmp al,4fh
		je @vbeSSW1

		popad
		mov eax,1
		retn

	@vbeSSW1:
		popad
		xor eax,eax
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetScanWidth_:
		pushad

		xor ecx,ecx
		xor edx,edx
		mov ebx,1
		mov ax,4f06h
		int 10h

		mov [TmpVar],cx

		cmp al,4fh
		je @vbeGSW1

		popad
		xor eax,eax			; failed
		retn

	@vbeGSW1:
		popad
		mov ax,[TmpVar]
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetStart_:
		pushad

		xor ebx,ebx
		mov ecx,eax		; ecx - x pos.  edx - y pos.
		mov ax,4f07h
		int 10h

		cmp al,4fh
		je @vbeSS1

		popad
		mov eax,1
		retn

	@vbeSS1:
		popad
		xor eax,eax
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetStart_:
		pushad

		push	eax		; save x & y adresses
		push	edx
		xor ecx,ecx
		xor edx,edx
		mov ebx,1
		mov ax,4f07h
		int 10h

		cmp al,4fh
		je @vbeGS1

		popad
		mov eax,1		; failed
		retn

	@vbeGS1:
		pop ebx
		mov [ebx],edx
		pop ebx
		mov [ebx],ecx		; save values

		popad
		xor eax,eax
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetVGAMode_:
		pushad

		xor ah,ah
		int 10h

		popad
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetVGAMode_:
		pushad

		mov ah,0fh
		int 10h
		mov [TmpVar],ax

		popad
		mov ax,[TmpVar]
		xor ah,ah
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeWR_:
		push	eax
		push	edx

		mov dx,03dah
	start:
		in al,dx
		test al,8
		jnz start
	_end:
		in al,dx
		test al,8
		jz _end

		pop	edx
		pop	eax
		retn
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´

		GROUP	DGROUP	STACK	DATA	CODE
;		END
