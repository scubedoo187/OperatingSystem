/* 메모리 관계 */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 386인지, 486 이후인지를 확인 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;	/* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) {
		/* 386 이상에서는 AC=1로 해도 자동으로 0이 되어 버린다 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;	/* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;	/* 캐시 금지 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;	/* 캐시 허가 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* 빈 정보 개수 */
	man->maxfrees = 0;	/* 상황 관촬용 : frees의 최대 값 */
	man->lostsize = 0;	/* 해제에 실패한 합계 사이즈 */
	man->losts = 0;		/* 해제에 실패한 횟수 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 빈 사이즈의 합계를 보고 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* 확보 */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 충분한 넓이의 빈 영역 발견 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i] 가 없어졌으므로 앞으로 채운다 */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];	/* 구조체의 대입 */
				}
			}
			return a;
		}
	}
	return 0;		/* 빈 영역이 없다. */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 해제 */
{
	int i, j;
	/* 정리하기 쉽게 하려면, free[]를 addr 순서로 나열하는 편이 좋다. */
	/* 그러니까 우선 어디에 넣을 것인지를 결정한다. */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 앞이 비어있지 않다. */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 앞의 빈 영역을 정리한다. */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 뒤에도 비어있지 않다. */
				if (addr + size == man->free[i].addr) {
					/* 뒤의 빈 영역도 정리한다. */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i] 의 삭제 */
					/* free[i] 가 없어졌으므로 앞으로 채운다. */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];	/* 구조체의 대입 */
					}
				}
			}
			return 0;	/* 성공 종료 */
		}
	}
	/* 앞의 빈 영역이 정리되지 않았다. */
	if (i < man->frees) {
		/* 뒤의 빈 영역이 없다. */
		if (addr + size == man->free[i].addr) {
			/* 뒤의 빈 영역을 정리한다. */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;	/* 성공 종료 */
		}
	}
	/* 앞에도 뒤에도 빈 영역이 없다. */
	if (man->frees < MEMMAN_FREES) {
		/* free[i] 보다 뒤의 빈 영역을 뒤로 옮겨 놓고, 간격을 만든다. */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;		/* 최대 값을 갱신 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;	/* 성공 종료 */
	}
	/* 뒤로 옮겨 놓을 수 없었다. */
	man->losts++;
	man->lostsize += size;
	return -1;	/* 실패 종료 */
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