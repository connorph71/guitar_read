CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lfftw3f -lm

TARGET = mic_main
SRC = mic_main.c rms.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)
