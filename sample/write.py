net/ipv4/tcp.c: remove superfluous header files from tcp.c

tcp.c hasn't use any macro or function declared in udp.h, types.h,
and net_namespace.h. Thus, these files can be removed from tcp.c
safely without affecting the compilation of the net module.