CFLAGS=-O3
LDFLAGS=-lpthread -ldl -lrt


default: bin/sem_mutex bin/sem_workers

bin/%: %.c
	@mkdir -p bin
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $<


