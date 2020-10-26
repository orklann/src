/*
 * Copyright (c) 2020 Anton Lindqvist <anton@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/param.h>	/* PAGE_SIZE */
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void growshrink(const char *);
static void writer(const char *, int);

static void sighandler(int);
static void __dead usage(void);

static int gotsig;
static int npages = 2;

/*
 * Trigger a deadlock between uvn_flush() and uvn_io() caused by uvn_flush()
 * holding the vnode lock while waiting for pages to become unbusy; the busy
 * pages are allocated and marked as busy by uvn_io() which in turn is trying to
 * lock the same vnode while populating the page(s) using I/O on the vnode.
 */
int
main(int argc, char *argv[])
{
	const char *path;
	pid_t pid;
	int killpip[2];
	int status;
	int iterations = 100;
	char ch;

	while ((ch = getopt(argc, argv, "I")) != -1) {
		switch (ch) {
		case 'I':
			iterations = -1;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		usage();
	path = argv[0];

	if (pipe2(killpip, O_NONBLOCK) == -1)
		err(1, "pipe2");

	if (signal(SIGINT, sighandler) == SIG_ERR)
		err(1, "signal");

	pid = fork();
	if (pid == -1)
		err(1, "fork");
	if (pid == 0) {
		close(killpip[1]);
		writer(path, killpip[0]);
		return 0;
	}
	close(killpip[0]);

	fprintf(stderr, "parent = %d, child = %d\n", getpid(), pid);

	while (gotsig == 0) {
		if (iterations > 0 && --iterations == 0)
			break;

		growshrink(path);
	}

	/* Signal shutdown to writer() process. */
	write(killpip[1], &ch, 1);
	close(killpip[1]);

	if (waitpid(pid, &status, 0) == -1)
		err(1, "waitpid");
	if (WIFSIGNALED(status))
		return 128 + WTERMSIG(status);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return 0;
}

static void
growshrink(const char *path)
{
	char *p;
	int fd;

	/*
	 * Open and truncate the file causing uvn_flush() to try evict pages
	 * which are currently being populated by the writer() process.
	 */
	fd = open(path, O_RDWR | O_TRUNC);
	if (fd == -1)
		err(1, "open: %s", path);

	/* Grow the file again. */
	if (ftruncate(fd, npages * PAGE_SIZE) == -1)
		err(1, "ftruncate");

	p = mmap(NULL, npages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
	    fd, 0);
	if (p == MAP_FAILED)
		err(1, "mmap");

	/* Populate the last page. */
	memset(&p[(npages - 1) * PAGE_SIZE], 'x', PAGE_SIZE);

	if (munmap(p, npages * PAGE_SIZE) == -1)
		err(1, "munmap");
	close(fd);
}

static void
writer(const char *path, int killfd)
{
	char *p;
	int fd;
	char c;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		err(1, "open: %s", path);
	p = mmap(NULL, npages * PAGE_SIZE, PROT_READ, MAP_SHARED,
	    fd, 0);
	if (p == MAP_FAILED)
		err(1, "mmap");

	for (;;) {
		const struct iovec *iov = (const struct iovec *)p;

		if (read(killfd, &c, 1) == 1)
			break;

		/*
		 * This write should never succeed since the file descriptor is
		 * invalid. However, it should cause a page fault during
		 * copyin() which in turn will invoke uvn_io() while trying to
		 * populate the page. At this point, it will try to lock the
		 * vnode, which is potentially already locked by the
		 * growshrink() process.
		 */
		pwritev(-1, iov, 1, 0);
	}
}

static void
sighandler(int signo)
{
	gotsig = signo;
}

static void __dead
usage(void)
{
	fprintf(stderr, "usage: vnode [-I] path\n");
	exit(1);
}
