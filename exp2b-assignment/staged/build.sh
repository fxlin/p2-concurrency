# compile
 gcc -o hashtable-biglock hashtable-biglock.c ../exp2/common.c ../exp2/measure.c \
  -I../exp2/ `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lpthread

