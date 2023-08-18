TARGET = scrapper

FLAGS = -Wall -Wextra -Werror

all: $(TARGET)

scrapper:
	gcc $(FLAGS) *.c -o ${TARGET}