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
