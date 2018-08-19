ifeq ($(subdir),csu)
tests += test-multiarch
endif

ifeq ($(subdir),string)
gen-as-const-headers += locale-defines.sym
endif

ifeq ($(subdir),math)
libm-sysdep_routines += s_fma-fma s_fmaf-fma
CFLAGS-s_fma-fma.c += -mavx -mfpmath=sse
CFLAGS-s_fmaf-fma.c += -mavx -mfpmath=sse
endif
