include .env
export

compile: src/prj1.c src/config/config.c src/peers/peers.c src/utils/utils.c src/logger/logger.c
	gcc -o bin/prj1 src/prj1.c src/config/config.c src/peers/peers.c src/utils/utils.c src/logger/logger.c -I.

dev: compile
	bin/prj1

execute: bin/prj1
	bin/prj1
