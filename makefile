include .env
export

compile: src/prj2.c src/config/config.c src/peers/peers.c src/utils/utils.c src/logger/logger.c src/token/token.c src/queue/queue.c
	gcc -o bin/prj2 src/prj2.c src/config/config.c src/peers/peers.c src/utils/utils.c src/logger/logger.c src/token/token.c src/queue/queue.c -I.

dev: compile
	bin/prj2

execute: bin/prj2
	bin/prj2
