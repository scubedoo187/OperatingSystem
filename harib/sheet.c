/* ���콺�� �������� ��ø ó�� */

#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;	/* ��Ʈ�� �� �嵵 ����. */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0;		/* �̻�� ��ũ */
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;	/* ����� ��ũ */
			sht->height = -1;		/* �� ǥ�� �� */
			return sht;
		}
	}
	return 0;		/* ��� ��Ʈ�� ������̴�. */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height;	/* ���� ���� ���̸� ����Ѵ�. */

	/* ������ ���̰� �ʹ� ���ų� �ʹ� ������ �����Ѵ�. */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height;	/* ���̸� ���� */

	/* ���ϴ� �ַ� sheets[] �� ��ü */
	if (old > height) {		/* �������ٵ� ��������. */
		if (height >= 0) {
			/* ���̿� �ִ� ���� �ϳ��� ���� �ø���. */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {		/* ��ǥ��ȭ */
			if (ctl->top > old) {
				/* ���� �ִ� ���� �ϳ��� ������. */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;		/* ǥ�� ���� ���̾ 1�� �پ��ԵǹǷ�, ������ ���̰� �پ���. */
		}
		sheet_refresh(ctl);		/* ���ο� ���̾��� ������ ���� ȭ���� �ٽ� �׸���. */
	} else if (old < height) {	/* �������� ��������. */
		if (old >= 0) {
			/* ���̿� �ִ� ���� �ϳ��� ������. */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ��ǥ�� ���¿��� ǥ�� ���·� */
			/* ���� �ִ� ���� �ϳ��� ��� �ø���. */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;		/* ǥ�� ���� ���̾ 1�� �����ϹǷ�, �� ���� ���̰� �����Ѵ�. */
		}
		sheet_refresh(ctl);	/* ���ο� ���̾��� ������ ���� ȭ���� �ٽ� �׸�. */
	}
	return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 = bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) {		/* ���� ǥ�� ���̶�� */
		sheet_refresh(ctl);		/* ���ο� ���̾� ������ ���� ȭ���� �ٽ� �׸���. */
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1);		/* ǥ�� ���̶�� �켱 ��ǥ�÷� �Ѵ�. */
	}
	sht->flags = 0;		/* �̻�� ��ũ */
	return;
}