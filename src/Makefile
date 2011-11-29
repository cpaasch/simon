CC=gcc
CFLAGS=`pkg-config --cflags --libs gtk+-2.0 libgtop-2.0 gobject-2.0`
LDFLAGS=`pkg-config --cflags --libs gtk+-2.0 libgtop-2.0 gobject-2.0`

SOURCES=gui.c main.c network.c utils.c utils.h gui.h network.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=simon

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
