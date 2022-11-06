.PHONY := clean
object = client server
all: $(object)

$(object): %: %.c
		gcc $< -o $@ -pthread


.PHONY := clean
clean:
		rm -rf $(object)

