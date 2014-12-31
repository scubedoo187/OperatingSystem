/* 마우스나 윈도우의 중첩 처리 */

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
	ctl->top = -1;	/* 시트가 한 장도 없다. */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0;		/* 미사용 마크 */
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
			sht->flags = SHEET_USE;	/* 사용중 마크 */
			sht->height = -1;		/* 비 표시 중 */
			return sht;
		}
	}
	return 0;		/* 모든 시트가 사용중이다. */
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
	int h, old = sht->height;	/* 설정 전의 높이를 기억한다. */

	/* 지정한 높이가 너무 낮거나 너무 높으면 수정한다. */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height;	/* 높이를 설정 */

	/* 이하는 주로 sheets[] 의 대체 */
	if (old > height) {		/* 이전보다도 낮아진다. */
		if (height >= 0) {
			/* 사이에 있는 것을 하나씩 끌어 올린다. */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {		/* 비표시화 */
			if (ctl->top > old) {
				/* 위에 있는 것을 하나씩 내린다. */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;		/* 표시 중의 레이어가 1개 줄어들게되므로, 맨위의 높이가 줄어든다. */
		}
		sheet_refresh(ctl);		/* 새로운 레이어의 정보에 따라 화면을 다시 그린다. */
	} else if (old < height) {	/* 이전보다 높아진다. */
		if (old >= 0) {
			/* 사이에 있는 것을 하나씩 내린다. */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* 비표시 상태에서 표시 상태로 */
			/* 위에 있는 것을 하나씩 들어 올린다. */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;		/* 표시 중의 레이어가 1개 증가하므로, 맨 위의 높이가 증가한다. */
		}
		sheet_refresh(ctl);	/* 새로운 레이어의 정보에 따라 화면을 다시 그림. */
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
	if (sht->height >= 0) {		/* 만약 표시 중이라면 */
		sheet_refresh(ctl);		/* 새로운 레이어 정보에 따라 화면을 다시 그린다. */
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1);		/* 표시 중이라면 우선 비표시로 한다. */
	}
	sht->flags = 0;		/* 미사용 마크 */
	return;
}