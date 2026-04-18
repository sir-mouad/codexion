NAME	= codexion
CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread -g
SRCS	= codexion.c utils.c coder.c monitor.c parce.c \
		 coder_utils.c heap.c heap_utils.c init.c
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