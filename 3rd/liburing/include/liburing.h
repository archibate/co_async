/* SPDX-License-Identifier: MIT */
#ifndef LIB_URING_H
#define LIB_URING_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500 /* Required for glibc to expose sigset_t */
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* Required for musl to expose cpu_set_t */
#endif

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <linux/swab.h>
#include <sys/wait.h>
/* SPDX-License-Identifier: MIT */
#ifndef LIBURING_COMPAT_H
#define LIBURING_COMPAT_H

#include <linux/time_types.h>
/* <linux/time_types.h> is included above and not needed again */
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H 1

/* #include <linux/openat2.h> */

#endif
/* SPDX-License-Identifier: (GPL-2.0 WITH Linux-syscall-note) OR MIT */
/*
 * Header file for the io_uring interface.
 *
 * Copyright (C) 2019 Jens Axboe
 * Copyright (C) 2019 Christoph Hellwig
 */
/* SPDX-License-Identifier: (GPL-2.0 WITH Linux-syscall-note) OR MIT */
/*
 * Header file for the io_uring interface.
 *
 * Copyright (C) 2019 Jens Axboe
 * Copyright (C) 2019 Christoph Hellwig
 */
#ifndef LINUX_IO_URING_H
#define LINUX_IO_URING_H

#include <linux/fs.h>
#include <linux/types.h>
/*
 * this file is shared with liburing and that has to autodetect
 * if linux/time_types.h is available or not, it can
 * define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
 * if linux/time_types.h is not available
 */
#ifndef UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
#include <linux/time_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

/*
 * IO submission data structure (Submission Queue Entry)
 */
struct io_uring_sqe {
	__u8	opcode;		/* type of operation for this sqe */
	__u8	flags;		/* IOSQE_ flags */
	__u16	ioprio;		/* ioprio for the request */
	__s32	fd;		/* file descriptor to do IO on */
	union {
		__u64	off;	/* offset into file */
		__u64	addr2;
		struct {
			__u32	cmd_op;
			__u32	__pad1;
		};
	};
	union {
		__u64	addr;	/* pointer to buffer or iovecs */
		__u64	splice_off_in;
		struct {
			__u32	level;
			__u32	optname;
		};
	};
	__u32	len;		/* buffer size or number of iovecs */
	union {
		__kernel_rwf_t	rw_flags;
		__u32		fsync_flags;
		__u16		poll_events;	/* compatibility */
		__u32		poll32_events;	/* word-reversed for BE */
		__u32		sync_range_flags;
		__u32		msg_flags;
		__u32		timeout_flags;
		__u32		accept_flags;
		__u32		cancel_flags;
		__u32		open_flags;
		__u32		statx_flags;
		__u32		fadvise_advice;
		__u32		splice_flags;
		__u32		rename_flags;
		__u32		unlink_flags;
		__u32		hardlink_flags;
		__u32		xattr_flags;
		__u32		msg_ring_flags;
		__u32		uring_cmd_flags;
		__u32		waitid_flags;
		__u32		futex_flags;
		__u32		install_fd_flags;
	};
	__u64	user_data;	/* data to be passed back at completion time */
	/* pack this to avoid bogus arm OABI complaints */
	union {
		/* index into fixed buffers, if used */
		__u16	buf_index;
		/* for grouped buffer selection */
		__u16	buf_group;
	} __attribute__((packed));
	/* personality to use, if used */
	__u16	personality;
	union {
		__s32	splice_fd_in;
		__u32	file_index;
		__u32	optlen;
		struct {
			__u16	addr_len;
			__u16	__pad3[1];
		};
	};
	union {
		struct {
			__u64	addr3;
			__u64	__pad2[1];
		};
		__u64	optval;
		/*
		 * If the ring is initialized with IORING_SETUP_SQE128, then
		 * this field is used for 80 bytes of arbitrary command data
		 */
		__u8	cmd[0];
	};
};

/*
 * If sqe->file_index is set to this for opcodes that instantiate a new
 * direct descriptor (like openat/openat2/accept), then io_uring will allocate
 * an available direct descriptor instead of having the application pass one
 * in. The picked direct descriptor will be returned in cqe->res, or -ENFILE
 * if the space is full.
 */
#define IORING_FILE_INDEX_ALLOC		(~0U)

enum {
	IOSQE_FIXED_FILE_BIT,
	IOSQE_IO_DRAIN_BIT,
	IOSQE_IO_LINK_BIT,
	IOSQE_IO_HARDLINK_BIT,
	IOSQE_ASYNC_BIT,
	IOSQE_BUFFER_SELECT_BIT,
	IOSQE_CQE_SKIP_SUCCESS_BIT,
};

/*
 * sqe->flags
 */
/* use fixed fileset */
#define IOSQE_FIXED_FILE	(1U << IOSQE_FIXED_FILE_BIT)
/* issue after inflight IO */
#define IOSQE_IO_DRAIN		(1U << IOSQE_IO_DRAIN_BIT)
/* links next sqe */
#define IOSQE_IO_LINK		(1U << IOSQE_IO_LINK_BIT)
/* like LINK, but stronger */
#define IOSQE_IO_HARDLINK	(1U << IOSQE_IO_HARDLINK_BIT)
/* always go async */
#define IOSQE_ASYNC		(1U << IOSQE_ASYNC_BIT)
/* select buffer from sqe->buf_group */
#define IOSQE_BUFFER_SELECT	(1U << IOSQE_BUFFER_SELECT_BIT)
/* don't post CQE if request succeeded */
#define IOSQE_CQE_SKIP_SUCCESS	(1U << IOSQE_CQE_SKIP_SUCCESS_BIT)

/*
 * io_uring_setup() flags
 */
#define IORING_SETUP_IOPOLL	(1U << 0)	/* io_context is polled */
#define IORING_SETUP_SQPOLL	(1U << 1)	/* SQ poll thread */
#define IORING_SETUP_SQ_AFF	(1U << 2)	/* sq_thread_cpu is valid */
#define IORING_SETUP_CQSIZE	(1U << 3)	/* app defines CQ size */
#define IORING_SETUP_CLAMP	(1U << 4)	/* clamp SQ/CQ ring sizes */
#define IORING_SETUP_ATTACH_WQ	(1U << 5)	/* attach to existing wq */
#define IORING_SETUP_R_DISABLED	(1U << 6)	/* start with ring disabled */
#define IORING_SETUP_SUBMIT_ALL	(1U << 7)	/* continue submit on error */
/*
 * Cooperative task running. When requests complete, they often require
 * forcing the submitter to transition to the kernel to complete. If this
 * flag is set, work will be done when the task transitions anyway, rather
 * than force an inter-processor interrupt reschedule. This avoids interrupting
 * a task running in userspace, and saves an IPI.
 */
#define IORING_SETUP_COOP_TASKRUN	(1U << 8)
/*
 * If COOP_TASKRUN is set, get notified if task work is available for
 * running and a kernel transition would be needed to run it. This sets
 * IORING_SQ_TASKRUN in the sq ring flags. Not valid with COOP_TASKRUN.
 */
#define IORING_SETUP_TASKRUN_FLAG	(1U << 9)
#define IORING_SETUP_SQE128		(1U << 10) /* SQEs are 128 byte */
#define IORING_SETUP_CQE32		(1U << 11) /* CQEs are 32 byte */
/*
 * Only one task is allowed to submit requests
 */
#define IORING_SETUP_SINGLE_ISSUER	(1U << 12)

/*
 * Defer running task work to get events.
 * Rather than running bits of task work whenever the task transitions
 * try to do it just before it is needed.
 */
#define IORING_SETUP_DEFER_TASKRUN	(1U << 13)

/*
 * Application provides ring memory
 */
#define IORING_SETUP_NO_MMAP		(1U << 14)

/*
 * Register the ring fd in itself for use with
 * IORING_REGISTER_USE_REGISTERED_RING; return a registered fd index rather
 * than an fd.
 */
#define IORING_SETUP_REGISTERED_FD_ONLY	(1U << 15)

/*
 * Removes indirection through the SQ index array.
 */
#define IORING_SETUP_NO_SQARRAY		(1U << 16)

enum io_uring_op {
	IORING_OP_NOP,
	IORING_OP_READV,
	IORING_OP_WRITEV,
	IORING_OP_FSYNC,
	IORING_OP_READ_FIXED,
	IORING_OP_WRITE_FIXED,
	IORING_OP_POLL_ADD,
	IORING_OP_POLL_REMOVE,
	IORING_OP_SYNC_FILE_RANGE,
	IORING_OP_SENDMSG,
	IORING_OP_RECVMSG,
	IORING_OP_TIMEOUT,
	IORING_OP_TIMEOUT_REMOVE,
	IORING_OP_ACCEPT,
	IORING_OP_ASYNC_CANCEL,
	IORING_OP_LINK_TIMEOUT,
	IORING_OP_CONNECT,
	IORING_OP_FALLOCATE,
	IORING_OP_OPENAT,
	IORING_OP_CLOSE,
	IORING_OP_FILES_UPDATE,
	IORING_OP_STATX,
	IORING_OP_READ,
	IORING_OP_WRITE,
	IORING_OP_FADVISE,
	IORING_OP_MADVISE,
	IORING_OP_SEND,
	IORING_OP_RECV,
	IORING_OP_OPENAT2,
	IORING_OP_EPOLL_CTL,
	IORING_OP_SPLICE,
	IORING_OP_PROVIDE_BUFFERS,
	IORING_OP_REMOVE_BUFFERS,
	IORING_OP_TEE,
	IORING_OP_SHUTDOWN,
	IORING_OP_RENAMEAT,
	IORING_OP_UNLINKAT,
	IORING_OP_MKDIRAT,
	IORING_OP_SYMLINKAT,
	IORING_OP_LINKAT,
	IORING_OP_MSG_RING,
	IORING_OP_FSETXATTR,
	IORING_OP_SETXATTR,
	IORING_OP_FGETXATTR,
	IORING_OP_GETXATTR,
	IORING_OP_SOCKET,
	IORING_OP_URING_CMD,
	IORING_OP_SEND_ZC,
	IORING_OP_SENDMSG_ZC,
	IORING_OP_READ_MULTISHOT,
	IORING_OP_WAITID,
	IORING_OP_FUTEX_WAIT,
	IORING_OP_FUTEX_WAKE,
	IORING_OP_FUTEX_WAITV,
	IORING_OP_FIXED_FD_INSTALL,
	IORING_OP_FTRUNCATE,

	/* this goes last, obviously */
	IORING_OP_LAST,
};

/*
 * sqe->uring_cmd_flags
 * IORING_URING_CMD_FIXED	use registered buffer; pass this flag
 *				along with setting sqe->buf_index.
 */
#define IORING_URING_CMD_FIXED	(1U << 0)


/*
 * sqe->fsync_flags
 */
#define IORING_FSYNC_DATASYNC	(1U << 0)

/*
 * sqe->timeout_flags
 */
#define IORING_TIMEOUT_ABS		(1U << 0)
#define IORING_TIMEOUT_UPDATE		(1U << 1)
#define IORING_TIMEOUT_BOOTTIME		(1U << 2)
#define IORING_TIMEOUT_REALTIME		(1U << 3)
#define IORING_LINK_TIMEOUT_UPDATE	(1U << 4)
#define IORING_TIMEOUT_ETIME_SUCCESS	(1U << 5)
#define IORING_TIMEOUT_MULTISHOT	(1U << 6)
#define IORING_TIMEOUT_CLOCK_MASK	(IORING_TIMEOUT_BOOTTIME | IORING_TIMEOUT_REALTIME)
#define IORING_TIMEOUT_UPDATE_MASK	(IORING_TIMEOUT_UPDATE | IORING_LINK_TIMEOUT_UPDATE)
/*
 * sqe->splice_flags
 * extends splice(2) flags
 */
#define SPLICE_F_FD_IN_FIXED	(1U << 31) /* the last bit of __u32 */

/*
 * POLL_ADD flags. Note that since sqe->poll_events is the flag space, the
 * command flags for POLL_ADD are stored in sqe->len.
 *
 * IORING_POLL_ADD_MULTI	Multishot poll. Sets IORING_CQE_F_MORE if
 *				the poll handler will continue to report
 *				CQEs on behalf of the same SQE.
 *
 * IORING_POLL_UPDATE		Update existing poll request, matching
 *				sqe->addr as the old user_data field.
 *
 * IORING_POLL_LEVEL		Level triggered poll.
 */
#define IORING_POLL_ADD_MULTI	(1U << 0)
#define IORING_POLL_UPDATE_EVENTS	(1U << 1)
#define IORING_POLL_UPDATE_USER_DATA	(1U << 2)
#define IORING_POLL_ADD_LEVEL		(1U << 3)

/*
 * ASYNC_CANCEL flags.
 *
 * IORING_ASYNC_CANCEL_ALL	Cancel all requests that match the given key
 * IORING_ASYNC_CANCEL_FD	Key off 'fd' for cancelation rather than the
 *				request 'user_data'
 * IORING_ASYNC_CANCEL_ANY	Match any request
 * IORING_ASYNC_CANCEL_FD_FIXED	'fd' passed in is a fixed descriptor
 */
#define IORING_ASYNC_CANCEL_ALL	(1U << 0)
#define IORING_ASYNC_CANCEL_FD	(1U << 1)
#define IORING_ASYNC_CANCEL_ANY	(1U << 2)
#define IORING_ASYNC_CANCEL_FD_FIXED	(1U << 3)

/*
 * send/sendmsg and recv/recvmsg flags (sqe->ioprio)
 *
 * IORING_RECVSEND_POLL_FIRST	If set, instead of first attempting to send
 *				or receive and arm poll if that yields an
 *				-EAGAIN result, arm poll upfront and skip
 *				the initial transfer attempt.
 *
 * IORING_RECV_MULTISHOT	Multishot recv. Sets IORING_CQE_F_MORE if
 *				the handler will continue to report
 *				CQEs on behalf of the same SQE.
 *
 * IORING_RECVSEND_FIXED_BUF	Use registered buffers, the index is stored in
 *				the buf_index field.
 *
 * IORING_SEND_ZC_REPORT_USAGE
 *				If set, SEND[MSG]_ZC should report
 *				the zerocopy usage in cqe.res
 *				for the IORING_CQE_F_NOTIF cqe.
 *				0 is reported if zerocopy was actually possible.
 *				IORING_NOTIF_USAGE_ZC_COPIED if data was copied
 *				(at least partially).
 */
#define IORING_RECVSEND_POLL_FIRST	(1U << 0)
#define IORING_RECV_MULTISHOT		(1U << 1)
#define IORING_RECVSEND_FIXED_BUF	(1U << 2)
#define IORING_SEND_ZC_REPORT_USAGE	(1U << 3)

/*
 * cqe.res for IORING_CQE_F_NOTIF if
 * IORING_SEND_ZC_REPORT_USAGE was requested
 *
 * It should be treated as a flag, all other
 * bits of cqe.res should be treated as reserved!
 */
#define IORING_NOTIF_USAGE_ZC_COPIED    (1U << 31)

/*
 * accept flags stored in sqe->ioprio
 */
#define IORING_ACCEPT_MULTISHOT	(1U << 0)

/*
 * IORING_OP_MSG_RING command types, stored in sqe->addr
 */
enum {
	IORING_MSG_DATA,	/* pass sqe->len as 'res' and off as user_data */
	IORING_MSG_SEND_FD,	/* send a registered fd to another ring */
};

/*
 * IORING_OP_MSG_RING flags (sqe->msg_ring_flags)
 *
 * IORING_MSG_RING_CQE_SKIP	Don't post a CQE to the target ring. Not
 *				applicable for IORING_MSG_DATA, obviously.
 */
#define IORING_MSG_RING_CQE_SKIP	(1U << 0)
/* Pass through the flags from sqe->file_index to cqe->flags */
#define IORING_MSG_RING_FLAGS_PASS	(1U << 1)

/*
 * IORING_OP_FIXED_FD_INSTALL flags (sqe->install_fd_flags)
 *
 * IORING_FIXED_FD_NO_CLOEXEC	Don't mark the fd as O_CLOEXEC
 */
#define IORING_FIXED_FD_NO_CLOEXEC	(1U << 0)

/*
 * IO completion data structure (Completion Queue Entry)
 */
struct io_uring_cqe {
	__u64	user_data;	/* sqe->data submission passed back */
	__s32	res;		/* result code for this event */
	__u32	flags;

	/*
	 * If the ring is initialized with IORING_SETUP_CQE32, then this field
	 * contains 16-bytes of padding, doubling the size of the CQE.
	 */
	__u64 big_cqe[];
};

/*
 * cqe->flags
 *
 * IORING_CQE_F_BUFFER	If set, the upper 16 bits are the buffer ID
 * IORING_CQE_F_MORE	If set, parent SQE will generate more CQE entries
 * IORING_CQE_F_SOCK_NONEMPTY	If set, more data to read after socket recv
 * IORING_CQE_F_NOTIF	Set for notification CQEs. Can be used to distinct
 * 			them from sends.
 */
#define IORING_CQE_F_BUFFER		(1U << 0)
#define IORING_CQE_F_MORE		(1U << 1)
#define IORING_CQE_F_SOCK_NONEMPTY	(1U << 2)
#define IORING_CQE_F_NOTIF		(1U << 3)

enum {
	IORING_CQE_BUFFER_SHIFT		= 16,
};

/*
 * Magic offsets for the application to mmap the data it needs
 */
#define IORING_OFF_SQ_RING		0ULL
#define IORING_OFF_CQ_RING		0x8000000ULL
#define IORING_OFF_SQES			0x10000000ULL
#define IORING_OFF_PBUF_RING		0x80000000ULL
#define IORING_OFF_PBUF_SHIFT		16
#define IORING_OFF_MMAP_MASK		0xf8000000ULL

/*
 * Filled with the offset for mmap(2)
 */
struct io_sqring_offsets {
	__u32 head;
	__u32 tail;
	__u32 ring_mask;
	__u32 ring_entries;
	__u32 flags;
	__u32 dropped;
	__u32 array;
	__u32 resv1;
	__u64 user_addr;
};

/*
 * sq_ring->flags
 */
#define IORING_SQ_NEED_WAKEUP	(1U << 0) /* needs io_uring_enter wakeup */
#define IORING_SQ_CQ_OVERFLOW	(1U << 1) /* CQ ring is overflown */
#define IORING_SQ_TASKRUN	(1U << 2) /* task should enter the kernel */

struct io_cqring_offsets {
	__u32 head;
	__u32 tail;
	__u32 ring_mask;
	__u32 ring_entries;
	__u32 overflow;
	__u32 cqes;
	__u32 flags;
	__u32 resv1;
	__u64 user_addr;
};

/*
 * cq_ring->flags
 */

/* disable eventfd notifications */
#define IORING_CQ_EVENTFD_DISABLED	(1U << 0)

/*
 * io_uring_enter(2) flags
 */
#define IORING_ENTER_GETEVENTS		(1U << 0)
#define IORING_ENTER_SQ_WAKEUP		(1U << 1)
#define IORING_ENTER_SQ_WAIT		(1U << 2)
#define IORING_ENTER_EXT_ARG		(1U << 3)
#define IORING_ENTER_REGISTERED_RING	(1U << 4)

/*
 * Passed in for io_uring_setup(2). Copied back with updated info on success
 */
struct io_uring_params {
	__u32 sq_entries;
	__u32 cq_entries;
	__u32 flags;
	__u32 sq_thread_cpu;
	__u32 sq_thread_idle;
	__u32 features;
	__u32 wq_fd;
	__u32 resv[3];
	struct io_sqring_offsets sq_off;
	struct io_cqring_offsets cq_off;
};

/*
 * io_uring_params->features flags
 */
#define IORING_FEAT_SINGLE_MMAP		(1U << 0)
#define IORING_FEAT_NODROP		(1U << 1)
#define IORING_FEAT_SUBMIT_STABLE	(1U << 2)
#define IORING_FEAT_RW_CUR_POS		(1U << 3)
#define IORING_FEAT_CUR_PERSONALITY	(1U << 4)
#define IORING_FEAT_FAST_POLL		(1U << 5)
#define IORING_FEAT_POLL_32BITS 	(1U << 6)
#define IORING_FEAT_SQPOLL_NONFIXED	(1U << 7)
#define IORING_FEAT_EXT_ARG		(1U << 8)
#define IORING_FEAT_NATIVE_WORKERS	(1U << 9)
#define IORING_FEAT_RSRC_TAGS		(1U << 10)
#define IORING_FEAT_CQE_SKIP		(1U << 11)
#define IORING_FEAT_LINKED_FILE		(1U << 12)
#define IORING_FEAT_REG_REG_RING	(1U << 13)

/*
 * io_uring_register(2) opcodes and arguments
 */
enum {
	IORING_REGISTER_BUFFERS			= 0,
	IORING_UNREGISTER_BUFFERS		= 1,
	IORING_REGISTER_FILES			= 2,
	IORING_UNREGISTER_FILES			= 3,
	IORING_REGISTER_EVENTFD			= 4,
	IORING_UNREGISTER_EVENTFD		= 5,
	IORING_REGISTER_FILES_UPDATE		= 6,
	IORING_REGISTER_EVENTFD_ASYNC		= 7,
	IORING_REGISTER_PROBE			= 8,
	IORING_REGISTER_PERSONALITY		= 9,
	IORING_UNREGISTER_PERSONALITY		= 10,
	IORING_REGISTER_RESTRICTIONS		= 11,
	IORING_REGISTER_ENABLE_RINGS		= 12,

	/* extended with tagging */
	IORING_REGISTER_FILES2			= 13,
	IORING_REGISTER_FILES_UPDATE2		= 14,
	IORING_REGISTER_BUFFERS2		= 15,
	IORING_REGISTER_BUFFERS_UPDATE		= 16,

	/* set/clear io-wq thread affinities */
	IORING_REGISTER_IOWQ_AFF		= 17,
	IORING_UNREGISTER_IOWQ_AFF		= 18,

	/* set/get max number of io-wq workers */
	IORING_REGISTER_IOWQ_MAX_WORKERS	= 19,

	/* register/unregister io_uring fd with the ring */
	IORING_REGISTER_RING_FDS		= 20,
	IORING_UNREGISTER_RING_FDS		= 21,

	/* register ring based provide buffer group */
	IORING_REGISTER_PBUF_RING		= 22,
	IORING_UNREGISTER_PBUF_RING		= 23,

	/* sync cancelation API */
	IORING_REGISTER_SYNC_CANCEL		= 24,

	/* register a range of fixed file slots for automatic slot allocation */
	IORING_REGISTER_FILE_ALLOC_RANGE	= 25,

	/* return status information for a buffer group */
	IORING_REGISTER_PBUF_STATUS		= 26,

	/* set/clear busy poll settings */
	IORING_REGISTER_NAPI			= 27,
	IORING_UNREGISTER_NAPI			= 28,

	/* this goes last */
	IORING_REGISTER_LAST,

	/* flag added to the opcode to use a registered ring fd */
	IORING_REGISTER_USE_REGISTERED_RING	= 1U << 31
};

/* io-wq worker categories */
enum {
	IO_WQ_BOUND,
	IO_WQ_UNBOUND,
};

/* deprecated, see struct io_uring_rsrc_update */
struct io_uring_files_update {
	__u32 offset;
	__u32 resv;
	__aligned_u64 /* __s32 * */ fds;
};

/*
 * Register a fully sparse file space, rather than pass in an array of all
 * -1 file descriptors.
 */
#define IORING_RSRC_REGISTER_SPARSE	(1U << 0)

struct io_uring_rsrc_register {
	__u32 nr;
	__u32 flags;
	__u64 resv2;
	__aligned_u64 data;
	__aligned_u64 tags;
};

struct io_uring_rsrc_update {
	__u32 offset;
	__u32 resv;
	__aligned_u64 data;
};

struct io_uring_rsrc_update2 {
	__u32 offset;
	__u32 resv;
	__aligned_u64 data;
	__aligned_u64 tags;
	__u32 nr;
	__u32 resv2;
};

/* Skip updating fd indexes set to this value in the fd table */
#define IORING_REGISTER_FILES_SKIP	(-2)

#define IO_URING_OP_SUPPORTED	(1U << 0)

struct io_uring_probe_op {
	__u8 op;
	__u8 resv;
	__u16 flags;	/* IO_URING_OP_* flags */
	__u32 resv2;
};

struct io_uring_probe {
	__u8 last_op;	/* last opcode supported */
	__u8 ops_len;	/* length of ops[] array below */
	__u16 resv;
	__u32 resv2[3];
	struct io_uring_probe_op ops[];
};

struct io_uring_restriction {
	__u16 opcode;
	union {
		__u8 register_op; /* IORING_RESTRICTION_REGISTER_OP */
		__u8 sqe_op;      /* IORING_RESTRICTION_SQE_OP */
		__u8 sqe_flags;   /* IORING_RESTRICTION_SQE_FLAGS_* */
	};
	__u8 resv;
	__u32 resv2[3];
};

struct io_uring_buf {
	__u64	addr;
	__u32	len;
	__u16	bid;
	__u16	resv;
};

struct io_uring_buf_ring {
	union {
		/*
		 * To avoid spilling into more pages than we need to, the
		 * ring tail is overlaid with the io_uring_buf->resv field.
		 */
		struct {
			__u64	resv1;
			__u32	resv2;
			__u16	resv3;
			__u16	tail;
		};
		struct io_uring_buf	bufs[0];
	};
};

/*
 * Flags for IORING_REGISTER_PBUF_RING.
 *
 * IOU_PBUF_RING_MMAP:	If set, kernel will allocate the memory for the ring.
 *			The application must not set a ring_addr in struct
 *			io_uring_buf_reg, instead it must subsequently call
 *			mmap(2) with the offset set as:
 *			IORING_OFF_PBUF_RING | (bgid << IORING_OFF_PBUF_SHIFT)
 *			to get a virtual mapping for the ring.
 */
enum {
	IOU_PBUF_RING_MMAP	= 1,
};

/* argument for IORING_(UN)REGISTER_PBUF_RING */
struct io_uring_buf_reg {
	__u64	ring_addr;
	__u32	ring_entries;
	__u16	bgid;
	__u16	flags;
	__u64	resv[3];
};

/* argument for IORING_REGISTER_PBUF_STATUS */
struct io_uring_buf_status {
	__u32	buf_group;	/* input */
	__u32	head;		/* output */
	__u32	resv[8];
};

/* argument for IORING_(UN)REGISTER_NAPI */
struct io_uring_napi {
	__u32   busy_poll_to;
	__u8    prefer_busy_poll;
	__u8    pad[3];
	__u64   resv;
};

/*
 * io_uring_restriction->opcode values
 */
enum {
	/* Allow an io_uring_register(2) opcode */
	IORING_RESTRICTION_REGISTER_OP		= 0,

	/* Allow an sqe opcode */
	IORING_RESTRICTION_SQE_OP		= 1,

	/* Allow sqe flags */
	IORING_RESTRICTION_SQE_FLAGS_ALLOWED	= 2,

	/* Require sqe flags (these flags must be set on each submission) */
	IORING_RESTRICTION_SQE_FLAGS_REQUIRED	= 3,

	IORING_RESTRICTION_LAST
};

struct io_uring_getevents_arg {
	__u64	sigmask;
	__u32	sigmask_sz;
	__u32	pad;
	__u64	ts;
};

/*
 * Argument for IORING_REGISTER_SYNC_CANCEL
 */
struct io_uring_sync_cancel_reg {
	__u64				addr;
	__s32				fd;
	__u32				flags;
	struct __kernel_timespec	timeout;
	__u64				pad[4];
};

/*
 * Argument for IORING_REGISTER_FILE_ALLOC_RANGE
 * The range is specified as [off, off + len)
 */
struct io_uring_file_index_range {
	__u32	off;
	__u32	len;
	__u64	resv;
};

struct io_uring_recvmsg_out {
	__u32 namelen;
	__u32 controllen;
	__u32 payloadlen;
	__u32 flags;
};

/*
 * Argument for IORING_OP_URING_CMD when file is a socket
 */
enum {
	SOCKET_URING_OP_SIOCINQ		= 0,
	SOCKET_URING_OP_SIOCOUTQ,
	SOCKET_URING_OP_GETSOCKOPT,
	SOCKET_URING_OP_SETSOCKOPT,
};

#ifdef __cplusplus
}
#endif

#endif
/* SPDX-License-Identifier: MIT */
#ifndef LIBURING_VERSION_H
#define LIBURING_VERSION_H

#define IO_URING_VERSION_MAJOR 2
#define IO_URING_VERSION_MINOR 5

#endif
/* SPDX-License-Identifier: MIT */
#ifndef LIBURING_BARRIER_H
#define LIBURING_BARRIER_H

/*
From the kernel documentation file refcount-vs-atomic.rst:

A RELEASE memory ordering guarantees that all prior loads and
stores (all po-earlier instructions) on the same CPU are completed
before the operation. It also guarantees that all po-earlier
stores on the same CPU and all propagated stores from other CPUs
must propagate to all other CPUs before the release operation
(A-cumulative property). This is implemented using
:c:func:`smp_store_release`.

An ACQUIRE memory ordering guarantees that all post loads and
stores (all po-later instructions) on the same CPU are
completed after the acquire operation. It also guarantees that all
po-later stores on the same CPU must propagate to all other CPUs
after the acquire operation executes. This is implemented using
:c:func:`smp_acquire__after_ctrl_dep`.
*/

#ifdef __cplusplus
#include <atomic>

template <typename T>
static inline void IO_URING_WRITE_ONCE(T &var, T val)
{
	std::atomic_store_explicit(reinterpret_cast<std::atomic<T> *>(&var),
				   val, std::memory_order_relaxed);
}
template <typename T>
static inline T IO_URING_READ_ONCE(const T &var)
{
	return std::atomic_load_explicit(
		reinterpret_cast<const std::atomic<T> *>(&var),
		std::memory_order_relaxed);
}

template <typename T>
static inline void io_uring_smp_store_release(T *p, T v)
{
	std::atomic_store_explicit(reinterpret_cast<std::atomic<T> *>(p), v,
				   std::memory_order_release);
}

template <typename T>
static inline T io_uring_smp_load_acquire(const T *p)
{
	return std::atomic_load_explicit(
		reinterpret_cast<const std::atomic<T> *>(p),
		std::memory_order_acquire);
}

static inline void io_uring_smp_mb()
{
	std::atomic_thread_fence(std::memory_order_seq_cst);
}
#else
#include <stdatomic.h>

#define IO_URING_WRITE_ONCE(var, val)				\
	atomic_store_explicit((_Atomic __typeof__(var) *)&(var),	\
			      (val), memory_order_relaxed)
#define IO_URING_READ_ONCE(var)					\
	atomic_load_explicit((_Atomic __typeof__(var) *)&(var),	\
			     memory_order_relaxed)

#define io_uring_smp_store_release(p, v)			\
	atomic_store_explicit((_Atomic __typeof__(*(p)) *)(p), (v), \
			      memory_order_release)
#define io_uring_smp_load_acquire(p)				\
	atomic_load_explicit((_Atomic __typeof__(*(p)) *)(p),	\
			     memory_order_acquire)

#define io_uring_smp_mb()					\
	atomic_thread_fence(memory_order_seq_cst)
#endif

#endif /* defined(LIBURING_BARRIER_H) */

#ifndef uring_unlikely
#define uring_unlikely(cond)	__builtin_expect(!!(cond), 0)
#endif

#ifndef uring_likely
#define uring_likely(cond)	__builtin_expect(!!(cond), 1)
#endif

#ifndef IOURINGINLINE
#define IOURINGINLINE static inline
#endif
#ifndef IOURINGEXTERN
#define IOURINGEXTERN
#endif

#ifdef __alpha__
/*
 * alpha and mips are the exceptions, all other architectures have
 * common numbers for new system calls.
 */
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup		535
#endif
#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter		536
#endif
#ifndef __NR_io_uring_register
#define __NR_io_uring_register		537
#endif
#elif defined __mips__
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup		(__NR_Linux + 425)
#endif
#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter		(__NR_Linux + 426)
#endif
#ifndef __NR_io_uring_register
#define __NR_io_uring_register		(__NR_Linux + 427)
#endif
#else /* !__alpha__ and !__mips__ */
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup		425
#endif
#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter		426
#endif
#ifndef __NR_io_uring_register
#define __NR_io_uring_register		427
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Library interface to io_uring
 */
struct io_uring_sq {
	unsigned *khead;
	unsigned *ktail;
	// Deprecated: use `ring_mask` instead of `*kring_mask`
	unsigned *kring_mask;
	// Deprecated: use `ring_entries` instead of `*kring_entries`
	unsigned *kring_entries;
	unsigned *kflags;
	unsigned *kdropped;
	unsigned *array;
	struct io_uring_sqe *sqes;

	unsigned sqe_head;
	unsigned sqe_tail;

	size_t ring_sz;
	unsigned char *ring_ptr;

	unsigned ring_mask;
	unsigned ring_entries;

	unsigned pad[2];
};

struct io_uring_cq {
	unsigned *khead;
	unsigned *ktail;
	// Deprecated: use `ring_mask` instead of `*kring_mask`
	unsigned *kring_mask;
	// Deprecated: use `ring_entries` instead of `*kring_entries`
	unsigned *kring_entries;
	unsigned *kflags;
	unsigned *koverflow;
	struct io_uring_cqe *cqes;

	size_t ring_sz;
	unsigned char *ring_ptr;

	unsigned ring_mask;
	unsigned ring_entries;

	unsigned pad[2];
};

struct io_uring {
	struct io_uring_sq sq;
	struct io_uring_cq cq;
	unsigned flags;
	int ring_fd;

	unsigned features;
	int enter_ring_fd;
	__u8 int_flags;
	__u8 pad[3];
	unsigned pad2;
};

/*
 * Library interface
 */

/*
 * return an allocated io_uring_probe structure, or NULL if probe fails (for
 * example, if it is not available). The caller is responsible for freeing it
 */
struct io_uring_probe *io_uring_get_probe_ring(struct io_uring *ring);
/* same as io_uring_get_probe_ring, but takes care of ring init and teardown */
struct io_uring_probe *io_uring_get_probe(void);

/*
 * frees a probe allocated through io_uring_get_probe() or
 * io_uring_get_probe_ring()
 */
void io_uring_free_probe(struct io_uring_probe *probe);

IOURINGINLINE int io_uring_opcode_supported(const struct io_uring_probe *p,
					    int op)
{
	if (op > p->last_op)
		return 0;
	return (p->ops[op].flags & IO_URING_OP_SUPPORTED) != 0;
}

IOURINGEXTERN int io_uring_queue_init_mem(unsigned entries, struct io_uring *ring,
				struct io_uring_params *p,
				void *buf, size_t buf_size);
IOURINGEXTERN int io_uring_queue_init_params(unsigned entries, struct io_uring *ring,
				struct io_uring_params *p);
IOURINGEXTERN int io_uring_queue_init(unsigned entries, struct io_uring *ring,
			unsigned flags);
IOURINGEXTERN int io_uring_queue_mmap(int fd, struct io_uring_params *p,
			struct io_uring *ring);
IOURINGEXTERN int io_uring_ring_dontfork(struct io_uring *ring);
IOURINGEXTERN void io_uring_queue_exit(struct io_uring *ring);
IOURINGEXTERN unsigned io_uring_peek_batch_cqe(struct io_uring *ring,
	struct io_uring_cqe **cqes, unsigned count);
IOURINGEXTERN int io_uring_wait_cqes(struct io_uring *ring, struct io_uring_cqe **cqe_ptr,
		       unsigned wait_nr, struct __kernel_timespec *ts,
		       sigset_t *sigmask);
IOURINGEXTERN int io_uring_wait_cqe_timeout(struct io_uring *ring,
			      struct io_uring_cqe **cqe_ptr,
			      struct __kernel_timespec *ts);
IOURINGEXTERN int io_uring_submit(struct io_uring *ring);
IOURINGEXTERN int io_uring_submit_and_wait(struct io_uring *ring, unsigned wait_nr);
IOURINGEXTERN int io_uring_submit_and_wait_timeout(struct io_uring *ring,
				     struct io_uring_cqe **cqe_ptr,
				     unsigned wait_nr,
				     struct __kernel_timespec *ts,
				     sigset_t *sigmask);

IOURINGEXTERN int io_uring_register_buffers(struct io_uring *ring, const struct iovec *iovecs,
			      unsigned nr_iovecs);
IOURINGEXTERN int io_uring_register_buffers_tags(struct io_uring *ring,
				   const struct iovec *iovecs,
				   const __u64 *tags, unsigned nr);
IOURINGEXTERN int io_uring_register_buffers_sparse(struct io_uring *ring, unsigned nr);
IOURINGEXTERN int io_uring_register_buffers_update_tag(struct io_uring *ring,
					 unsigned off,
					 const struct iovec *iovecs,
					 const __u64 *tags, unsigned nr);
IOURINGEXTERN int io_uring_unregister_buffers(struct io_uring *ring);

IOURINGEXTERN int io_uring_register_files(struct io_uring *ring, const int *files,
			    unsigned nr_files);
IOURINGEXTERN int io_uring_register_files_tags(struct io_uring *ring, const int *files,
				 const __u64 *tags, unsigned nr);
IOURINGEXTERN int io_uring_register_files_sparse(struct io_uring *ring, unsigned nr);
IOURINGEXTERN int io_uring_register_files_update_tag(struct io_uring *ring, unsigned off,
				       const int *files, const __u64 *tags,
				       unsigned nr_files);

IOURINGEXTERN int io_uring_unregister_files(struct io_uring *ring);
IOURINGEXTERN int io_uring_register_files_update(struct io_uring *ring, unsigned off,
				   const int *files, unsigned nr_files);
IOURINGEXTERN int io_uring_register_eventfd(struct io_uring *ring, int fd);
IOURINGEXTERN int io_uring_register_eventfd_async(struct io_uring *ring, int fd);
IOURINGEXTERN int io_uring_unregister_eventfd(struct io_uring *ring);
IOURINGEXTERN int io_uring_register_probe(struct io_uring *ring, struct io_uring_probe *p,
			    unsigned nr);
IOURINGEXTERN int io_uring_register_personality(struct io_uring *ring);
IOURINGEXTERN int io_uring_unregister_personality(struct io_uring *ring, int id);
IOURINGEXTERN int io_uring_register_restrictions(struct io_uring *ring,
				   struct io_uring_restriction *res,
				   unsigned int nr_res);
IOURINGEXTERN int io_uring_enable_rings(struct io_uring *ring);
IOURINGEXTERN int __io_uring_sqring_wait(struct io_uring *ring);
IOURINGEXTERN int io_uring_register_iowq_aff(struct io_uring *ring, size_t cpusz,
				const cpu_set_t *mask);
IOURINGEXTERN int io_uring_unregister_iowq_aff(struct io_uring *ring);
IOURINGEXTERN int io_uring_register_iowq_max_workers(struct io_uring *ring,
				       unsigned int *values);
IOURINGEXTERN int io_uring_register_ring_fd(struct io_uring *ring);
IOURINGEXTERN int io_uring_unregister_ring_fd(struct io_uring *ring);
IOURINGEXTERN int io_uring_close_ring_fd(struct io_uring *ring);
IOURINGEXTERN int io_uring_register_buf_ring(struct io_uring *ring,
			       struct io_uring_buf_reg *reg, unsigned int flags);
IOURINGEXTERN int io_uring_unregister_buf_ring(struct io_uring *ring, int bgid);
IOURINGEXTERN int io_uring_register_sync_cancel(struct io_uring *ring,
				 struct io_uring_sync_cancel_reg *reg);

IOURINGEXTERN int io_uring_register_file_alloc_range(struct io_uring *ring,
					unsigned off, unsigned len);

IOURINGEXTERN int io_uring_get_events(struct io_uring *ring);
IOURINGEXTERN int io_uring_submit_and_get_events(struct io_uring *ring);

/*
 * io_uring syscalls.
 */
IOURINGEXTERN int io_uring_enter(unsigned int fd, unsigned int to_submit,
		   unsigned int min_complete, unsigned int flags, sigset_t *sig);
IOURINGEXTERN int io_uring_enter2(unsigned int fd, unsigned int to_submit,
		    unsigned int min_complete, unsigned int flags,
		    sigset_t *sig, size_t sz);
IOURINGEXTERN int io_uring_setup(unsigned int entries, struct io_uring_params *p);
IOURINGEXTERN int io_uring_register(unsigned int fd, unsigned int opcode, const void *arg,
		      unsigned int nr_args);

/*
 * Mapped buffer ring alloc/register + unregister/free helpers
 */
IOURINGEXTERN struct io_uring_buf_ring *io_uring_setup_buf_ring(struct io_uring *ring,
						  unsigned int nentries,
						  int bgid, unsigned int flags,
						  int *ret);
IOURINGEXTERN int io_uring_free_buf_ring(struct io_uring *ring, struct io_uring_buf_ring *br,
			   unsigned int nentries, int bgid);

/*
 * Helper for the peek/wait single cqe functions. Exported because of that,
 * but probably shouldn't be used directly in an application.
 */
IOURINGEXTERN int __io_uring_get_cqe(struct io_uring *ring,
			struct io_uring_cqe **cqe_ptr, unsigned submit,
			unsigned wait_nr, sigset_t *sigmask);

#define LIBURING_UDATA_TIMEOUT	((__u64) -1)

/*
 * Calculates the step size for CQE iteration.
 * 	For standard CQE's its 1, for big CQE's its two.
 */
#define io_uring_cqe_shift(ring)					\
	(!!((ring)->flags & IORING_SETUP_CQE32))

#define io_uring_cqe_index(ring,ptr,mask)				\
	(((ptr) & (mask)) << io_uring_cqe_shift(ring))

#define io_uring_for_each_cqe(ring, head, cqe)				\
	/*								\
	 * io_uring_smp_load_acquire() enforces the order of tail	\
	 * and CQE reads.						\
	 */								\
	for (head = *(ring)->cq.khead;					\
	     (cqe = (head != io_uring_smp_load_acquire((ring)->cq.ktail) ? \
		&(ring)->cq.cqes[io_uring_cqe_index(ring, head, (ring)->cq.ring_mask)] : NULL)); \
	     head++)							\

/*
 * Must be called after io_uring_for_each_cqe()
 */
IOURINGINLINE void io_uring_cq_advance(struct io_uring *ring, unsigned nr)
{
	if (nr) {
		struct io_uring_cq *cq = &ring->cq;

		/*
		 * Ensure that the kernel only sees the new value of the head
		 * index after the CQEs have been read.
		 */
		io_uring_smp_store_release(cq->khead, *cq->khead + nr);
	}
}

/*
 * Must be called after io_uring_{peek,wait}_cqe() after the cqe has
 * been processed by the application.
 */
IOURINGINLINE void io_uring_cqe_seen(struct io_uring *ring,
				     struct io_uring_cqe *cqe)
{
	if (cqe)
		io_uring_cq_advance(ring, 1);
}

/*
 * Command prep helpers
 */

/*
 * Associate pointer @data with the sqe, for later retrieval from the cqe
 * at command completion time with io_uring_cqe_get_data().
 */
IOURINGINLINE void io_uring_sqe_set_data(struct io_uring_sqe *sqe, void *data)
{
	sqe->user_data = (unsigned long) data;
}

IOURINGINLINE void *io_uring_cqe_get_data(const struct io_uring_cqe *cqe)
{
	return (void *) (uintptr_t) cqe->user_data;
}

/*
 * Assign a 64-bit value to this sqe, which can get retrieved at completion
 * time with io_uring_cqe_get_data64. Just like the non-64 variants, except
 * these store a 64-bit type rather than a data pointer.
 */
IOURINGINLINE void io_uring_sqe_set_data64(struct io_uring_sqe *sqe,
					   __u64 data)
{
	sqe->user_data = data;
}

IOURINGINLINE __u64 io_uring_cqe_get_data64(const struct io_uring_cqe *cqe)
{
	return cqe->user_data;
}

/*
 * Tell the app the have the 64-bit variants of the get/set userdata
 */
#define LIBURING_HAVE_DATA64

IOURINGINLINE void io_uring_sqe_set_flags(struct io_uring_sqe *sqe,
					  unsigned flags)
{
	sqe->flags = (__u8) flags;
}

IOURINGINLINE void __io_uring_set_target_fixed_file(struct io_uring_sqe *sqe,
						    unsigned int file_index)
{
	/* 0 means no fixed files, indexes should be encoded as "index + 1" */
	sqe->file_index = file_index + 1;
}

IOURINGINLINE void io_uring_prep_rw(int op, struct io_uring_sqe *sqe, int fd,
				    const void *addr, unsigned len,
				    __u64 offset)
{
	sqe->opcode = (__u8) op;
	sqe->flags = 0;
	sqe->ioprio = 0;
	sqe->fd = fd;
	sqe->off = offset;
	sqe->addr = (unsigned long) addr;
	sqe->len = len;
	sqe->rw_flags = 0;
	sqe->buf_index = 0;
	sqe->personality = 0;
	sqe->file_index = 0;
	sqe->addr3 = 0;
	sqe->__pad2[0] = 0;
}

/*
 * io_uring_prep_splice() - Either @fd_in or @fd_out must be a pipe.
 *
 * - If @fd_in refers to a pipe, @off_in is ignored and must be set to -1.
 *
 * - If @fd_in does not refer to a pipe and @off_in is -1, then @nbytes are read
 *   from @fd_in starting from the file offset, which is incremented by the
 *   number of bytes read.
 *
 * - If @fd_in does not refer to a pipe and @off_in is not -1, then the starting
 *   offset of @fd_in will be @off_in.
 *
 * This splice operation can be used to implement sendfile by splicing to an
 * intermediate pipe first, then splice to the final destination.
 * In fact, the implementation of sendfile in kernel uses splice internally.
 *
 * NOTE that even if fd_in or fd_out refers to a pipe, the splice operation
 * can still fail with EINVAL if one of the fd doesn't explicitly support splice
 * operation, e.g. reading from terminal is unsupported from kernel 5.7 to 5.11.
 * Check issue #291 for more information.
 */
IOURINGINLINE void io_uring_prep_splice(struct io_uring_sqe *sqe,
					int fd_in, int64_t off_in,
					int fd_out, int64_t off_out,
					unsigned int nbytes,
					unsigned int splice_flags)
{
	io_uring_prep_rw(IORING_OP_SPLICE, sqe, fd_out, NULL, nbytes,
				(__u64) off_out);
	sqe->splice_off_in = (__u64) off_in;
	sqe->splice_fd_in = fd_in;
	sqe->splice_flags = splice_flags;
}

IOURINGINLINE void io_uring_prep_tee(struct io_uring_sqe *sqe,
				     int fd_in, int fd_out,
				     unsigned int nbytes,
				     unsigned int splice_flags)
{
	io_uring_prep_rw(IORING_OP_TEE, sqe, fd_out, NULL, nbytes, 0);
	sqe->splice_off_in = 0;
	sqe->splice_fd_in = fd_in;
	sqe->splice_flags = splice_flags;
}

IOURINGINLINE void io_uring_prep_readv(struct io_uring_sqe *sqe, int fd,
				       const struct iovec *iovecs,
				       unsigned nr_vecs, __u64 offset)
{
	io_uring_prep_rw(IORING_OP_READV, sqe, fd, iovecs, nr_vecs, offset);
}

IOURINGINLINE void io_uring_prep_readv2(struct io_uring_sqe *sqe, int fd,
				       const struct iovec *iovecs,
				       unsigned nr_vecs, __u64 offset,
				       int flags)
{
	io_uring_prep_readv(sqe, fd, iovecs, nr_vecs, offset);
	sqe->rw_flags = flags;
}

IOURINGINLINE void io_uring_prep_read_fixed(struct io_uring_sqe *sqe, int fd,
					    void *buf, unsigned nbytes,
					    __u64 offset, int buf_index)
{
	io_uring_prep_rw(IORING_OP_READ_FIXED, sqe, fd, buf, nbytes, offset);
	sqe->buf_index = (__u16) buf_index;
}

IOURINGINLINE void io_uring_prep_writev(struct io_uring_sqe *sqe, int fd,
					const struct iovec *iovecs,
					unsigned nr_vecs, __u64 offset)
{
	io_uring_prep_rw(IORING_OP_WRITEV, sqe, fd, iovecs, nr_vecs, offset);
}

IOURINGINLINE void io_uring_prep_writev2(struct io_uring_sqe *sqe, int fd,
				       const struct iovec *iovecs,
				       unsigned nr_vecs, __u64 offset,
				       int flags)
{
	io_uring_prep_writev(sqe, fd, iovecs, nr_vecs, offset);
	sqe->rw_flags = flags;
}

IOURINGINLINE void io_uring_prep_write_fixed(struct io_uring_sqe *sqe, int fd,
					     const void *buf, unsigned nbytes,
					     __u64 offset, int buf_index)
{
	io_uring_prep_rw(IORING_OP_WRITE_FIXED, sqe, fd, buf, nbytes, offset);
	sqe->buf_index = (__u16) buf_index;
}

IOURINGINLINE void io_uring_prep_recvmsg(struct io_uring_sqe *sqe, int fd,
					 struct msghdr *msg, unsigned flags)
{
	io_uring_prep_rw(IORING_OP_RECVMSG, sqe, fd, msg, 1, 0);
	sqe->msg_flags = flags;
}

IOURINGINLINE void io_uring_prep_recvmsg_multishot(struct io_uring_sqe *sqe,
						   int fd, struct msghdr *msg,
						   unsigned flags)
{
	io_uring_prep_recvmsg(sqe, fd, msg, flags);
	sqe->ioprio |= IORING_RECV_MULTISHOT;
}

IOURINGINLINE void io_uring_prep_sendmsg(struct io_uring_sqe *sqe, int fd,
					 const struct msghdr *msg,
					 unsigned flags)
{
	io_uring_prep_rw(IORING_OP_SENDMSG, sqe, fd, msg, 1, 0);
	sqe->msg_flags = flags;
}

IOURINGINLINE unsigned __io_uring_prep_poll_mask(unsigned poll_mask)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	poll_mask = __swahw32(poll_mask);
#endif
	return poll_mask;
}

IOURINGINLINE void io_uring_prep_poll_add(struct io_uring_sqe *sqe, int fd,
					  unsigned poll_mask)
{
	io_uring_prep_rw(IORING_OP_POLL_ADD, sqe, fd, NULL, 0, 0);
	sqe->poll32_events = __io_uring_prep_poll_mask(poll_mask);
}

IOURINGINLINE void io_uring_prep_poll_multishot(struct io_uring_sqe *sqe,
						int fd, unsigned poll_mask)
{
	io_uring_prep_poll_add(sqe, fd, poll_mask);
	sqe->len = IORING_POLL_ADD_MULTI;
}

IOURINGINLINE void io_uring_prep_poll_remove(struct io_uring_sqe *sqe,
					     __u64 user_data)
{
	io_uring_prep_rw(IORING_OP_POLL_REMOVE, sqe, -1, NULL, 0, 0);
	sqe->addr = user_data;
}

IOURINGINLINE void io_uring_prep_poll_update(struct io_uring_sqe *sqe,
					     __u64 old_user_data,
					     __u64 new_user_data,
					     unsigned poll_mask, unsigned flags)
{
	io_uring_prep_rw(IORING_OP_POLL_REMOVE, sqe, -1, NULL, flags,
			 new_user_data);
	sqe->addr = old_user_data;
	sqe->poll32_events = __io_uring_prep_poll_mask(poll_mask);
}

IOURINGINLINE void io_uring_prep_fsync(struct io_uring_sqe *sqe, int fd,
				       unsigned fsync_flags)
{
	io_uring_prep_rw(IORING_OP_FSYNC, sqe, fd, NULL, 0, 0);
	sqe->fsync_flags = fsync_flags;
}

IOURINGINLINE void io_uring_prep_nop(struct io_uring_sqe *sqe)
{
	io_uring_prep_rw(IORING_OP_NOP, sqe, -1, NULL, 0, 0);
}

IOURINGINLINE void io_uring_prep_timeout(struct io_uring_sqe *sqe,
					 struct __kernel_timespec *ts,
					 unsigned count, unsigned flags)
{
	io_uring_prep_rw(IORING_OP_TIMEOUT, sqe, -1, ts, 1, count);
	sqe->timeout_flags = flags;
}

IOURINGINLINE void io_uring_prep_timeout_remove(struct io_uring_sqe *sqe,
						__u64 user_data, unsigned flags)
{
	io_uring_prep_rw(IORING_OP_TIMEOUT_REMOVE, sqe, -1, NULL, 0, 0);
	sqe->addr = user_data;
	sqe->timeout_flags = flags;
}

IOURINGINLINE void io_uring_prep_timeout_update(struct io_uring_sqe *sqe,
						struct __kernel_timespec *ts,
						__u64 user_data, unsigned flags)
{
	io_uring_prep_rw(IORING_OP_TIMEOUT_REMOVE, sqe, -1, NULL, 0,
				(uintptr_t) ts);
	sqe->addr = user_data;
	sqe->timeout_flags = flags | IORING_TIMEOUT_UPDATE;
}

IOURINGINLINE void io_uring_prep_accept(struct io_uring_sqe *sqe, int fd,
					struct sockaddr *addr,
					socklen_t *addrlen, int flags)
{
	io_uring_prep_rw(IORING_OP_ACCEPT, sqe, fd, addr, 0,
				(__u64) (unsigned long) addrlen);
	sqe->accept_flags = (__u32) flags;
}

/* accept directly into the fixed file table */
IOURINGINLINE void io_uring_prep_accept_direct(struct io_uring_sqe *sqe, int fd,
					       struct sockaddr *addr,
					       socklen_t *addrlen, int flags,
					       unsigned int file_index)
{
	io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
	/* offset by 1 for allocation */
	if (file_index == IORING_FILE_INDEX_ALLOC)
		file_index--;
	__io_uring_set_target_fixed_file(sqe, file_index);
}

IOURINGINLINE void io_uring_prep_multishot_accept(struct io_uring_sqe *sqe,
						  int fd, struct sockaddr *addr,
						  socklen_t *addrlen, int flags)
{
	io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
	sqe->ioprio |= IORING_ACCEPT_MULTISHOT;
}

/* multishot accept directly into the fixed file table */
IOURINGINLINE void io_uring_prep_multishot_accept_direct(struct io_uring_sqe *sqe,
							 int fd,
							 struct sockaddr *addr,
							 socklen_t *addrlen,
							 int flags)
{
	io_uring_prep_multishot_accept(sqe, fd, addr, addrlen, flags);
	__io_uring_set_target_fixed_file(sqe, IORING_FILE_INDEX_ALLOC - 1);
}

IOURINGINLINE void io_uring_prep_cancel64(struct io_uring_sqe *sqe,
					  __u64 user_data, int flags)
{
	io_uring_prep_rw(IORING_OP_ASYNC_CANCEL, sqe, -1, NULL, 0, 0);
	sqe->addr = user_data;
	sqe->cancel_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_cancel(struct io_uring_sqe *sqe,
					void *user_data, int flags)
{
	io_uring_prep_cancel64(sqe, (__u64) (uintptr_t) user_data, flags);
}

IOURINGINLINE void io_uring_prep_cancel_fd(struct io_uring_sqe *sqe, int fd,
					   unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_ASYNC_CANCEL, sqe, fd, NULL, 0, 0);
	sqe->cancel_flags = (__u32) flags | IORING_ASYNC_CANCEL_FD;
}

IOURINGINLINE void io_uring_prep_link_timeout(struct io_uring_sqe *sqe,
					      struct __kernel_timespec *ts,
					      unsigned flags)
{
	io_uring_prep_rw(IORING_OP_LINK_TIMEOUT, sqe, -1, ts, 1, 0);
	sqe->timeout_flags = flags;
}

IOURINGINLINE void io_uring_prep_connect(struct io_uring_sqe *sqe, int fd,
					 const struct sockaddr *addr,
					 socklen_t addrlen)
{
	io_uring_prep_rw(IORING_OP_CONNECT, sqe, fd, addr, 0, addrlen);
}

IOURINGINLINE void io_uring_prep_files_update(struct io_uring_sqe *sqe,
					      int *fds, unsigned nr_fds,
					      int offset)
{
	io_uring_prep_rw(IORING_OP_FILES_UPDATE, sqe, -1, fds, nr_fds,
				(__u64) offset);
}

IOURINGINLINE void io_uring_prep_fallocate(struct io_uring_sqe *sqe, int fd,
					   int mode, __u64 offset, __u64 len)
{
	io_uring_prep_rw(IORING_OP_FALLOCATE, sqe, fd,
			NULL, (unsigned int) mode, (__u64) offset);
	sqe->addr = (__u64) len;
}

IOURINGINLINE void io_uring_prep_openat(struct io_uring_sqe *sqe, int dfd,
					const char *path, int flags,
					mode_t mode)
{
	io_uring_prep_rw(IORING_OP_OPENAT, sqe, dfd, path, mode, 0);
	sqe->open_flags = (__u32) flags;
}

/* open directly into the fixed file table */
IOURINGINLINE void io_uring_prep_openat_direct(struct io_uring_sqe *sqe,
					       int dfd, const char *path,
					       int flags, mode_t mode,
					       unsigned file_index)
{
	io_uring_prep_openat(sqe, dfd, path, flags, mode);
	/* offset by 1 for allocation */
	if (file_index == IORING_FILE_INDEX_ALLOC)
		file_index--;
	__io_uring_set_target_fixed_file(sqe, file_index);
}

IOURINGINLINE void io_uring_prep_close(struct io_uring_sqe *sqe, int fd)
{
	io_uring_prep_rw(IORING_OP_CLOSE, sqe, fd, NULL, 0, 0);
}

IOURINGINLINE void io_uring_prep_close_direct(struct io_uring_sqe *sqe,
					      unsigned file_index)
{
	io_uring_prep_close(sqe, 0);
	__io_uring_set_target_fixed_file(sqe, file_index);
}

IOURINGINLINE void io_uring_prep_read(struct io_uring_sqe *sqe, int fd,
				      void *buf, unsigned nbytes, __u64 offset)
{
	io_uring_prep_rw(IORING_OP_READ, sqe, fd, buf, nbytes, offset);
}

IOURINGINLINE void io_uring_prep_write(struct io_uring_sqe *sqe, int fd,
				       const void *buf, unsigned nbytes,
				       __u64 offset)
{
	io_uring_prep_rw(IORING_OP_WRITE, sqe, fd, buf, nbytes, offset);
}

struct statx;
IOURINGINLINE void io_uring_prep_statx(struct io_uring_sqe *sqe, int dfd,
				       const char *path, int flags,
				       unsigned mask, struct statx *statxbuf)
{
	io_uring_prep_rw(IORING_OP_STATX, sqe, dfd, path, mask,
				(__u64) (unsigned long) statxbuf);
	sqe->statx_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_fadvise(struct io_uring_sqe *sqe, int fd,
					 __u64 offset, off_t len, int advice)
{
	io_uring_prep_rw(IORING_OP_FADVISE, sqe, fd, NULL, (__u32) len, offset);
	sqe->fadvise_advice = (__u32) advice;
}

IOURINGINLINE void io_uring_prep_madvise(struct io_uring_sqe *sqe, void *addr,
					 off_t length, int advice)
{
	io_uring_prep_rw(IORING_OP_MADVISE, sqe, -1, addr, (__u32) length, 0);
	sqe->fadvise_advice = (__u32) advice;
}

IOURINGINLINE void io_uring_prep_send(struct io_uring_sqe *sqe, int sockfd,
				      const void *buf, size_t len, int flags)
{
	io_uring_prep_rw(IORING_OP_SEND, sqe, sockfd, buf, (__u32) len, 0);
	sqe->msg_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_send_set_addr(struct io_uring_sqe *sqe,
						const struct sockaddr *dest_addr,
						__u16 addr_len)
{
	sqe->addr2 = (unsigned long)(const void *)dest_addr;
	sqe->addr_len = addr_len;
}

IOURINGINLINE void io_uring_prep_sendto(struct io_uring_sqe *sqe, int sockfd,
					const void *buf, size_t len, int flags,
					const struct sockaddr *addr,
					socklen_t addrlen)
{
	io_uring_prep_send(sqe, sockfd, buf, len, flags);
	io_uring_prep_send_set_addr(sqe, addr, addrlen);
}

IOURINGINLINE void io_uring_prep_send_zc(struct io_uring_sqe *sqe, int sockfd,
					 const void *buf, size_t len, int flags,
					 unsigned zc_flags)
{
	io_uring_prep_rw(IORING_OP_SEND_ZC, sqe, sockfd, buf, (__u32) len, 0);
	sqe->msg_flags = (__u32) flags;
	sqe->ioprio = zc_flags;
}

IOURINGINLINE void io_uring_prep_send_zc_fixed(struct io_uring_sqe *sqe,
						int sockfd, const void *buf,
						size_t len, int flags,
						unsigned zc_flags,
						unsigned buf_index)
{
	io_uring_prep_send_zc(sqe, sockfd, buf, len, flags, zc_flags);
	sqe->ioprio |= IORING_RECVSEND_FIXED_BUF;
	sqe->buf_index = buf_index;
}

IOURINGINLINE void io_uring_prep_sendmsg_zc(struct io_uring_sqe *sqe, int fd,
					    const struct msghdr *msg,
					    unsigned flags)
{
	io_uring_prep_sendmsg(sqe, fd, msg, flags);
	sqe->opcode = IORING_OP_SENDMSG_ZC;
}

IOURINGINLINE void io_uring_prep_recv(struct io_uring_sqe *sqe, int sockfd,
				      void *buf, size_t len, int flags)
{
	io_uring_prep_rw(IORING_OP_RECV, sqe, sockfd, buf, (__u32) len, 0);
	sqe->msg_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_recv_multishot(struct io_uring_sqe *sqe,
						int sockfd, void *buf,
						size_t len, int flags)
{
	io_uring_prep_recv(sqe, sockfd, buf, len, flags);
	sqe->ioprio |= IORING_RECV_MULTISHOT;
}

IOURINGINLINE struct io_uring_recvmsg_out *
io_uring_recvmsg_validate(void *buf, int buf_len, struct msghdr *msgh)
{
	unsigned long header = msgh->msg_controllen + msgh->msg_namelen +
				sizeof(struct io_uring_recvmsg_out);
	if (buf_len < 0 || (unsigned long)buf_len < header)
		return NULL;
	return (struct io_uring_recvmsg_out *)buf;
}

IOURINGINLINE void *io_uring_recvmsg_name(struct io_uring_recvmsg_out *o)
{
	return (void *) &o[1];
}

IOURINGINLINE struct cmsghdr *
io_uring_recvmsg_cmsg_firsthdr(struct io_uring_recvmsg_out *o,
			       struct msghdr *msgh)
{
	if (o->controllen < sizeof(struct cmsghdr))
		return NULL;

	return (struct cmsghdr *)((unsigned char *) io_uring_recvmsg_name(o) +
			msgh->msg_namelen);
}

IOURINGINLINE struct cmsghdr *
io_uring_recvmsg_cmsg_nexthdr(struct io_uring_recvmsg_out *o, struct msghdr *msgh,
			      struct cmsghdr *cmsg)
{
	unsigned char *end;

	if (cmsg->cmsg_len < sizeof(struct cmsghdr))
		return NULL;
	end = (unsigned char *) io_uring_recvmsg_cmsg_firsthdr(o, msgh) +
		o->controllen;
	cmsg = (struct cmsghdr *)((unsigned char *) cmsg +
			CMSG_ALIGN(cmsg->cmsg_len));

	if ((unsigned char *) (cmsg + 1) > end)
		return NULL;
	if (((unsigned char *) cmsg) + CMSG_ALIGN(cmsg->cmsg_len) > end)
		return NULL;

	return cmsg;
}

IOURINGINLINE void *io_uring_recvmsg_payload(struct io_uring_recvmsg_out *o,
					     struct msghdr *msgh)
{
	return (void *)((unsigned char *)io_uring_recvmsg_name(o) +
			msgh->msg_namelen + msgh->msg_controllen);
}

IOURINGINLINE unsigned int
io_uring_recvmsg_payload_length(struct io_uring_recvmsg_out *o,
				int buf_len, struct msghdr *msgh)
{
	unsigned long payload_start, payload_end;

	payload_start = (unsigned long) io_uring_recvmsg_payload(o, msgh);
	payload_end = (unsigned long) o + buf_len;
	return (unsigned int) (payload_end - payload_start);
}

IOURINGINLINE void io_uring_prep_openat2(struct io_uring_sqe *sqe, int dfd,
					const char *path, struct open_how *how)
{
	io_uring_prep_rw(IORING_OP_OPENAT2, sqe, dfd, path, 3 * sizeof(__u64),
				(uint64_t) (uintptr_t) how);
}

/* open directly into the fixed file table */
IOURINGINLINE void io_uring_prep_openat2_direct(struct io_uring_sqe *sqe,
						int dfd, const char *path,
						struct open_how *how,
						unsigned file_index)
{
	io_uring_prep_openat2(sqe, dfd, path, how);
	/* offset by 1 for allocation */
	if (file_index == IORING_FILE_INDEX_ALLOC)
		file_index--;
	__io_uring_set_target_fixed_file(sqe, file_index);
}

struct epoll_event;
IOURINGINLINE void io_uring_prep_epoll_ctl(struct io_uring_sqe *sqe, int epfd,
					   int fd, int op,
					   struct epoll_event *ev)
{
	io_uring_prep_rw(IORING_OP_EPOLL_CTL, sqe, epfd, ev,
				(__u32) op, (__u32) fd);
}

IOURINGINLINE void io_uring_prep_provide_buffers(struct io_uring_sqe *sqe,
						 void *addr, int len, int nr,
						 int bgid, int bid)
{
	io_uring_prep_rw(IORING_OP_PROVIDE_BUFFERS, sqe, nr, addr, (__u32) len,
				(__u64) bid);
	sqe->buf_group = (__u16) bgid;
}

IOURINGINLINE void io_uring_prep_remove_buffers(struct io_uring_sqe *sqe,
						int nr, int bgid)
{
	io_uring_prep_rw(IORING_OP_REMOVE_BUFFERS, sqe, nr, NULL, 0, 0);
	sqe->buf_group = (__u16) bgid;
}

IOURINGINLINE void io_uring_prep_shutdown(struct io_uring_sqe *sqe, int fd,
					  int how)
{
	io_uring_prep_rw(IORING_OP_SHUTDOWN, sqe, fd, NULL, (__u32) how, 0);
}

IOURINGINLINE void io_uring_prep_unlinkat(struct io_uring_sqe *sqe, int dfd,
					  const char *path, int flags)
{
	io_uring_prep_rw(IORING_OP_UNLINKAT, sqe, dfd, path, 0, 0);
	sqe->unlink_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_unlink(struct io_uring_sqe *sqe,
					  const char *path, int flags)
{
	io_uring_prep_unlinkat(sqe, AT_FDCWD, path, flags);
}

IOURINGINLINE void io_uring_prep_renameat(struct io_uring_sqe *sqe, int olddfd,
					  const char *oldpath, int newdfd,
					  const char *newpath, unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_RENAMEAT, sqe, olddfd, oldpath,
				(__u32) newdfd,
				(uint64_t) (uintptr_t) newpath);
	sqe->rename_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_rename(struct io_uring_sqe *sqe,
					const char *oldpath,
					const char *newpath)
{
	io_uring_prep_renameat(sqe, AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
}

IOURINGINLINE void io_uring_prep_sync_file_range(struct io_uring_sqe *sqe,
						 int fd, unsigned len,
						 __u64 offset, int flags)
{
	io_uring_prep_rw(IORING_OP_SYNC_FILE_RANGE, sqe, fd, NULL, len, offset);
	sqe->sync_range_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_mkdirat(struct io_uring_sqe *sqe, int dfd,
					const char *path, mode_t mode)
{
	io_uring_prep_rw(IORING_OP_MKDIRAT, sqe, dfd, path, mode, 0);
}

IOURINGINLINE void io_uring_prep_mkdir(struct io_uring_sqe *sqe,
					const char *path, mode_t mode)
{
	io_uring_prep_mkdirat(sqe, AT_FDCWD, path, mode);
}

IOURINGINLINE void io_uring_prep_symlinkat(struct io_uring_sqe *sqe,
					   const char *target, int newdirfd,
					   const char *linkpath)
{
	io_uring_prep_rw(IORING_OP_SYMLINKAT, sqe, newdirfd, target, 0,
				(uint64_t) (uintptr_t) linkpath);
}

IOURINGINLINE void io_uring_prep_symlink(struct io_uring_sqe *sqe,
					 const char *target,
					 const char *linkpath)
{
	io_uring_prep_symlinkat(sqe, target, AT_FDCWD, linkpath);
}

IOURINGINLINE void io_uring_prep_linkat(struct io_uring_sqe *sqe, int olddfd,
					const char *oldpath, int newdfd,
					const char *newpath, int flags)
{
	io_uring_prep_rw(IORING_OP_LINKAT, sqe, olddfd, oldpath, (__u32) newdfd,
				(uint64_t) (uintptr_t) newpath);
	sqe->hardlink_flags = (__u32) flags;
}

IOURINGINLINE void io_uring_prep_link(struct io_uring_sqe *sqe,
				      const char *oldpath, const char *newpath,
				      int flags)
{
	io_uring_prep_linkat(sqe, AT_FDCWD, oldpath, AT_FDCWD, newpath, flags);
}

IOURINGINLINE void io_uring_prep_msg_ring_cqe_flags(struct io_uring_sqe *sqe,
					  int fd, unsigned int len, __u64 data,
					  unsigned int flags, unsigned int cqe_flags)
{
	io_uring_prep_rw(IORING_OP_MSG_RING, sqe, fd, NULL, len, data);
	sqe->msg_ring_flags = IORING_MSG_RING_FLAGS_PASS | flags;
	sqe->file_index = cqe_flags;
}

IOURINGINLINE void io_uring_prep_msg_ring(struct io_uring_sqe *sqe, int fd,
					  unsigned int len, __u64 data,
					  unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_MSG_RING, sqe, fd, NULL, len, data);
	sqe->msg_ring_flags = flags;
}

IOURINGINLINE void io_uring_prep_msg_ring_fd(struct io_uring_sqe *sqe, int fd,
					     int source_fd, int target_fd,
					     __u64 data, unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_MSG_RING, sqe, fd,
			 (void *) (uintptr_t) IORING_MSG_SEND_FD, 0, data);
	sqe->addr3 = source_fd;
	/* offset by 1 for allocation */
	if ((unsigned int) target_fd == IORING_FILE_INDEX_ALLOC)
		target_fd--;
	__io_uring_set_target_fixed_file(sqe, target_fd);
	sqe->msg_ring_flags = flags;
}

IOURINGINLINE void io_uring_prep_msg_ring_fd_alloc(struct io_uring_sqe *sqe,
						   int fd, int source_fd,
						   __u64 data, unsigned int flags)
{
	io_uring_prep_msg_ring_fd(sqe, fd, source_fd, IORING_FILE_INDEX_ALLOC,
				  data, flags);
}

IOURINGINLINE void io_uring_prep_getxattr(struct io_uring_sqe *sqe,
					  const char *name, char *value,
					  const char *path, unsigned int len)
{
	io_uring_prep_rw(IORING_OP_GETXATTR, sqe, 0, name, len,
				(__u64) (uintptr_t) value);
	sqe->addr3 = (__u64) (uintptr_t) path;
	sqe->xattr_flags = 0;
}

IOURINGINLINE void io_uring_prep_setxattr(struct io_uring_sqe *sqe,
					  const char *name, const char *value,
					  const char *path, int flags,
					  unsigned int len)
{
	io_uring_prep_rw(IORING_OP_SETXATTR, sqe, 0, name, len,
				(__u64) (uintptr_t) value);
	sqe->addr3 = (__u64) (uintptr_t) path;
	sqe->xattr_flags = flags;
}

IOURINGINLINE void io_uring_prep_fgetxattr(struct io_uring_sqe *sqe,
					   int fd, const char *name,
					   char *value, unsigned int len)
{
	io_uring_prep_rw(IORING_OP_FGETXATTR, sqe, fd, name, len,
				(__u64) (uintptr_t) value);
	sqe->xattr_flags = 0;
}

IOURINGINLINE void io_uring_prep_fsetxattr(struct io_uring_sqe *sqe, int fd,
					   const char *name, const char	*value,
					   int flags, unsigned int len)
{
	io_uring_prep_rw(IORING_OP_FSETXATTR, sqe, fd, name, len,
				(__u64) (uintptr_t) value);
	sqe->xattr_flags = flags;
}

IOURINGINLINE void io_uring_prep_socket(struct io_uring_sqe *sqe, int domain,
					int type, int protocol,
					unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_SOCKET, sqe, domain, NULL, protocol, type);
	sqe->rw_flags = flags;
}

IOURINGINLINE void io_uring_prep_socket_direct(struct io_uring_sqe *sqe,
					       int domain, int type,
					       int protocol,
					       unsigned file_index,
					       unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_SOCKET, sqe, domain, NULL, protocol, type);
	sqe->rw_flags = flags;
	/* offset by 1 for allocation */
	if (file_index == IORING_FILE_INDEX_ALLOC)
		file_index--;
	__io_uring_set_target_fixed_file(sqe, file_index);
}

IOURINGINLINE void io_uring_prep_socket_direct_alloc(struct io_uring_sqe *sqe,
						     int domain, int type,
						     int protocol,
						     unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_SOCKET, sqe, domain, NULL, protocol, type);
	sqe->rw_flags = flags;
	__io_uring_set_target_fixed_file(sqe, IORING_FILE_INDEX_ALLOC - 1);
}


#define UNUSED(x) (void)(x)

/*
 * Prepare commands for sockets
 */
IOURINGINLINE void io_uring_prep_cmd_sock(struct io_uring_sqe *sqe,
					  int cmd_op,
					  int fd,
					  int level,
					  int optname,
					  void *optval,
					  int optlen)
{
	io_uring_prep_rw(IORING_OP_URING_CMD, sqe, fd, NULL, 0, 0);
	sqe->optval = (unsigned long) (uintptr_t) optval;
	sqe->optname = optname;
	sqe->optlen = optlen;
	sqe->cmd_op = cmd_op;
	sqe->level = level;
}

IOURINGINLINE void io_uring_prep_waitid(struct io_uring_sqe *sqe,
					idtype_t idtype,
					id_t id,
					siginfo_t *infop,
					int options, unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_WAITID, sqe, id, NULL, (unsigned) idtype, 0);
	sqe->waitid_flags = flags;
	sqe->file_index = options;
	sqe->addr2 = (unsigned long) infop;
}

IOURINGINLINE void io_uring_prep_futex_wake(struct io_uring_sqe *sqe,
					    uint32_t *futex, uint64_t val,
					    uint64_t mask, uint32_t futex_flags,
					    unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_FUTEX_WAKE, sqe, futex_flags, futex, 0, val);
	sqe->futex_flags = flags;
	sqe->addr3 = mask;
}

IOURINGINLINE void io_uring_prep_futex_wait(struct io_uring_sqe *sqe,
					    uint32_t *futex, uint64_t val,
					    uint64_t mask, uint32_t futex_flags,
					    unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_FUTEX_WAIT, sqe, futex_flags, futex, 0, val);
	sqe->futex_flags = flags;
	sqe->addr3 = mask;
}

struct futex_waitv;
IOURINGINLINE void io_uring_prep_futex_waitv(struct io_uring_sqe *sqe,
					     struct futex_waitv *futex,
					     uint32_t nr_futex,
					     unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_FUTEX_WAITV, sqe, 0, futex, nr_futex, 0);
	sqe->futex_flags = flags;
}

IOURINGINLINE void io_uring_prep_fixed_fd_install(struct io_uring_sqe *sqe,
						  int fd,
						  unsigned int flags)
{
	io_uring_prep_rw(IORING_OP_FIXED_FD_INSTALL, sqe, fd, NULL, 0, 0);
	sqe->flags = IOSQE_FIXED_FILE;
	sqe->install_fd_flags = flags;
}

IOURINGINLINE void io_uring_prep_ftruncate(struct io_uring_sqe *sqe,
				       int fd, loff_t len)
{
	io_uring_prep_rw(IORING_OP_FTRUNCATE, sqe, fd, NULL, 0, len);
}

/*
 * Returns number of unconsumed (if SQPOLL) or unsubmitted entries exist in
 * the SQ ring
 */
IOURINGINLINE unsigned io_uring_sq_ready(const struct io_uring *ring)
{
	unsigned khead = *ring->sq.khead;

	/*
	 * Without a barrier, we could miss an update and think the SQ wasn't
	 * ready. We don't need the load acquire for non-SQPOLL since then we
	 * drive updates.
	 */
	if (ring->flags & IORING_SETUP_SQPOLL)
		khead = io_uring_smp_load_acquire(ring->sq.khead);

	/* always use real head, to avoid losing sync for short submit */
	return ring->sq.sqe_tail - khead;
}

/*
 * Returns how much space is left in the SQ ring.
 */
IOURINGINLINE unsigned io_uring_sq_space_left(const struct io_uring *ring)
{
	return ring->sq.ring_entries - io_uring_sq_ready(ring);
}

/*
 * Only applicable when using SQPOLL - allows the caller to wait for space
 * to free up in the SQ ring, which happens when the kernel side thread has
 * consumed one or more entries. If the SQ ring is currently non-full, no
 * action is taken. Note: may return -EINVAL if the kernel doesn't support
 * this feature.
 */
IOURINGINLINE int io_uring_sqring_wait(struct io_uring *ring)
{
	if (!(ring->flags & IORING_SETUP_SQPOLL))
		return 0;
	if (io_uring_sq_space_left(ring))
		return 0;

	return __io_uring_sqring_wait(ring);
}

/*
 * Returns how many unconsumed entries are ready in the CQ ring
 */
IOURINGINLINE unsigned io_uring_cq_ready(const struct io_uring *ring)
{
	return io_uring_smp_load_acquire(ring->cq.ktail) - *ring->cq.khead;
}

/*
 * Returns true if there are overflow entries waiting to be flushed onto
 * the CQ ring
 */
IOURINGINLINE bool io_uring_cq_has_overflow(const struct io_uring *ring)
{
	return IO_URING_READ_ONCE(*ring->sq.kflags) & IORING_SQ_CQ_OVERFLOW;
}

/*
 * Returns true if the eventfd notification is currently enabled
 */
IOURINGINLINE bool io_uring_cq_eventfd_enabled(const struct io_uring *ring)
{
	if (!ring->cq.kflags)
		return true;

	return !(*ring->cq.kflags & IORING_CQ_EVENTFD_DISABLED);
}

/*
 * Toggle eventfd notification on or off, if an eventfd is registered with
 * the ring.
 */
IOURINGINLINE int io_uring_cq_eventfd_toggle(struct io_uring *ring,
					     bool enabled)
{
	uint32_t flags;

	if (!!enabled == io_uring_cq_eventfd_enabled(ring))
		return 0;

	if (!ring->cq.kflags)
		return -EOPNOTSUPP;

	flags = *ring->cq.kflags;

	if (enabled)
		flags &= ~IORING_CQ_EVENTFD_DISABLED;
	else
		flags |= IORING_CQ_EVENTFD_DISABLED;

	IO_URING_WRITE_ONCE(*ring->cq.kflags, flags);

	return 0;
}

/*
 * Return an IO completion, waiting for 'wait_nr' completions if one isn't
 * readily available. Returns 0 with cqe_ptr filled in on success, -errno on
 * failure.
 */
IOURINGINLINE int io_uring_wait_cqe_nr(struct io_uring *ring,
				      struct io_uring_cqe **cqe_ptr,
				      unsigned wait_nr)
{
	return __io_uring_get_cqe(ring, cqe_ptr, 0, wait_nr, NULL);
}

/*
 * Internal helper, don't use directly in applications. Use one of the
 * "official" versions of this, io_uring_peek_cqe(), io_uring_wait_cqe(),
 * or io_uring_wait_cqes*().
 */
IOURINGINLINE int __io_uring_peek_cqe(struct io_uring *ring,
				      struct io_uring_cqe **cqe_ptr,
				      unsigned *nr_available)
{
	struct io_uring_cqe *cqe;
	int err = 0;
	unsigned available;
	unsigned mask = ring->cq.ring_mask;
	int shift = 0;

	if (ring->flags & IORING_SETUP_CQE32)
		shift = 1;

	do {
		unsigned tail = io_uring_smp_load_acquire(ring->cq.ktail);
		unsigned head = *ring->cq.khead;

		cqe = NULL;
		available = tail - head;
		if (!available)
			break;

		cqe = &ring->cq.cqes[(head & mask) << shift];
		if (!(ring->features & IORING_FEAT_EXT_ARG) &&
				cqe->user_data == LIBURING_UDATA_TIMEOUT) {
			if (cqe->res < 0)
				err = cqe->res;
			io_uring_cq_advance(ring, 1);
			if (!err)
				continue;
			cqe = NULL;
		}

		break;
	} while (1);

	*cqe_ptr = cqe;
	if (nr_available)
		*nr_available = available;
	return err;
}

/*
 * Return an IO completion, if one is readily available. Returns 0 with
 * cqe_ptr filled in on success, -errno on failure.
 */
IOURINGINLINE int io_uring_peek_cqe(struct io_uring *ring,
				    struct io_uring_cqe **cqe_ptr)
{
	if (!__io_uring_peek_cqe(ring, cqe_ptr, NULL) && *cqe_ptr)
		return 0;

	return io_uring_wait_cqe_nr(ring, cqe_ptr, 0);
}

/*
 * Return an IO completion, waiting for it if necessary. Returns 0 with
 * cqe_ptr filled in on success, -errno on failure.
 */
IOURINGINLINE int io_uring_wait_cqe(struct io_uring *ring,
				    struct io_uring_cqe **cqe_ptr)
{
	if (!__io_uring_peek_cqe(ring, cqe_ptr, NULL) && *cqe_ptr)
		return 0;

	return io_uring_wait_cqe_nr(ring, cqe_ptr, 1);
}

/*
 * Return an sqe to fill. Application must later call io_uring_submit()
 * when it's ready to tell the kernel about it. The caller may call this
 * function multiple times before calling io_uring_submit().
 *
 * Returns a vacant sqe, or NULL if we're full.
 */
IOURINGINLINE struct io_uring_sqe *_io_uring_get_sqe(struct io_uring *ring)
{
	struct io_uring_sq *sq = &ring->sq;
	unsigned int head, next = sq->sqe_tail + 1;
	int shift = 0;

	if (ring->flags & IORING_SETUP_SQE128)
		shift = 1;
	if (!(ring->flags & IORING_SETUP_SQPOLL))
		head = IO_URING_READ_ONCE(*sq->khead);
	else
		head = io_uring_smp_load_acquire(sq->khead);

	if (next - head <= sq->ring_entries) {
		struct io_uring_sqe *sqe;

		sqe = &sq->sqes[(sq->sqe_tail & sq->ring_mask) << shift];
		sq->sqe_tail = next;
		return sqe;
	}

	return NULL;
}

/*
 * Return the appropriate mask for a buffer ring of size 'ring_entries'
 */
IOURINGINLINE int io_uring_buf_ring_mask(__u32 ring_entries)
{
	return ring_entries - 1;
}

IOURINGINLINE void io_uring_buf_ring_init(struct io_uring_buf_ring *br)
{
	br->tail = 0;
}

/*
 * Assign 'buf' with the addr/len/buffer ID supplied
 */
IOURINGINLINE void io_uring_buf_ring_add(struct io_uring_buf_ring *br,
					 void *addr, unsigned int len,
					 unsigned short bid, int mask,
					 int buf_offset)
{
	struct io_uring_buf *buf = &br->bufs[(br->tail + buf_offset) & mask];

	buf->addr = (unsigned long) (uintptr_t) addr;
	buf->len = len;
	buf->bid = bid;
}

/*
 * Make 'count' new buffers visible to the kernel. Called after
 * io_uring_buf_ring_add() has been called 'count' times to fill in new
 * buffers.
 */
IOURINGINLINE void io_uring_buf_ring_advance(struct io_uring_buf_ring *br,
					     int count)
{
	unsigned short new_tail = br->tail + count;

	io_uring_smp_store_release(&br->tail, new_tail);
}

IOURINGINLINE void __io_uring_buf_ring_cq_advance(struct io_uring *ring,
						  struct io_uring_buf_ring *br,
						  int cq_count, int buf_count)
{
	br->tail += buf_count;
	io_uring_cq_advance(ring, cq_count);
}

/*
 * Make 'count' new buffers visible to the kernel while at the same time
 * advancing the CQ ring seen entries. This can be used when the application
 * is using ring provided buffers and returns buffers while processing CQEs,
 * avoiding an extra atomic when needing to increment both the CQ ring and
 * the ring buffer index at the same time.
 */
IOURINGINLINE void io_uring_buf_ring_cq_advance(struct io_uring *ring,
						struct io_uring_buf_ring *br,
						int count)
{
	__io_uring_buf_ring_cq_advance(ring, br, count, count);
}

#ifndef LIBURING_INTERNAL
IOURINGINLINE struct io_uring_sqe *io_uring_get_sqe(struct io_uring *ring)
{
	return _io_uring_get_sqe(ring);
}
#else
struct io_uring_sqe *io_uring_get_sqe(struct io_uring *ring);
#endif

IOURINGEXTERN ssize_t io_uring_mlock_size(unsigned entries, unsigned flags);
IOURINGEXTERN ssize_t io_uring_mlock_size_params(unsigned entries, struct io_uring_params *p);

/*
 * Versioning information for liburing.
 *
 * Use IO_URING_CHECK_VERSION() for compile time checks including from
 * preprocessor directives.
 *
 * Use io_uring_check_version() for runtime checks of the version of
 * liburing that was loaded by the dynamic linker.
 */
IOURINGEXTERN int io_uring_major_version(void);
IOURINGEXTERN int io_uring_minor_version(void);
IOURINGEXTERN bool io_uring_check_version(int major, int minor);

#define IO_URING_CHECK_VERSION(major,minor) \
  (major > IO_URING_VERSION_MAJOR ||        \
   (major == IO_URING_VERSION_MAJOR &&      \
    minor >= IO_URING_VERSION_MINOR))

#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif

#ifdef IOURINGINLINE
#undef IOURINGINLINE
#endif
/* #ifdef IOURINGEXTERN */
/* #undef IOURINGEXTERN */
/* #endif */

#endif
