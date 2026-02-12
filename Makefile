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

.PHONY: all clean install install-headers install-libs shared

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

install: install-headers install-libs

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

clean:
	${RM} ${OBJS} ${STATICLIB} ${SHLIB_NAME} ${SHLIB_LINK}
