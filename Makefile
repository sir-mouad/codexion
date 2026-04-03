NAME	= codexion
CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread
SRCDIR	= coders
SRCS	= $(SRCDIR)/main.c $(SRCDIR)/utils.c \
		  $(SRCDIR)/coder.c $(SRCDIR)/monitor.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
