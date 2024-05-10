/*
 * Authors: Benstone Zhang <benstonezhang@gmail.com>
 *
 * GPLv2
 */

#include <stdlib.h>
#include <dlfcn.h>

#include "tools-util.h"

typedef void *(*libc_call_malloc)(size_t size);
typedef void *(*libc_call_calloc)(size_t nmemb, size_t size);
typedef void *(*libc_call_realloc)(void *ptr, size_t size);
typedef void *(*libc_call_reallocarray)(void *ptr, size_t nmemb, size_t size);
typedef int (*libc_call_posix_memalign)(void **memptr, size_t alignment, size_t size);
typedef void *(*libc_call_aligned_alloc)(size_t alignment, size_t size);

static libc_call_malloc real_malloc;
static libc_call_calloc real_calloc;
static libc_call_realloc real_realloc;
static libc_call_reallocarray real_reallocarray;
static libc_call_posix_memalign real_posix_memalign;
static libc_call_aligned_alloc real_aligned_alloc;

__attribute__((constructor(101)))
void _bcachefs_tools_init(void)
{
	real_malloc = (libc_call_malloc)dlsym(RTLD_NEXT, "malloc");
	real_calloc = (libc_call_calloc)dlsym(RTLD_NEXT, "calloc");
	real_realloc = (libc_call_realloc)dlsym(RTLD_NEXT, "realloc");
	real_reallocarray = (libc_call_reallocarray)dlsym(RTLD_NEXT, "reallocarray");
	real_posix_memalign = (libc_call_posix_memalign)dlsym(RTLD_NEXT, "posix_memalign");
	real_aligned_alloc = (libc_call_aligned_alloc)dlsym(RTLD_NEXT, "aligned_alloc");
}

void *malloc(size_t size)
{
	void *p = real_malloc(size);
	if (!p)
		die("insufficient memory");
	return p;
}

void *calloc(size_t nmemb, size_t size)
{
	void *p = real_calloc(nmemb, size);
	if (!p)
		die("insufficient memory");
	return p;
}

void *realloc(void *ptr, size_t size)
{
	void *p = real_realloc(ptr, size);
	if (!p)
		die("insufficient memory");
	return p;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	void *p = real_reallocarray(ptr, nmemb, size);
	if (!p)
		die("insufficient memory");
	return p;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	int ret = real_posix_memalign(memptr, alignment, size);
	if (!*memptr)
		die("insufficient memory");
	return ret;
}

void *aligned_alloc(size_t alignment, size_t size)
{
	void *p = real_aligned_alloc(alignment, size);
	if (!p)
		die("insufficient memory");
	return p;
}
