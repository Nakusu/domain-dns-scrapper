TARGET = scrapper

FLAGS = 

all: $(TARGET)

scrapper:
	gcc $(FLAGS) *.c -o ${TARGET}