CC = gcc
CFLAGS = -Wall -g -IC:"C:/Program Files (x86)/mosquitto/devel"
LDFLAGS = -LC:"C:/Program Files (x86)/mosquitto/devel"
TARGET = main.exe
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) "C:/Program Files (x86)/mosquitto/devel/mosquitto.lib" -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: $(SOURCES)  # Explicit rule to compile directly if needed
	$(CC) $(CFLAGS) $< -o main.exe

clean:
	rm -f $(TARGET) $(OBJECTS) main.exe