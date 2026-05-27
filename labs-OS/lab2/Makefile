# makefile for scheduling program
#
# make rr - for round-robin scheduling
# make fcfs - for FCFS scheduling
# make sjf - for SJF scheduling
# make priority - for priority scheduling
# make priority_rr - for priority with round robin scheduling
# make scheduler - for interactive menu

CC=gcc
CFLAGS=-Wall

# Очистка всех скомпилированных файлов
clean:
	rm -rf *.o
	rm -rf fcfs
	rm -rf sjf
	rm -rf rr
	rm -rf priority
	rm -rf priority_rr
	rm -rf scheduler

# Интерактивное меню (НОВИНКА!)
scheduler: main_menu.o list.o CPU.o
	$(CC) $(CFLAGS) -o scheduler main_menu.o list.o CPU.o

main_menu.o: main_menu.c
	$(CC) $(CFLAGS) -c main_menu.c

# Round Robin
rr: driver.o list.o CPU.o schedule_rr.o
	$(CC) $(CFLAGS) -o rr driver.o schedule_rr.o list.o CPU.o

# Shortest Job First
sjf: driver.o list.o CPU.o schedule_sjf.o
	$(CC) $(CFLAGS) -o sjf driver.o schedule_sjf.o list.o CPU.o

# First Come First Served
fcfs: driver.o list.o CPU.o schedule_fcfs.o
	$(CC) $(CFLAGS) -o fcfs driver.o schedule_fcfs.o list.o CPU.o

# Priority Scheduling
priority: driver.o list.o CPU.o schedule_priority.o
	$(CC) $(CFLAGS) -o priority driver.o schedule_priority.o list.o CPU.o

# Priority with Round Robin
priority_rr: driver.o list.o CPU.o schedule_priority_rr.o
	$(CC) $(CFLAGS) -o priority_rr driver.o schedule_priority_rr.o list.o CPU.o

# Объектные файлы
schedule_fcfs.o: schedule_fcfs.c
	$(CC) $(CFLAGS) -c schedule_fcfs.c

schedule_sjf.o: schedule_sjf.c
	$(CC) $(CFLAGS) -c schedule_sjf.c

schedule_priority.o: schedule_priority.c
	$(CC) $(CFLAGS) -c schedule_priority.c

schedule_rr.o: schedule_rr.c
	$(CC) $(CFLAGS) -c schedule_rr.c

schedule_priority_rr.o: schedule_priority_rr.c
	$(CC) $(CFLAGS) -c schedule_priority_rr.c

driver.o: driver.c
	$(CC) $(CFLAGS) -c driver.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

CPU.o: CPU.c cpu.h
	$(CC) $(CFLAGS) -c CPU.c