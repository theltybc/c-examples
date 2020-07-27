FILE=open_mp.c

OUT_FILE=open_mp

CC=clang-10

STD=gnu11
# STD=c11
# STD=c99


CHECKS_ON=\
  -ferror-limit=3 \
  -Werror \
  -Wall \
  -Wextra \
  -Wpedantic \
  -Weverything

CHECKS_OFF= \
  -Wno-gnu-binary-literal
  # -fno-strict-aliasing \
  # -Wno-nested-externs \
  # -fno-stack-protector


LIBS= \
  -lm \

SPEC = \
  -fopenmp

OPTIONS= -O2 -std=$(STD) $(CHECKS_ON) $(CHECKS_OFF) $(SPEC) $(LIBS)

run: build
	./$(OUT_FILE)

build:
	$(CC) $(OPTIONS) $(FILE) -o $(OUT_FILE)

check: 
	clang-check-10 $(FILE) -- $(OPTIONS) -x c 

clear:
	$(RM) $(OUT_FILE)
