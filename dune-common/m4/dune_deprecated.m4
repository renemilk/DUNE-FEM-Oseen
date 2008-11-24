# $Id: dune_deprecated.m4 5007 2007-10-16 10:50:35Z christi $

# Check for the right way to create the deprecated warning

AC_DEFUN([DUNE_CHECKDEPRECATED],[
	AC_MSG_CHECKING([for __attribute__((deprecated))])
        AC_TRY_COMPILE([#define DEP __attribute__((deprecated))
                    class bar { bar() DEP; };
                    class peng { } DEP;
                    template <class T>
                    class t_bar { t_bar() DEP; };
                    template <class T>
                    class t_peng { t_peng() {}; } DEP;
                    void foo() DEP;
                    void foo() {};],[],
				   [DUNE_DEPRECATED="__attribute__((deprecated))"
                    AC_MSG_RESULT(yes)],
				   [DUNE_DEPRECATED=""
                    AC_MSG_RESULT(no)])

    AC_DEFINE_UNQUOTED(DUNE_DEPRECATED, $DUNE_DEPRECATED,
                      [how to create a deprecated warning])
])