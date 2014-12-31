/* �޸� ���� */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 386����, 486 ���������� Ȯ�� */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;	/* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) {
		/* 386 �̻󿡼��� AC=1�� �ص� �ڵ����� 0�� �Ǿ� ������ */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;	/* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;	/* ĳ�� ���� */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;	/* ĳ�� �㰡 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* �� ���� ���� */
	man->maxfrees = 0;	/* ��Ȳ ���Կ� : frees�� �ִ� �� */
	man->lostsize = 0;	/* ������ ������ �հ� ������ */
	man->losts = 0;		/* ������ ������ Ƚ�� */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* �� �������� �հ踦 ���� */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* Ȯ�� */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* ����� ������ �� ���� �߰� */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i] �� ���������Ƿ� ������ ä��� */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];	/* ����ü�� ���� */
				}
			}
			return a;
		}
	}
	return 0;		/* �� ������ ����. */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* ���� */
{
	int i, j;
	/* �����ϱ� ���� �Ϸ���, free[]�� addr ������ �����ϴ� ���� ����. */
	/* �׷��ϱ� �켱 ��� ���� �������� �����Ѵ�. */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* ���� ������� �ʴ�. */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* ���� �� ������ �����Ѵ�. */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* �ڿ��� ������� �ʴ�. */
				if (addr + size == man->free[i].addr) {
					/* ���� �� ������ �����Ѵ�. */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i] �� ���� */
					/* free[i] �� ���������Ƿ� ������ ä���. */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];	/* ����ü�� ���� */
					}
				}
			}
			return 0;	/* ���� ���� */
		}
	}
	/* ���� �� ������ �������� �ʾҴ�. */
	if (i < man->frees) {
		/* ���� �� ������ ����. */
		if (addr + size == man->free[i].addr) {
			/* ���� �� ������ �����Ѵ�. */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;	/* ���� ���� */
		}
	}
	/* �տ��� �ڿ��� �� ������ ����. */
	if (man->frees < MEMMAN_FREES) {
		/* free[i] ���� ���� �� ������ �ڷ� �Ű� ����, ������ �����. */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;		/* �ִ� ���� ���� */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;	/* ���� ���� */
	}
	/* �ڷ� �Ű� ���� �� ������. */
	man->losts++;
	man->lostsize += size;
	return -1;	/* ���� ���� */
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

unsigned int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}