CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lfftw3f -lm

TARGET = mic_read
SRC = main.c rms.c mic_read.c

TARGET2 = server
SRC2 = tcp_server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET2)

run: $(TARGET)
	sudo ./$(TARGET)

run2: $(TARGET2)
	sudo ./$(TARGET2)
