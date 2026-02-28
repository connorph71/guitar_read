CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lfftw3f -lm

TARGET = micRead
SRC = micRead.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)
