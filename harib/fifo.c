/* FIFO ���̺귯�� */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFO ������ �ʱ�ȭ */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* �� �� */
	fifo->flags = 0;
	fifo->p = 0; /* write ��ġ */
	fifo->q = 0; /* read ��ġ */
	fifo->task = task;	/* �����Ͱ� ���� ���� ����Ű�� �½�ũ */
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* FIFO�� �����͸� ������ */
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
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) {	/* �½�ũ�� �ڰ� ������ */
			task_run(fifo->task, -1, 0);		/* ���� �ش� */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
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

int fifo32_status(struct FIFO32 *fifo)
/* ��� ���� �����Ͱ� �� ����� �����Ѵ� */
{
	return fifo->size - fifo->free;
}
