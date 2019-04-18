
AC_DEFUN([AX_HDF5],
[
AC_ARG_WITH([hdf5],
  [AS_HELP_STRING([--with-hdf5@<:@=ARG@:>@],
    [use HDF5 library from a standard location (ARG=yes),
     from the specified location (ARG=<path>),
     or disable it (ARG=no)
     @<:@ARG=yes@:>@ ])],
    [
    if test "$withval" = "no"; then
        want_hdf5="no"
        want_hdf5_path="no"
    elif test "$withval" = "yes"; then
        want_hdf5="yes"
        want_hdf5_path="no"
        ac_hdf5_path=""
    else
        want_hdf5="yes"
        want_hdf5_path="yes"
        ac_hdf5_path="$withval"
    fi
    ],
    [
      want_hdf5="yes"
      want_hdf5_path="no"
      ac_hdf5_path=""
    ])


AC_ARG_WITH([hdf5-libdir],
        AS_HELP_STRING([--with-hdf5-libdir=LIB_DIR],
        [Force given directory for HDF5 libraries. Note that this will override library path detection, so use this parameter only if default library detection fails and you know exactly where your HDF5 libraries are located.]),
        [
        if test -d "$withval"
        then
                ac_hdf5_lib_path="$withval"
        else
                AC_MSG_ERROR(--with-hdf5-libdir expected directory name)
        fi
        ],
        [ac_hdf5_lib_path=""]
)

if test "x$want_hdf5" = "xyes"; then
    AC_MSG_CHECKING(for HDF5)
    succeeded=no

    dnl On 64-bit systems check for system libraries in both lib64 and lib.
    dnl The former is specified by FHS, but e.g. Debian does not adhere to
    dnl this (as it rises problems for generic multi-arch support).
    dnl The last entry in the list is chosen by default when no libraries
    dnl are found, e.g. when only header-only libraries are installed!
    libsubdirs="lib"
    ax_arch=`uname -m`
    if test $ax_arch = x86_64 -o $ax_arch = ppc64 -o $ax_arch = s390x -o $ax_arch = sparc64; then
        libsubdirs="lib64 lib lib64"
    fi

    if test "$ac_hdf5_path" != ""; then
        HDF5_CFLAGS="-I$ac_hdf5_path/include"
        for ac_hdf5_path_tmp in $libsubdirs; do
                if test -d "$ac_hdf5_path"/"$ac_hdf5_path_tmp" ; then
                        HDF5_LDFLAGS="-L$ac_hdf5_path/$ac_hdf5_path_tmp"
                        break
                fi
        done
    else
        if test "$HDF5_INCLUDEDIR" != ""; then
            HDF5_CFLAGS="-I$HDF5_INCLUDEDIR"
        else
            for ac_hdf5_path_tmp in /usr /usr/local /opt /opt/local ; do
                if test -e "$ac_hdf5_path_tmp/include/hdf5.h"; then
                    HDF5_CFLAGS="-I$ac_hdf5_path_tmp/include"
                    break;
                fi
            done
        fi
        
        if test "$HDF5_LIBDIR" != ""; then
            HDF5_LDFLAGS="-L$HDF5_LIBDIR"
        fi
    fi

    dnl overwrite ld flags if we have required special directory with
    dnl --with-hdf5-libdir parameter
    if test "$ac_hdf5_lib_path" != ""; then
       HDF5_LDFLAGS="-L$ac_hdf5_lib_path"
    fi
    HDF5_LIBS="-lhdf5"

    CPPFLAGS_SAVED="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $HDF5_CFLAGS"
    export CPPFLAGS

    LDFLAGS_SAVED="$LDFLAGS"
    LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
    export LDFLAGS

    AC_REQUIRE([AC_PROG_CXX])
    AC_LANG_PUSH(C++)
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    @%:@include <hdf5.h>
    ]], [[ int i; ]])],[
        AC_MSG_RESULT(yes)
    succeeded=yes
    found_system=yes
        ],[
        ])
    AC_LANG_POP([C++])


    if test "$succeeded" != "yes" ; then
        AC_MSG_NOTICE([[We could not detect the HDF5 libraries.]])
        # execute ACTION-IF-NOT-FOUND (if present):
        ifelse([$2], , :, [$2])
    else
        AC_SUBST(HDF5_CFLAGS)
        AC_SUBST(HDF5_LIBS)
        AC_SUBST(HDF5_LDFLAGS)
        AC_DEFINE(HAVE_HDF5,,[define if the HDF5 library is available])
        # execute ACTION-IF-FOUND (if present):
        ifelse([$1], , :, [$1])
    fi

    CPPFLAGS="$CPPFLAGS_SAVED"
    LDFLAGS="$LDFLAGS_SAVED"
fi

])
