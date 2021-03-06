CC=gcc
AR=ar
CFLAGS=-c -Wall -g
LDFLAGS= -lpthread  -lrt -fPIC
SOURCES=list.c group.c task.c coordinator.c  accelerator.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=librtsrel.a

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR)  rcs $(EXECUTABLE) $(OBJECTS)

.c.o:
	$(CC) $(LDFLAGS) $(CFLAGS) $(FLAGS)  $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
	
help:
	echo "make -FLAGS=\"-DDEPENDENCIES\" to compile with dependencies or if you want them disable just write make"