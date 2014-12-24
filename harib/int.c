/* ���ͷ�Ʈ ���� */

#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PIC�� �ʱ�ȭ */
{
	io_out8(PIC0_IMR,  0xff  ); /* ��� ���ͷ�Ʈ�� �޾Ƶ����� �ʴ´� */
	io_out8(PIC1_IMR,  0xff  ); /* ��� ���ͷ�Ʈ�� �޾Ƶ����� �ʴ´� */

	io_out8(PIC0_ICW1, 0x11  ); /* edge trigger ��� */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7��, INT20-27���� �޴´� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1�� IRQ2���� ���� */
	io_out8(PIC0_ICW4, 0x01  ); /* non buffer��� */

	io_out8(PIC1_ICW1, 0x11  ); /* edge trigger ��� */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15��, INT28-2 f�� �޴´� */
	io_out8(PIC1_ICW3, 2     ); /* PIC1�� IRQ2���� ���� */
	io_out8(PIC1_ICW4, 0x01  ); /* non buffer��� */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1 �ܴ̿� ��� ���� */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ��� ���ͷ�Ʈ�� �޾Ƶ����� �ʴ´� */

	return;
}

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* IRQ-01 ���� �ϷḦ PIC�� ���� */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

void inthandler2c(int *esp)
/* PS/2 ���콺�κ����� ���ͷ�Ʈ */
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* IRQ-12 ���� �ϷḦ PIC1�� ���� */
	io_out8(PIC0_OCW2, 0x62);	/* IRQ-02 ���� �ϷḦ PIC0�� ���� */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}