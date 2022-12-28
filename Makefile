all:	client	manager

client:	file_client.c
	gcc	file_client.c	-o	client	-lreadline

manager:	file_manager.c
	gcc file_manager.c	-o	manager -lm