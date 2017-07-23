COMMENT~
ÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜ

                          ßş MaLiCe VeSa LiBrARy şß


                            by MaLiCe / WitchCraft

                                 August 1998 

ÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜ
~



		.386
		.MODEL FLAT
		
		.STACK
                
                
; ú ÄÄÍÍ´ şDATAş ÃÍÍÍÍÄÄÄÄÄÄÄ ú   ú
		.DATA
	        
		PUBLIC _VbeInfoBlock, _ModeInfoBlock
                                          
        VBEINFOBLOCK    STRUC
                            
        VbeSignature    db 'VBE2'       ; VBE Signature
        VbeVersion      dw 0200h        ; VBE Version
        OemStringPtr    dd ?            ; Pointer to OEM String
        Capabilities    db 4 dup (?)    ; Capabilities of graphics cont.
        VideoModePtr    dd ?            ; Pointer to Video Mode List
        TotalMemory     dw ?            ; Number of 64kb memory blocks
                            
        ; Added for VBE 2.0 
                                           
        OemSoftwareRev          dw ?    ; VBE implementation Software revision
        OemVendorNamePtr        dd ?    ; Pointer to Vendor Name String
        OemProductNamePtr       dd ?    ; Pointer to Product Name String
        OemProductRevPtr        dd ?    ; Pointer to Product Revision String
        Reserved        db 222 dup (?)  ; Reserved for VBE implementation
                                         
        OemData         db 256 dup (?)  ; Data Area for OEM Strings
                            
        VBEINFOBLOCK    ENDS
                            
        ALIGN 4                   
        _VbeInfoBlock VBEINFOBLOCK <>   ;VBEINFOBLOCK struct
        VbeTmpBlock     dd ?            ;Linear adr. for temporary block
        VbeTmpSel    	dw ?		;Selector for temporary block
        VbeDataBuffer	db 1024 DUP (?)	;Buffer for video modes, etc...
                     
                                                                      
          
        MODEINFOBLOCK   STRUC
         
        ; Mandatory information for all VBE revisions:
                                           
        ModeAttributes          dw ?    ; mode attributes
        WinAAttributes          db ?    ; window A attributes
        WinBAttributes          db ?    ; window B attributes
        WinGranularity          dw ?    ; window granularity
        WinSize                 dw ?    ; window size
        WinASegment             dw ?    ; window A start segment
        WinBSegment             dw ?    ; window B start segment
        WinFuncPtr              dd ?    ; pointer to window function
        BytesPerScanLine        dw ?    ; bytes per scan line
                
        ; Mandatory information for VBE 1.2 and above:
         
        XResolution     dw ?    ; horizontal resolution in pixels or chars
        YResolution     dw ?    ; vertical resolution in pixels or chars
        XCharSize       db ?    ; character cell width in pixels      
        YCharSize       db ?    ; character cell height in pixels
        NumberOfPlanes  db ?    ; number of memory planes
        BitsPerPixel    db ?    ; bits per pixel
        NumberOfBanks   db ?    ; number of banks
        MemoryModel     db ?    ; memory model type
        BankSize        db ?    ; bank size in KB
        NumberOfImages  db ?    ; number of images
        _Reserved       db ?    ; reserved for page function
             
        ; Direct Color fields (required for direct/6 and YUV/7 memory models)
                                         
        RedMaskSize             db ?    ; size of direct color red mask (bits)
        RedFieldPosition        db ?    ; bit position of lsb of red mask
        GreenMaskSize           db ?    ; size of direct color green mask
        GreenFieldPosition      db ?    ; bit position of lsb of green mask
        BlueMaskSize            db ?    ; size of direct color blue mask
        BlueFieldPosition       db ?    ; bit position of lsb of blue mask
        RsvdMaskSize            db ?    ; size of direct color reserved mask
        RsvdFieldPosition       db ?    ; bit position of lsb of reserved mask
        DirectColorModeInfo     db ?    ; direct color mode attributes
         
        ; Mandatory information for VBE 2.0 and above:
                                                
        PhysBasePtr             dd ?    ; physical address for LFB
        OffScreenMemOffset      dd ?    ; pointer to start of off screen memory
        OffScreenMemSize        dw ?    ; amount of off screen memory in K's
        __Reserved              db 206 dup (?)
         
        MODEINFOBLOCK   ENDS
                                           
        ALIGN 4 
        _ModeInfoBlock MODEINFOBLOCK <?> ;MODEINFOBLOCK struct
        ModeTmpBlock    dd ?            ;Pointer to temporary block
        ModeTmpSel	dw ?		;Selector of temporary block
                                                     
                                                     
                                                     
        VBESCREEN       STRUC
        
        xres            dw ?            ; screen width in pixels
        yres            dw ?		; screen heigth in pixels
        size		dd ?		; screen size in bytes
        adr             dd ?            ; address (of LFB, if not virtual)
        handle		dd ?		; mem handle (only for virtuals)
        
        VBESCREEN       ENDS                                           
        
        
                
        RMREGS	label	dword
         	rm_edi		dd	?
             	rm_esi		dd	?
        	rm_ebp		dd	?  
        	rm_esp		dd  	?
        	rm_ebx		dd	?
        	rm_edx		dd	?
        	rm_ecx		dd	?  
        	rm_eax		dd	?

        	rm_flags	dw	?
        	rm_es		dw	?
        	rm_ds		dw	?
        	rm_fs		dw	?
        	rm_gs		dw	?
        	rm_ip		dw	?
        	rm_cs		dw	?
        	rm_sp		dw	?
        	rm_ss		dw	?

        	rm_spare_data	dd	20 dup(?)

        TmpVar	dw ?



; ú ÄÄÍÍ´ şCODEş ÃÍÍÍÍÄÄÄÄÄÄÄ ú   ú
		.CODE

		PUBLIC vbeInit_,	vbeGetModeInfo_,	vbeSetMode_
		PUBLIC vbeSetVirtual_,	vbeFreeVirtual_
		PUBLIC vbeFlip_,	vbeClearScreen_
		PUBLIC vbeSetVGAMode_,	vbeGetVGAMode_
		PUBLIC vbePutPixel_
		PUBLIC vbeSetScanWidth_,vbeGetScanWidth_
		PUBLIC vbeSetStart_,	vbeGetStart_
		PUBLIC vbeWR_                                          
  
;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	Seg2Linear PROC		; converts segmented address 2 linear address
 		push edx                                          
 		mov edx,eax		; needs rm SEG:OFS pair in eax
 		and edx,0ffffh		; isolate offset part
 		shr eax,16                                            
 		and eax,0ffffh		; and segment part            
 		shl eax,4	 	; make it a 20 bit address    
 		add eax,edx		; rets linear value in eax    
 		pop edx                 
		ret                                                   
	Seg2Linear ENDP                 
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        vbeInit_	PROC   		;Checks VBE 2.0 availability
                pushad                                                 
                                         
                xor eax,eax              
                mov ax,100h		; request low mem for the VbeTmpBlock
                mov bx,size _VbeInfoBlock
                shr bx,4		; number of paragraphs
                int 31h			; call DPMI functions
                jnc @vbeInit1            
                                         
                popad                    
                mov eax,2		; return 'Out of memory'
                ret                      
                                         
	@vbeInit1:                                                    
		mov VbeTmpSel,dx	; save selector
                mov edx,eax	  	; ax - real mode seg.
                and edx,0ffffh                                
                shl edx,4	  	; edx - linear address
                mov VbeTmpBlock,edx      

                ; call VESA function 00h in real mode                  
                mov edi,offset RMREGS    
                mov rm_eax,4f00h	; VESA function 00h
                mov rm_es,ax		; seg address of VbeTmpBlock
                mov rm_edi,0             
                                                       
                mov eax,300h		; DPMI simulate rm int
                mov bl,10h		; interrupt number
                xor bh,bh                
                xor ecx,ecx              
                int 31h                  
                jnc @vbeInit2            
                                         
                mov ax,101h                                           
                mov dx,VbeTmpSel         
                int 31h			; DPMI free mem
                popad                          
                mov eax,3		; return 'DPMI error'
                ret                            
                                                       
	@vbeInit2:                                                     
		mov eax,rm_eax                               
		cmp al,4fh		; Check if VBE 2.0 exists
		je @vbeInit3                                 
		                                             
                mov ax,101h                                  
                mov dx,VbeTmpSel                             
                int 31h			; DPMI free mem      
		popad                                        
		mov eax,1		; return 'VBE 2.0 not installed' 
		ret                                                      
		                                                         
	@vbeInit3:
		mov esi,VbeTmpBlock                                      
		cmp dword ptr [esi].VBEINFOBLOCK.VbeSignature,'ASEV'     
		je @vbeInit4             
		                         
                mov ax,101h              
                mov dx,VbeTmpSel         
                int 31h	 		; DPMI free mem
	       	popad                                                    
		mov eax,1                                                
		ret                                                      
		                                                         
	@vbeInit4:                                                       
		mov edi,offset _VbeInfoBlock   	; copy data to our struct
		mov ecx,size _VbeInfoBlock
		rep movsb               
		
		; now copy data such as video modes to a local buffer
		mov edi,offset VbeDataBuffer
		
		mov esi,VbeTmpBlock                          
		mov eax,[esi].VBEINFOBLOCK.VideoModePtr
		call Seg2Linear
		mov esi,eax			;esi - linear address
		                        
		mov _VbeInfoBlock.VideoModePtr,edi	; set new address
		                        
	@vbeInit5:                                     
		lodsw  		 	; copy list of supp'ed video modes
		cmp ax,-1                                                 
		je @vbeInit6                                              
		stosw                                                     
	 	jmp @vbeInit5                                             
	@vbeInit6:
		stosw                                                     
		                                                          
		; copy the OEM string                                     
		mov esi,VbeTmpBlock                                       
		mov eax,[esi].VBEINFOBLOCK.OemStringPtr                   
		call Seg2Linear                                           
		mov esi,eax                                               
		                                                          
		mov _VbeInfoBlock.OemStringPtr,edi	; set new address
		                                                          
	@vbeInit7:                                                        
		lodsb                                                     
		cmp al,0                                                  
		je @vbeInit8                                              
		stosb                                                     
	 	jmp @vbeInit7                                             
	@vbeInit8:                                                        
		stosb            
		                 
		; now copy OEM vendor name                                
		mov esi,VbeTmpBlock                                       
		mov eax,[esi].VBEINFOBLOCK.OemVendorNamePtr
		call Seg2Linear  
 		mov esi,eax      
 				 
		mov _VbeInfoBlock.OemVendorNamePtr,edi
		                                                          
	@vbeInit9:               
		lodsb            
		cmp al,0         
		je @vbeInit10           
		stosb            
	 	jmp @vbeInit9
	@vbeInit10:                                    
		stosb                                                  
		                 
		; copy OEM Product Name
		mov esi,VbeTmpBlock
		mov eax,[esi].VBEINFOBLOCK.OemProductNamePtr
		call Seg2Linear  
		mov esi,eax                                
		                                           
		mov _VbeInfoBlock.OemProductNamePtr,edi
		                                 
	@vbeInit11:              
		lodsb            
		cmp al,0                                     
		je @vbeInit12    
		stosb            
	 	jmp @vbeInit11   
	@vbeInit12:                     
		stosb            
		                        
		; and OEM Product revision             
		mov esi,VbeTmpBlock                                    
		mov eax,[esi].VBEINFOBLOCK.OemProductRevPtr
		call Seg2Linear  
		mov esi,eax                                
		                                           
		mov _VbeInfoBlock.OemProductRevPtr,edi
		                 
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
 		mov dx,VbeTmpSel                                       
 		int 31h          
		                            
		popad                       
		xor eax,eax	 	; return success code
		ret			; and exit           
	vbeInit_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetModeInfo_	PROC                                        
		pushad                  
		                        
		push eax		; save mode number
		xor eax,eax                                         
		mov ax,100h		; request low mem for ModeTmpBlock
		mov bx,size _ModeInfoBlock                          
		shr bx,4		; number of paragraphs      
		int 31h			; call DPMI functions          
		jnc @vbeGMI1                                        
	                                                            
		popad                                               
		mov eax,2  		; return 'Out of memory'
		ret

	@vbeGMI1:
		mov ModeTmpSel,dx	; save selector
		mov edx,eax	  	; ax - real mode seg.
		and edx,0ffffh
		shl edx,4	  	; edx - linear address
		mov ModeTmpBlock,edx

		; call VESA function 01h in real mode               
		mov edi,offset RMREGS                               
		mov rm_eax,4f01h	; VESA function 01h
		mov rm_es,ax   		; seg address of VbeTmpBlock
		mov rm_edi,0            
		mov rm_ebx,0            
		pop eax                                                
		mov rm_ecx,eax		; mode number
		                                                    
		mov eax,300h		; DPMI simulate rm int      
		mov bl,10h  		; interrupt number          
		xor bh,bh                                           
		xor ecx,ecx                                         
		int 31h                                             
		jnc @vbeGMI2
		           
		mov ax,101h
		mov dx,ModeTmpSel
		int 31h	   		; DPMI free mem
		popad                                  
		mov eax,3  		; return 'DPMI error'
		ret             
		                                          
	@vbeGMI2:
		mov eax,rm_eax
		cmp al,4fh		; Check if VBE 2.0 exists
		je @vbeGMI3                                            
		                                          
		mov ax,101h                               
		mov dx,ModeTmpSel                         
		int 31h	  		; DPMI free mem   
		popad                                     
		mov eax,1	 	; return 'VBE 2.0 not installed'
		ret                                       
		                                          
	@vbeGMI3:
		mov esi,ModeTmpBlock	; copy data to local struct
		mov edi,offset _ModeInfoBlock
		mov ecx,size _ModeInfoBlock
		rep movsb
		
		mov al,_ModeInfoBlock.BitsPerPixel	; check if 15bpp
		cmp al,16
		jne @vbeGMI4
		
		mov al,_ModeInfoBlock.RsvdMaskSize
		cmp al,1                                               
		jne @vbeGMI4
		mov _ModeInfoBlock.BitsPerPixel,15
		
	@vbeGMI4:
		mov ax,101h
		mov dx,ModeTmpSel	; free mem
		int 31h
		popad
		xor eax,eax		; return success code
		ret
	vbeGetModeInfo_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetMode_	PROC
		pushad

		xchg ebx,ecx			; ebx - pointer to vbeScreen
		mov [ebx].xres,ax
		mov [ebx].yres,dx

		mov esi,_VbeInfoBlock.VideoModePtr

	@vbeSetMode1:
		xor eax,eax
		lodsw
		cmp ax,-1
		jnz @vbeSetMode3

		popad
		mov eax,4		; return 'Mode not supported'
		ret

	@vbeSetMode3:
		mov TmpVar,ax		; save mode number
		call vbeGetModeInfo_
		or eax,eax
		jne @vbeSetMode1 	; if not supp'ed, try next mode

	@vbeSetMode2:
		; check this is the mode we wanted
		mov ax,[ebx].xres
		cmp _ModeInfoBlock.XResolution,ax
		jne @vbeSetMode1
		mov ax,[ebx].yres
		cmp _ModeInfoBlock.YResolution,ax
		jne @vbeSetMode1
		cmp _ModeInfoBlock.BitsPerPixel,cl
		jne @vbeSetMode1
		mov ebp,ebx		; preserve address

		; this is the mode, try setting with LFB
		mov ax,4F02h		; function 02h - set video mode
		mov bx,TmpVar
		or bx,4000h
		int 10h

		or ah,ah
		jz @vbeSetMode4

		popad
		mov eax,4
		ret

	@vbeSetMode4:

		; map LFB to accessible mem
		mov ebx,_ModeInfoBlock.PhysBasePtr
		mov ecx,ebx
		shr ebx,16		; bx:cx - physical address
		movzx esi,_VbeInfoBlock.TotalMemory
		shl esi,6
		shl esi,10		; TM * 64 * 1024 = Total video mem
		mov edi,esi
		shr esi,16		; si:di - region size

		mov ax,800h
		int 31h			; call DPMI

		jnc @vbeSetMode5
		popad
		mov eax,2		; return memory error
		ret

	@vbeSetMode5:

		shl ebx,16
		mov bx,cx
		mov [ebp].adr,ebx	; save new LFB address

		mov ax,[ebp].yres
		xor edx,edx
		mov dx,_ModeInfoBlock.BytesPerScanLine
		mul dx
		shl edx,16
		mov dx,ax
		mov [ebp].size,edx

    call vbeGetScanWidth_
    or ax,ax
    je @vbeSetMode6
    mov bx,ax
		movzx eax,[ebp].xres
    cmp ax,bx
    je @vbeSetMode6

		call vbeSetScanWidth_

  @vbeSetMode6:

		popad
		xor eax,eax		; return success
		ret

	vbeSetMode_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetVirtual_	PROC
		pushad

		mov [ebx].xres,ax
		mov [ebx].yres,dx

		xor edx,edx
		mov dl,_ModeInfoBlock.BitsPerPixel
                inc dl			; just in case its 15bpp
		shr dl,3		; dl - bytes per pixel
		mul dx
		ror eax,16
		mov ax,dx
		ror eax,16		; ax - bytes per line

		mov dx,[ebx].yres
		mul dx
		shl edx,16
		mov dx,ax      		; (xres * bytespp) * yres
		mov [ebx].size,edx

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
		ret                                             
		                                                
	@vbeSV1:                                                
		shl ebx,16                                      
		mov bx,cx		; ebx - screen address  
		mov eax,ebx                                     
		pop ebx                                          
		mov [ebx].adr,eax                               
                                                                
		shl esi,16                                      
		mov si,di 		; esi - memory handle   
		mov [ebx].handle,esi                            
		                                                
		popad                                           
		xor eax,eax                                     
		ret                                             
	vbeSetVirtual_	ENDP                                    
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´                        
                                                                
                                                                
                                                                
;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeFreeVirtual_	PROC
		pushad
		
		mov ebx,eax
		mov esi,[ebx].handle
		mov di,si
		shr esi,16		; si:di - block handle
		
		mov ax,502h                                      
		int 31h			; free linear memory
		jc @vbeFV_error
		
		mov [ebx].xres,0
		mov [ebx].yres,0
		mov [ebx].size,0                                
		mov [ebx].adr,0
		
		popad
		xor eax,eax                                     
		ret
		
	@vbeFV_error:
		popad
		mov eax,5		; return 'Memory allocation error'
		ret      
	vbeFreeVirtual_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´

                                                                
                                                                
;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ                        
	vbeFlip_	PROC                                    
		pushad                                          
		                                                
		mov ebx,eax                                     
		mov esi,[ebx].adr                               
		mov ebx,edx                                     
		mov edi,[ebx].adr                               
		                                                
		mov ecx,[ebx].size                              
		shr ecx,2                                       
		                                                
		rep movsd                                       
		popad                                           
		ret                                             
	vbeFlip_	ENDP                                    
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´                        
                                                                
                                                                
                                                                
;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ                        
	vbePutPixel_	PROC                                    
		pushad                                          
		                                                
		push eax                                        
		mov ax,_ModeInfoBlock.BytesPerScanLine          
		mul dx                                          
		shl edx,16                                      
		mov dx,ax		; edx - y * bpsl        
		pop eax                                         
		                                                
		push edx                                        
		movzx edx,_ModeInfoBlock.BitsPerPixel           
		inc dl	 		; just in case its 15bpp
		shr dl,3		; find out bytes per pixel
		mul dx                                          
		shl edx,16                                      
		mov dx,ax                                       
		pop eax                                         
		add eax,edx 		; eax - pixel offset    
		push eax                                        
		                                                
		cmp _ModeInfoBlock.MemoryModel,4	; 256 colors modes
		jne @vbePP1                                     
		                                                
		pop eax                                         
		mov edi,[ebx].adr                               
		add edi,eax                                     
		mov [edi],cl                                    
		jmp @vbePP_end                                  
		                                                
	@vbePP1:                                                
		cmp _ModeInfoBlock.MemoryModel,6	; Direct Color modes
		jne @vbePP_end                                  
		                                                
		mov ebp,ebx                                     
		xor ebx,ebx                                     
		push ecx                                        
		                                                
		mov eax,-1 			; build blue mask
		mov edx,eax                                       
		mov cl,_ModeInfoBlock.BlueMaskSize              
		shl edx,cl                                        
		xor eax,edx	      		; eax - blue mask
		and edi,eax                                     
		mov cl,_ModeInfoBlock.BlueFieldPosition         
		shl edi,cl                                      
		or ebx,edi                                      
		                                                
		mov eax,-1                                      
		mov edx,eax                                     
		mov cl,_ModeInfoBlock.GreenMaskSize             
		shl edx,cl                                      
		xor eax,edx			; eax - green mask
		and esi,eax                                     
		mov cl,_ModeInfoBlock.GreenFieldPosition        
		shl esi,cl                                      
		or ebx,esi                                      
		                                                
		mov eax,-1                                      
		mov edx,eax                                     
		mov cl,_ModeInfoBlock.RedMaskSize               
		shl edx,cl                                      
		xor eax,edx			; eax - red mask
		pop edx                                         
		and edx,eax                                     
		mov cl,_ModeInfoBlock.RedFieldPosition          
		shl edx,cl                                      
		or ebx,edx 			; ebx - composed pixel color
		                                                
		mov eax,ebx                                     
		mov ebx,[ebp].adr                               
		pop edx                                         
		add ebx,edx                                     
		                                                
		cmp _ModeInfoBlock.BitsPerPixel,16              
		jnbe @vbePP2                                    
		                                                
		mov [ebx],ax                                    
		jmp @vbePP_end                                  
		                                                
	@vbePP2:                                                
		cmp _ModeInfoBlock.BitsPerPixel,24              
		jne @vbePP3                                     
		                                                
		mov [ebx],ax                                    
		shr eax,16                                      
		mov [ebx+2],al                                  
		jmp @vbePP_end                                  
		                                                
	@vbePP3:                                                
		cmp _ModeInfoBlock.BitsPerPixel,32              
		jne @vbePP_end                                  
		                                                
		mov [ebx],eax                                   
		                                                
	@vbePP_end:                                             
		popad                                           
		ret                                             
	vbePutPixel_	ENDP                                    
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´                        
                                                                
                                                                
                                                                
;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ                        
	vbeClearScreen_	PROC                                    
		pushad                                          
		                                                
		mov ebx,eax                                     
		mov edi,[ebx].adr                               
		                                                
		mov ecx,[ebx].size                              
		shr ecx,2                                       
		xor eax,eax                                     
		                                                
		rep stosd                                       
		                                                
		popad                                           
		ret
	vbeClearScreen_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetScanWidth_	PROC
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
		ret                                             
		                                                
	@vbeSSW1:                                               
		popad                                           
		xor eax,eax                                     
		ret                                             
	vbeSetScanWidth_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetScanWidth_	PROC
		pushad

		xor ecx,ecx
		xor edx,edx
		mov ebx,1
		mov ax,4f06h
		int 10h

		mov TmpVar,cx

		cmp al,4fh
		je @vbeGSW1

		popad
		xor eax,eax			; failed
		ret

	@vbeGSW1:
		popad
		mov ax,TmpVar
		ret
	vbeGetScanWidth_     	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetStart_	PROC
		pushad
		
		xor ebx,ebx
		mov ecx,eax		; ecx - x pos.  edx - y pos.
		mov ax,4f07h
		int 10h
		
		cmp al,4fh
		je @vbeSS1
		
		popad
		mov eax,1
		ret
		
	@vbeSS1:
		popad
		xor eax,eax
		ret
	vbeSetStart_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetStart_	PROC
		pushad
		
		push eax edx		; save x & y adresses
		xor ecx,ecx
		xor edx,edx
		mov ebx,1
		mov ax,4f07h
		int 10h
		
		cmp al,4fh
		je @vbeGS1
		
		popad
		mov eax,1		; failed
		ret
		
	@vbeGS1:
		pop ebx
		mov [ebx],edx
		pop ebx
		mov [ebx],ecx		; save values
		
		popad
		xor eax,eax
		ret
	vbeGetStart_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeSetVGAMode_	PROC
		pushad
		
		xor ah,ah
		int 10h
				
		popad
		ret
	vbeSetVGAMode_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeGetVGAMode_	PROC
		pushad
		
		mov ah,0fh
		int 10h
		mov TmpVar,ax
		
		popad
		mov ax,TmpVar
		xor ah,ah
		ret
	vbeGetVGAMode_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´



;ÃÍÍÍÍÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	vbeWR_	PROC
		push eax edx
		
		mov dx,03dah
	start:             
		in al,dx   
		test al,8  
		jnz start  
	end:               
		in al,dx   
		test al,8  
		jz end
		
		pop edx eax
		ret
	vbeWR_	ENDP
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÍÍÍÍ´

		END
