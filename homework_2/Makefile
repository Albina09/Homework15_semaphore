.PHONY := clean
object = client server
all: $(object)

$(object): %: %.c
		gcc $< -o $@ -pthread -g


.PHONY := clean
clean:
		rm -rf $(object)

