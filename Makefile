TARGET = scrapper

FLAGS = -Wall -Wextra -Werror

scrapper:
	gcc $(FLAGS) *.c -o ${TARGET}

all: $(TARGET)

clean:
	rm -rf $(TARGET)
