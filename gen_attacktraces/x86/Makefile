CC=    clang
CXX=   g++
LD=    clang




ifndef PROJ
PROJ = ref
#PROJ = amd64
#PROJ = ssse3
#PROJ = avx2
endif


SRC_DIR  = ./src
UTIL_DIR = ./utils


CFLAGS   := -O0 -g $(CFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer #-pg -g -fsanitize=address
CXXFLAGS := -O0 -g $(CPPFLAGS) -Wall -Wextra -fno-exceptions -fno-rtti -nostdinc++
INCPATH  := -I/usr/local/include -I/opt/local/include -I/usr/include -I$(SRC_DIR) -I$(UTIL_DIR)
LDFLAGS  := $(LDFLAGS) #-fsanitize=address
LIBPATH  = -L/usr/local/lib -L/opt/local/lib -L/usr/lib
LIBS     = -lcrypto #-lasan



ifeq ($(PROJ),ref)

SRC_EXT_DIRS  = ./src/ref
INCPATH      += -I./src/ref

else ifeq ($(PROJ),amd64)

SRC_EXT_DIRS  = ./src/ref ./src/amd64
INCPATH      += -I./src/ref -I./src/amd64

CFLAGS    += -D_BLAS_UINT64_
CXXFLAGS  += -D_BLAS_UINT64_

else ifeq ($(PROJ),neon)

SRC_EXT_DIRS  = ./src/ref ./src/amd64 ./src/neon
INCPATH      += -I./src/ref -I./src/amd64 -I./src/neon

ifeq ($(CC),clang)
CFLAGS    += -flax-vector-conversions -D_BLAS_NEON_
CXXFLAGS  += -flax-vector-conversions -D_BLAS_NEON_
else
CFLAGS    += march=armv7-a -mfpu=neon -flax-vector-conversions -D_BLAS_NEON_
CXXFLAGS  += march=armv7-a -mfpu=neon -flax-vector-conversions -D_BLAS_NEON_
endif

else ifeq ($(PROJ),ssse3)

SRC_EXT_DIRS  = ./src/ref ./src/amd64 ./src/ssse3
INCPATH      += -I./src/ref -I./src/amd64 -I./src/ssse3

CFLAGS    += -mssse3  -D_BLAS_SSE_ -D_MUL_WITH_MULTAB_
CXXFLAGS  += -mssse3  -D_BLAS_SSE_ -D_MUL_WITH_MULTAB_

else ifeq ($(PROJ),avx2)

SRC_EXT_DIRS  = ./src/ref ./src/amd64 ./src/ssse3 ./src/avx2
INCPATH      += -I./src/ref -I./src/amd64 -I./src/ssse3 -I./src/avx2
CFLAGS       += -mavx2  -D_BLAS_AVX2_ -D_MUL_WITH_MULTAB_
CXXFLAGS     += -mavx2  -D_BLAS_AVX2_ -D_MUL_WITH_MULTAB_

endif


SRCS    :=  $(foreach dir, $(SRC_EXT_DIRS), $(wildcard  $(dir)/*.c) )
SRCS    +=  $(wildcard $(SRC_DIR)/*.c)   $(wildcard $(UTIL_DIR)/*.c)

SRCS_O         :=  $(SRCS:.c=.o)
SRCS_O_NOTDIR  :=  $(notdir $(SRCS_O))


ifdef PARAM
ifeq ($(PARAM),2)
CFLAGS    += -D_UOV256_96_64
CXXFLAGS  += -D_UOV256_96_64
else ifeq ($(PARAM),3)
CFLAGS    += -D_UOV256_112_44
CXXFLAGS  += -D_UOV256_112_44
else ifeq ($(PARAM),4)
CFLAGS    += -D_UOV256_184_72
CXXFLAGS  += -D_UOV256_184_72
else ifeq ($(PARAM),5)
CFLAGS    += -D_UOV256_244_96
CXXFLAGS  += -D_UOV256_244_96
else
CFLAGS    += -D_UOV16_96_64
CXXFLAGS  += -D_UOV16_96_64
endif
endif


ifdef VARIANT
ifeq ($(VARIANT),2)
CFLAGS += -D_UOV_CIRCUMZENITHAL
CXXFLAGS += -D_UOV_CIRCUMZENITHAL
else ifeq ($(VARIANT),3)
CFLAGS += -D_UOV_COMPRESSED
CXXFLAGS += -D_UOV_COMPRESSED
else
CFLAGS += -D_UOV_CLASSIC
CXXFLAGS += -D_UOV_CLASSIC
endif
endif


OS := $(shell uname -s)
ifeq  ($(OS), Darwin)
CFLAGS    +=  -D_MAC_OS_
CXXFLAGS  +=  -D_MAC_OS_
endif

OBJ = $(SRCS_O_NOTDIR)

.INTERMEDIATE:  $(OBJ)

EXE= uov-test uov-cz-test sign_api-benchmark
#PQCgenKAT_sign cpk-pk-test crypto_sign-benchmark
# blas-elim-test 


ifdef DEBUG
        CFLAGS+=  -D_DEBUG_ -g
        CXXFLAGS+= -D_DEBUG_ -g
endif

ifdef NO_SSE
	CFLAGS += -D_NO_SSE_
	CXXFLAGS += -D_NO_SSE_
endif


ifdef K
	CFLAGS += -DK=$(K)
	CXXFLAGS += -DK=$(K)
endif


ifdef AVX2
	CFLAGS += -mavx2 -D_USE_AVX2_
	CXXFLAGS += -mavx2 -D_USE_AVX2_
endif

ifdef AVX
	CFLAGS += -mavx -D_USE_AVX_
	CXXFLAGS += -mavx -D_USE_AVX_
endif

ifdef GPROF
	CFLAGS += -pg
	CXXFLAGS += -pg
	LDFLAGS += -pg
endif

.PHONY: all tests tables clean

all: $(EXE)


%-test: $(OBJ) %-test.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)

%-benchmark: $(OBJ) %-benchmark.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)


rainbow-genkey: $(OBJ) rainbow-genkey.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)

rainbow-sign: $(OBJ) rainbow-sign.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)

rainbow-verify: $(OBJ) rainbow-verify.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)

PQCgenKAT_sign: $(OBJ) PQCgenKAT_sign.o
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LIBS)



%.o: unit_tests/%.c
	$(CC) $(CFLAGS) $(INCPATH) -c $<

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCPATH) -c $<

%.o: $(UTIL_DIR)/%.c
	$(CC) $(CFLAGS) $(INCPATH) -c $<

define GEN_O
%.o: $(1)/%.c
	$(CC) $(CFLAGS) $(INCPATH) -c $$<
endef
$(foreach dir, $(SRC_EXT_DIRS), $(eval $(call GEN_O,$(dir))))



%.o: %.c
	$(CC) $(CFLAGS) $(INCPATH) -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $<

tests:
	cd tests; make

tables:
	cd supplement; make

clean:
	-rm -f *.o *-test *-benchmark massif.out.* $(EXE);
