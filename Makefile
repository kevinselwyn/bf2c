NAME   := bf2c
BINDIR := /usr/local/bin

all: $(NAME)

$(NAME): $(NAME).c
	gcc -o $@ $<

test: $(NAME)
	./$(NAME) test.bf

install: $(NAME)
	install -m 0755 $< $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(NAME)

clean:
	rm $(NAME)
