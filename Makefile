CC = gcc
CFLAGS = -Wall -g -Iinclude -Wpedantic -Wextra -Wno-unused-variable -Wno-unused-parameter #estes 2 ultimos ignoram variaveis e paramentros que nunca forem usados
LDFLAGS =
INCLUDE_PATH = /include

all: folders server client
server: bin/orchestrator
client: bin/client
folders:
	@mkdir -p src include obj bin tmp output_folder
bin/orchestrator: obj/orchestrator.o obj/utils.o obj/parser.o
	$(CC) -I$(INCLUDE_PATH) $(LDFLAGS) $^ -o $@
bin/client: obj/client.o obj/utils.o
	$(CC) -I$(INCLUDE_PATH) $(LDFLAGS) $^ -o $@
obj/%.o: src/%.c
	$(CC) -I$(INCLUDE_PATH) $(CFLAGS) -c $< -o $@
clean:
	rm -f obj/* tmp/* bin/* output_folder/*