TARGET = scrapper

FLAGS = -Wall -Wextra -Werror -lpthread

scrapper:
	gcc $(FLAGS) *.c -o ${TARGET}

all: $(TARGET)

force: clean all

clean:
	rm -rf $(TARGET)
