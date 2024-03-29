#include <assert.h>
#include <string.h>
#include "uvwasi.h"
#include "test-common.h"

static void check(uvwasi_errno_t err, const char* str) {
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(err), str));
}

int main(void) {
  setup_test_environment();

  check(UVWASI_E2BIG, "UVWASI_E2BIG");
  check(UVWASI_EACCES, "UVWASI_EACCES");
  check(UVWASI_EADDRINUSE, "UVWASI_EADDRINUSE");
  check(UVWASI_EADDRNOTAVAIL, "UVWASI_EADDRNOTAVAIL");
  check(UVWASI_EAFNOSUPPORT, "UVWASI_EAFNOSUPPORT");
  check(UVWASI_EAGAIN, "UVWASI_EAGAIN");
  check(UVWASI_EALREADY, "UVWASI_EALREADY");
  check(UVWASI_EBADF, "UVWASI_EBADF");
  check(UVWASI_EBADMSG, "UVWASI_EBADMSG");
  check(UVWASI_EBUSY, "UVWASI_EBUSY");
  check(UVWASI_ECANCELED, "UVWASI_ECANCELED");
  check(UVWASI_ECHILD, "UVWASI_ECHILD");
  check(UVWASI_ECONNABORTED, "UVWASI_ECONNABORTED");
  check(UVWASI_ECONNREFUSED, "UVWASI_ECONNREFUSED");
  check(UVWASI_ECONNRESET, "UVWASI_ECONNRESET");
  check(UVWASI_EDEADLK, "UVWASI_EDEADLK");
  check(UVWASI_EDESTADDRREQ, "UVWASI_EDESTADDRREQ");
  check(UVWASI_EDOM, "UVWASI_EDOM");
  check(UVWASI_EDQUOT, "UVWASI_EDQUOT");
  check(UVWASI_EEXIST, "UVWASI_EEXIST");
  check(UVWASI_EFAULT, "UVWASI_EFAULT");
  check(UVWASI_EFBIG, "UVWASI_EFBIG");
  check(UVWASI_EHOSTUNREACH, "UVWASI_EHOSTUNREACH");
  check(UVWASI_EIDRM, "UVWASI_EIDRM");
  check(UVWASI_EILSEQ, "UVWASI_EILSEQ");
  check(UVWASI_EINPROGRESS, "UVWASI_EINPROGRESS");
  check(UVWASI_EINTR, "UVWASI_EINTR");
  check(UVWASI_EINVAL, "UVWASI_EINVAL");
  check(UVWASI_EIO, "UVWASI_EIO");
  check(UVWASI_EISCONN, "UVWASI_EISCONN");
  check(UVWASI_EISDIR, "UVWASI_EISDIR");
  check(UVWASI_ELOOP, "UVWASI_ELOOP");
  check(UVWASI_EMFILE, "UVWASI_EMFILE");
  check(UVWASI_EMLINK, "UVWASI_EMLINK");
  check(UVWASI_EMSGSIZE, "UVWASI_EMSGSIZE");
  check(UVWASI_EMULTIHOP, "UVWASI_EMULTIHOP");
  check(UVWASI_ENAMETOOLONG, "UVWASI_ENAMETOOLONG");
  check(UVWASI_ENETDOWN, "UVWASI_ENETDOWN");
  check(UVWASI_ENETRESET, "UVWASI_ENETRESET");
  check(UVWASI_ENETUNREACH, "UVWASI_ENETUNREACH");
  check(UVWASI_ENFILE, "UVWASI_ENFILE");
  check(UVWASI_ENOBUFS, "UVWASI_ENOBUFS");
  check(UVWASI_ENODEV, "UVWASI_ENODEV");
  check(UVWASI_ENOENT, "UVWASI_ENOENT");
  check(UVWASI_ENOEXEC, "UVWASI_ENOEXEC");
  check(UVWASI_ENOLCK, "UVWASI_ENOLCK");
  check(UVWASI_ENOLINK, "UVWASI_ENOLINK");
  check(UVWASI_ENOMEM, "UVWASI_ENOMEM");
  check(UVWASI_ENOMSG, "UVWASI_ENOMSG");
  check(UVWASI_ENOPROTOOPT, "UVWASI_ENOPROTOOPT");
  check(UVWASI_ENOSPC, "UVWASI_ENOSPC");
  check(UVWASI_ENOSYS, "UVWASI_ENOSYS");
  check(UVWASI_ENOTCONN, "UVWASI_ENOTCONN");
  check(UVWASI_ENOTDIR, "UVWASI_ENOTDIR");
  check(UVWASI_ENOTEMPTY, "UVWASI_ENOTEMPTY");
  check(UVWASI_ENOTRECOVERABLE, "UVWASI_ENOTRECOVERABLE");
  check(UVWASI_ENOTSOCK, "UVWASI_ENOTSOCK");
  check(UVWASI_ENOTSUP, "UVWASI_ENOTSUP");
  check(UVWASI_ENOTTY, "UVWASI_ENOTTY");
  check(UVWASI_ENXIO, "UVWASI_ENXIO");
  check(UVWASI_EOVERFLOW, "UVWASI_EOVERFLOW");
  check(UVWASI_EOWNERDEAD, "UVWASI_EOWNERDEAD");
  check(UVWASI_EPERM, "UVWASI_EPERM");
  check(UVWASI_EPIPE, "UVWASI_EPIPE");
  check(UVWASI_EPROTO, "UVWASI_EPROTO");
  check(UVWASI_EPROTONOSUPPORT, "UVWASI_EPROTONOSUPPORT");
  check(UVWASI_EPROTOTYPE, "UVWASI_EPROTOTYPE");
  check(UVWASI_ERANGE, "UVWASI_ERANGE");
  check(UVWASI_EROFS, "UVWASI_EROFS");
  check(UVWASI_ESPIPE, "UVWASI_ESPIPE");
  check(UVWASI_ESRCH, "UVWASI_ESRCH");
  check(UVWASI_ESTALE, "UVWASI_ESTALE");
  check(UVWASI_ETIMEDOUT, "UVWASI_ETIMEDOUT");
  check(UVWASI_ETXTBSY, "UVWASI_ETXTBSY");
  check(UVWASI_EXDEV, "UVWASI_EXDEV");
  check(UVWASI_ENOTCAPABLE, "UVWASI_ENOTCAPABLE");
  check(UVWASI_ESUCCESS, "UVWASI_ESUCCESS");
  check(34000, "UVWASI_UNKNOWN_ERROR");

  return 0;
}
