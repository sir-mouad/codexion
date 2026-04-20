NAME	= codexion
CC		= cc
CFLAGS	= -Wall -Wextra -Werror
SRCS	= codexion.c utils1.c coder.c monitor.c parce.c \
			heap.c init.c utils2.c
OBJS	= $(SRCS:.c=.o)

.: all clean
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