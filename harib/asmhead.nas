; haribote-os boot asm
; TAB=4

[INSTRSET "i486p"]				; 486��ɱ��� ����ϰ� �ʹٰ� �ϴ� ���

VBEMODE	EQU		0x105			; 1024 x  768 x 8 bit Į��
; (ȭ�� ��� �϶�)
;	0x100 :  640 x  400 x 8 bit Į��
;	0x101 :  640 x  480 x 8 bit Į��
;	0x103 :  800 x  600 x 8 bit Į��
;	0x105 : 1024 x  768 x 8 bit Į��
;	0x107 : 1280 x 1024 x 8 bit Į��

BOTPAK	EQU		0x00280000		; bootpack�� �ε� ���
DSKCAC	EQU		0x00100000		; ��ũ ĳ�� ���α׷��� ���
DSKCAC0	EQU		0x00008000		; ��ũ ĳ�� ���α׷��� ���(������)

; BOOT_INFO ����
CYLS	EQU		0x0ff0			; boot sector�� �����Ѵ�
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �� �������� ���� ����.� ��Ʈ Į���ΰ�?
SCRNX	EQU		0x0ff4			; �ػ� X
SCRNY	EQU		0x0ff6			; �ػ� Y
VRAM	EQU		0x0ff8			; �׷��� ������ ���� ����

		ORG		0xc200		; �� ���α׷��� ��� Read�Ǵ°�
		
; VBE ���� Ȯ��
	
		MOV		AX, 0x9000
		MOV		ES, AX
		MOV		DI, 0
		MOV		AX, 0x4f00
		INT		0x10
		CMP		AX, 0x004f
		JNE		scrn320
		
; VBE�� ���� üũ
	
		MOV		AX, [ES:DI+4]
		CMP		AX, 0x0200
		JB		scrn320
				
; ȭ�� ��� ������ ��´�.

		MOV		CX, VBEMODE
		MOV		AX, 0x4f01
		INT		0x10
		CMP		AX, 0x004f
		JNE		scrn320
		
; ȭ�� ��� ������ Ȯ��

		CMP		BYTE [ES:DI+0x19], 8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b], 4
		JNE		scrn320
		MOV		AX, [ES:DI+0x00]
		AND		AX, 0x0080
		JZ		scrn320		; ��� �Ӽ��� bit7�� 0�̾����Ƿ� �����Ѵ�.

; ȭ�� ����� ��ȯ

	MOV		BX, VBEMODE+0x4000
	MOV		AX, 0x4f02
	INT 	0x10
	MOV		BYTE [VMODE], 8		;ȭ�� ��带 write(c�� �����Ѵ�)
	MOV		AX, [ES:DI+0x12]
	MOV		[SCRNX], AX
	MOV		AX, [ES:DI+0x14]
	MOV		[SCRNY], AX
	MOV		EAX, [ES:DI+0x28]
	MOV		[VRAM], EAX
	JMP		keystatus
		
scrn320:
		MOV		AL, 0x13	; VGA �׷��Ƚ�, 320x200x8bit�÷�
		MOV		AH, 0x00
		INT		0x10
		MOV		BYTE[VMODE], 8	; ȭ�� ��� ���� (c�� �����Ѵ�)
		MOV		WORD[SCRNX], 320
		MOV		WORD[SCRNY], 200
		MOV		DWORD[VRAM], 0x000a0000
		
; Ű������ LED ���¸� BIOS�� �˷��ش�

keystatus:
		MOV		AH,0x02
		INT		0x16 		; keyboard BIOS
		MOV		[LEDS],AL

; PIC�� ������ ���ͷ�Ʈ�� �޾Ƶ����� �ʰ� �Ѵ�
;	ATȣȯ���� ��翡���� PIC�� �ʱ�ȭ�� �Ѵٸ�,
;	�̰͵��� CLI�տ� �� ���� ������ �̵��� ��� �Ѵ�
;	PIC�� �ʱ�ȭ�� ���߿� �Ѵ�

		MOV		AL,0xff
		OUT		0x21,AL
		NOP				; OUT����� �����ϸ� �� ���� �ʴ� ������ �ִ� �� ���� ������
		OUT		0xa1,AL

		CLI				; CPU���������� ���ͷ�Ʈ ����

; CPU�κ��� 1MB�̻��� �޸𸮿� �׼��� �� �� �ֵ���, A20GATE�� ����

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL, 0xdf	; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; ������Ʈ ��� ����

		LGDT	[GDTR0]			; ���� GDT�� ����
		MOV		EAX,CR0
		AND		EAX, 0x7fffffff	; bit31�� 0���� �Ѵ�(����¡ ������ ����)
		OR		EAX, 0x00000001	; bit0�� 1�� �Ѵ�(������Ʈ ��� �����̹Ƿ�)
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8		; read, write ���� ���׸�Ʈ(segment) 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack�� ����

		MOV		ESI, bootpack	; ���ۿ�
		MOV		EDI, BOTPAK	; ����ó
		MOV		ECX,512*1024/4
		CALL	memcpy

; �ϴ� �迡 ��ũ �����͵� ������ ��ġ�� ����

; �켱�� boot sector�κ���

		MOV		ESI, 0x7c00	; ���ۿ�
		MOV		EDI, DSKCAC	; ����ó
		MOV		ECX,512/4
		CALL	memcpy

; ������ ����

		MOV		ESI, DSKCAC0+512; ���ۿ�
		MOV		EDI, DSKCAC+512	; ����ó
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4		; �Ǹ������κ��� ����Ʈ��/4�� ��ȯ
		SUB		ECX,512/4	; IPL�и�ŭ �����Ѵ�
		CALL	memcpy

; asmhead�� �ؾ� �ϴ� ���� ���� �� �����Ƿ�,
;	�������� bootpack�� �ñ��

; bootpack�� �⵿

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX, 3		; ECX += 3;
		SHR		ECX, 2		; ECX /= 4;
		JZ		skip		; �����ؾ� �� ���� ����
		MOV		ESI,[EBX+20]	; ���ۿ�
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; ����ó
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; ���� �ʱ�ġ
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL, 0x60 	; �� ������ Read(���� ���۰� �������� ���ϰ�)
		JNZ		waitkbdout	; AND����� 0�� �ƴϸ� waitkbdout��
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy		; ���� �� ����� 0�� �ƴϸ� memcpy��
		RET
; memcpy�� �ּ� ������ prefix�� ���� ���� ���� ������, string ��ɿ����� �� �� �ִ�

		ALIGNB	16
GDT0:
		RESB	8			; null selector
		DW		0xffff, 0x0000, 0x9200, 0x00cf	; read/write ���� ���׸�Ʈ(segment) 32bit
		DW		0xffff, 0x0000, 0x9a28, 0x0047	; ���� ���� ���׸�Ʈ(segment) 32 bit(bootpack��)

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
