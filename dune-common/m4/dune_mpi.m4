# $Id: dune_mpi.m4 4910 2007-04-18 21:46:02Z christi $

# wrapper for the autoconf-archive check. Note: compiling MPI-stuff sucks!

# Explanation:
# ============
#

# compilation of MPI-programs is normally done by a
# mpicc/mpiCC-wrapper that adds all options needed. Thus, it may seem
# possible to just replace the compiler call by the wrapper and
# everything works. Unfortunately that's not the case: automake and
# libtool both show strange behaviour.
#
# In detail: replacing the compiler globally via ./configure CXX=mpiCC
# should work (at least I've found reports claiming this) but that is
# not what we want: mainly, it just adds a level of possible errors
# (mpiCC from MPICH does _nothing_ if "mpicc -c dummy.cc" is called!)
# and might introduce nice library-clashes.
#
# The next approach would be to include
#       if MPI
#         CXX = $(MPICXX)
#       endif
# in the Makefile.am where MPI is needed. First, this will change
# compilations of all binaries in this directory and secondly the
# dependency-tracking seems to break: the first compilation worked but
# the second failed with the compiler complaining about mismatching
# flags... There is no 'program_CXX = ...' in automake but even if
# there were it would break as well
#
# Thus, the best solution is to extract the flags needed for
# compilation and linking. Unfortunately, the parameters and behaviour
# of mpicc is not at all consistent over different
# implementations. For MPICH the parameters -compile_info and
# -link_info exist (albeit not being documented in the manpage, only
# in -help), for LAM dummy-calls of compilation and linking together
# with a -showme parameter (which is called -show in MPICH...) have to
# be used. Obviously, we have to identify the type of package... this
# is done via mpiCC-calls for now, I wouldn't be surprised if ths
# breaks often. Bad luck. Blame the MPI folks for this mess. And blame
# them a lot. [Thimo 26.8.2004]

# Sometimes ACX_MPI will not be able to find the correct MPI compiler,
# or sometimes you might have several MPI installations. Specify the
# MPICC variable to enforce a certain MPI compiler.

# In order to disable the usage of MPI (and make Dune purely
# sequential) you can supply the option
#   --disable-parallel
# [Christian 9.7.2006]

AC_DEFUN([DUNE_MPI],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  # get compilation script
  AC_LANG_CASE([C],[
    dune_mpi_isgnu="$GCC"
  ],
  [C++],[
    dune_mpi_isgnu="$GXX"
  ])

  AC_LANG_PUSH([C])

  # enable/disable parallel features
  AC_ARG_ENABLE(parallel,
    AC_HELP_STRING([--enable-parallel],
      [Enable the parallel features of Dune. If enabled
       configure will try to determine your MPI automatically. You can
       overwrite this setting by specifying the MPICC variable]),
    [with_parallel=$enableval],
    [with_parallel=no]
  )
  AC_SUBST(ENABLE_PARALLEL, "$with_parallel")

  # disable runtest if we have a queuing system
  AC_ARG_ENABLE(mpiruntest,
    AC_HELP_STRING([--disable-mpiruntest],
      [Don't try to run a MPI program during configure. (This is need if you depend on a queuing system)]),
    [mpiruntest=${enableval}],
    [mpiruntest=yes]
  )

  with_mpi="no"

  ## do nothing if --disable-parallel is used
  if test x$with_parallel == xyes ; then
  
    ACX_MPI([
      MPICOMP="$MPICC"

      MPI_CONFIG()
      MPI_CPPFLAGS="$MPI_CPPFLAGS $MPI_NOCXXFLAGS -DENABLE_MPI=1"

      with_mpi="yes ($MPI_VERSION)"
    ],[
      # ACX_MPI didn't find anything
      with_mpi="no"
    ])
  fi # end of MPI identification

  # if an MPI implementation was found..
  if test x"$with_mpi" != xno ; then
    ### do a sanity check: can we compile and link a trivial MPI program?
    AC_MSG_CHECKING([whether compiling with $MPI_VERSION works])

    # store old values
    ac_save_LIBS="$LIBS"
    ac_save_CPPFLAGS="$CPPFLAGS"
    
    # looks weird but as the -l... are contained in the MPI_LDFLAGS these
    # parameters have to be last on the commandline: with LIBS this is true
    LIBS="$MPI_LDFLAGS"
    CPPFLAGS="$CPPFLAGS $MPI_CPPFLAGS"

    # try to create MPI program
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE(
      AC_LANG_SOURCE(
        [ #include <mpi.h>
          int main (int argc, char** argv) { 
          MPI_Init(&argc, &argv); 
          MPI_Finalize(); }]),
        [ AC_MSG_RESULT([yes]) ],
        [ AC_MSG_RESULT([no])
          AC_MSG_ERROR([could not compile MPI testprogram!
          See config.log for details])
          with_mpi=no]
    )

    if test "x$mpiruntest" != "xyes" ; then
      AC_MSG_WARN([Diabled test whether running with $MPI_VERSION works.])    
    else
      AC_MSG_CHECKING([whether running with $MPI_VERSION works])
      AC_RUN_IFELSE(
        AC_LANG_SOURCE(
          [ #include <mpi.h>
            int main (int argc, char** argv) { 
            MPI_Init(&argc, &argv); 
            MPI_Finalize(); }]),
          [ AC_MSG_RESULT([yes]) ],
          [ AC_MSG_RESULT([no])
            AC_MSG_ERROR([could not run MPI testprogram!
            Did you forget to setup your MPI environment?
            Some MPI implementations require a special deamon to be running!
            If you don't want to use MPI you can use --disable-parallel to disable
            all parallel code in Dune.
            If you want to use parallel code, but cannot run the MPI run test
            during configure (This is needed if you depend on a queuing system), you
            might use the --disable-mpiruntest switch.
            See config.log for details])
            with_mpi=no]
      )
    fi
    AC_LANG_POP

    # restore variables
    LIBS="$ac_save_LIBS"
    CPPFLAGS="$ac_save_CPPFLAGS"
  fi
    
  # set flags
  if test x"$with_mpi" != xno ; then
    AC_SUBST(MPI_CPPFLAGS, $MPI_CPPFLAGS)
    AC_SUBST(MPI_LDFLAGS, $MPI_LDFLAGS)
    AC_SUBST(MPI_VERSION, $MPI_VERSION)
    AC_DEFINE(HAVE_MPI,ENABLE_MPI,[Define if you have the MPI library.
    This is only true if MPI was found by configure 
    _and_ if the application uses the MPI_CPPFLAGS])
  else
    AC_SUBST(MPI_CPPFLAGS, "")
    AC_SUBST(MPI_LDFLAGS, "")
  fi

  AM_CONDITIONAL(MPI, test x"$with_mpi" != xno)

  AC_LANG_POP
])