redis_test.bin: main.c make_log.c redis_op.c
	gcc main.c make_log.c redis_op.c -I/usr/local/include/hiredis -lhiredis  -L/usr/local/lib -o redis_test.bin

clean:
	rm -f *.o *.bin

