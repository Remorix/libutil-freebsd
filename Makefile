LIB?=		util-fbsd
SHLIB_MAJOR?=	10

SRCS=	_secure_path.c expand_number.c flopen.c fparseln.c \
	ftime.c getlocalbase.c gr_util.c \
	hexdump.c \
	kinfo_getallproc.c kinfo_getproc.c kinfo_getvmmap.c \
	login_auth.c login_cap.c \
	login_class.c login_crypt.c login_ok.c login_times.c login_tty.c \
	mntopts.c \
	pidfile.c property.c pw_util.c quotafile.c \
	trimdomain.c uucplock.c
INCS=	libutil.h login_cap.h mntopts.h

MAN=	expand_number.3 flopen.3 fparseln.3 \
	ftime.3 getlocalbase.3 hexdump.3 humanize_number.3 \
	kinfo_getallproc.3 kinfo_getproc.3 kinfo_getvmmap.3 \
	login_auth.3 login_cap.3 login_class.3 login_ok.3 login_times.3 \
	login_tty.3 mntopts.3 pidfile.3 property.3 pw_util.3 quotafile.3 \
	_secure_path.3 trimdomain.3 uucplock.3 login.conf.5
MLINKS=	flopen.3 flopenat.3 \
	login_auth.3 auth_cat.3 \
	login_auth.3 auth_checknologin.3 \
	login_cap.3 login_close.3 \
	login_cap.3 login_getcapbool.3 \
	login_cap.3 login_getcapenum.3 \
	login_cap.3 login_getcaplist.3 \
	login_cap.3 login_getcapnum.3 \
	login_cap.3 login_getcapsize.3 \
	login_cap.3 login_getcapstr.3 \
	login_cap.3 login_getcaptime.3 \
	login_cap.3 login_getclass.3 \
	login_cap.3 login_getclassbyname.3 \
	login_cap.3 login_getpath.3 \
	login_cap.3 login_getpwclass.3 \
	login_cap.3 login_getstyle.3 \
	login_cap.3 login_getuserclass.3 \
	login_cap.3 login_setcryptfmt.3 \
	login_class.3 setclasscontext.3 \
	login_class.3 setclasscpumask.3 \
	login_class.3 setclassenvironment.3 \
	login_class.3 setclassresources.3 \
	login_class.3 setusercontext.3 \
	login_ok.3 auth_hostok.3 \
	login_ok.3 auth_timeok.3 \
	login_ok.3 auth_ttyok.3 \
	login_times.3 in_lt.3 \
	login_times.3 in_ltm.3 \
	login_times.3 in_ltms.3 \
	login_times.3 in_lts.3 \
	login_times.3 parse_lt.3 \
	mntopts.3 build_iovec.3 \
	mntopts.3 build_iovec_argf.3 \
	mntopts.3 checkpath.3 \
	mntopts.3 chkdoreload.3 \
	mntopts.3 free_iovec.3 \
	mntopts.3 getmntopts.3 \
	mntopts.3 getmntpoint.3 \
	mntopts.3 rmslashes.3 \
	pidfile.3 pidfile_close.3 \
	pidfile.3 pidfile_fileno.3 \
	pidfile.3 pidfile_open.3 \
	pidfile.3 pidfile_remove.3 \
	pidfile.3 pidfile_signal.3 \
	pidfile.3 pidfile_write.3 \
	property.3 property_find.3 \
	property.3 properties_free.3 \
	property.3 properties_read.3 \
	pw_util.3 pw_copy.3 \
	pw_util.3 pw_dup.3 \
	pw_util.3 pw_edit.3 \
	pw_util.3 pw_equal.3 \
	pw_util.3 pw_fini.3 \
	pw_util.3 pw_init.3 \
	pw_util.3 pw_initpwd.3 \
	pw_util.3 pw_make.3 \
	pw_util.3 pw_make_v7.3 \
	pw_util.3 pw_mkdb.3 \
	pw_util.3 pw_lock.3 \
	pw_util.3 pw_scan.3 \
	pw_util.3 pw_tempname.3 \
	pw_util.3 pw_tmp.3 \
	quotafile.3 quota_check_path.3 \
	quotafile.3 quota_close.3 \
	quotafile.3 quota_convert.3 \
	quotafile.3 quota_fsname.3 \
	quotafile.3 quota_maxid.3 \
	quotafile.3 quota_off.3 \
	quotafile.3 quota_on.3 \
	quotafile.3 quota_open.3 \
	quotafile.3 quota_qfname.3 \
	quotafile.3 quota_read.3 \
	quotafile.3 quota_write_limits.3 \
	quotafile.3 quota_write_usage.3 \
	uucplock.3 uu_lock.3 \
	uucplock.3 uu_lock_txfr.3 \
	uucplock.3 uu_lockerr.3 \
	uucplock.3 uu_unlock.3

CC?=		cc
AR?=		ar
RANLIB?=	ranlib
INSTALL?=	install
LN?=		ln
RM?=		rm -f
MKDIR?=		mkdir -p

.if empty(RANLIB)
RANLIB=		ranlib
.endif

CFLAGS?=	-O2
CPPFLAGS?=
LDFLAGS?=
LDADD?=		-L. -lcrypt-fbsd
PICFLAG?=	-fPIC

CFLAGS+=	-DNO__SCCSID -I.

MK_INET6_SUPPORT?= yes
.if ${MK_INET6_SUPPORT:tl} != "no"
CFLAGS+=	-DINET6
.endif

UNAME_S!=	uname -s

.if ${UNAME_S} == "Darwin"
SHLIB_EXT?=	dylib
SHLIB_LINK?=    lib${LIB}.${SHLIB_MAJOR}.${SHLIB_EXT}
SHLIB_LDFLAGS?=	-dynamiclib \
		-Wl,-install_name,@rpath/${SHLIB_LINK} \
		-Wl,-compatibility_version,${SHLIB_MAJOR} \
		-Wl,-current_version,${SHLIB_MAJOR}
.else
SHLIB_EXT?=	so
SHLIB_LINK?=    lib${LIB}.${SHLIB_EXT}.${SHLIB_MAJOR}
SHLIB_LDFLAGS?=	-shared -Wl,-soname,${SHLIB_NAME}
.endif

# libutil is not using versioned path
SHLIB_NAME?=    lib${LIB}.${SHLIB_EXT}

OBJS=		${SRCS:.c=.o}
STATICLIB=	lib${LIB}.a
BUILD_SHARED?=	yes

.if ${BUILD_SHARED:tl} == "yes"
ALL_LIBS=	${STATICLIB} ${SHLIB_NAME} ${SHLIB_LINK}
.else
ALL_LIBS=	${STATICLIB}
.endif

PREFIX?=	/usr/local
LIBDIR?=	${PREFIX}/lib
INCLUDEDIR?=	${PREFIX}/include
MANDIR?=	${PREFIX}/share/man
MANMODE?=	644

.PHONY: all clean install install-headers install-libs install-man check-manlinks shared

all: ${ALL_LIBS}

${STATICLIB}: ${OBJS}
	${AR} rcs ${.TARGET} ${OBJS}
	${RANLIB} ${.TARGET}

${SHLIB_NAME}: ${OBJS}
	${CC} ${LDFLAGS} ${SHLIB_LDFLAGS} -o ${.TARGET} ${OBJS} ${LDADD}

${SHLIB_LINK}: ${SHLIB_NAME}
	${LN} -sf ${SHLIB_NAME} ${.TARGET}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${PICFLAG} -c ${.IMPSRC} -o ${.TARGET}

shared: ${SHLIB_NAME} ${SHLIB_LINK}

install: install-headers install-libs install-man

install-headers: ${INCS}
	${MKDIR} ${DESTDIR}${INCLUDEDIR}
	${INSTALL} -m 644 ${INCS} ${DESTDIR}${INCLUDEDIR}

install-libs: all
	${MKDIR} ${DESTDIR}${LIBDIR}
	${INSTALL} -m 644 ${STATICLIB} ${DESTDIR}${LIBDIR}/${STATICLIB}
.if ${BUILD_SHARED:tl} == "yes"
	${INSTALL} -m 755 ${SHLIB_NAME} ${DESTDIR}${LIBDIR}/${SHLIB_NAME}
	${LN} -sf ${SHLIB_NAME} ${DESTDIR}${LIBDIR}/${SHLIB_LINK}
.endif

check-manlinks:
	set -- ${MLINKS}; \
	while [ $$# -ge 2 ]; do \
		src="$$1"; dst="$$2"; shift 2; \
		if [ ! -f "$$src" ]; then \
			echo "missing source man page: $$src (for $$dst)" >&2; \
			exit 1; \
		fi; \
	done; \
	if [ $$# -ne 0 ]; then \
		echo "MLINKS has an odd number of entries" >&2; \
		exit 1; \
	fi

install-man: check-manlinks ${MAN}
	set -e; \
	for man in ${MAN}; do \
		sect=$${man##*.}; \
		${MKDIR} ${DESTDIR}${MANDIR}/man$${sect}; \
		${INSTALL} -m ${MANMODE} "$$man" ${DESTDIR}${MANDIR}/man$${sect}/"$$man"; \
	done
	set -e; \
	set -- ${MLINKS}; \
	while [ $$# -ge 2 ]; do \
		src="$$1"; dst="$$2"; shift 2; \
		srcsect=$${src##*.}; dstsect=$${dst##*.}; \
		if [ "$$srcsect" = "$$dstsect" ]; then \
			link_target="$$src"; \
		else \
			link_target="../man$$srcsect/$$src"; \
		fi; \
		${MKDIR} ${DESTDIR}${MANDIR}/man$$dstsect; \
		${LN} -sf "$$link_target" ${DESTDIR}${MANDIR}/man$$dstsect/$$dst; \
	done

clean:
	${RM} ${OBJS} ${STATICLIB} ${SHLIB_NAME} ${SHLIB_LINK}
