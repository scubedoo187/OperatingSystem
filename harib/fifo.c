/* FIFO ���̺귯�� */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
/* FIFO ������ �ʱ�ȭ */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* �� �� */
	fifo->flags = 0;
	fifo->p = 0; /* write ��ġ */
	fifo->q = 0; /* read ��ġ */
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data)
/* FIFO�� �����͸� ���� �״´� */
{
	if (fifo->free == 0) {
		/* �� ���� ��� ���ƴ� */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

int fifo8_get(struct FIFO8 *fifo)
/* FIFO�κ��� �����͸� 1�� �����´� */
{
	int data;
	if (fifo->free == fifo->size) {
		/* ���۰� ������� ���� �켱 -1�� �־����� */
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo)
/* ��� ���� �����Ͱ� �� ����� �����Ѵ� */
{
	return fifo->size - fifo->free;
}
