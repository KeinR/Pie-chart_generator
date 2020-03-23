OBJS = main.o stb_image_write.o stb_truetype.o
CFLAGS = -Wall
CC = g++
INCLUDE = -Iextern

CCA := $(CC) $(CFLAGS) $(INCLUDE)

chartgen: $(OBJS)
	$(CCA) -o $@ $(OBJS)

run: chartgen config.txt
	./chartgen.exe config.txt

# depens
stb_%.o: extern/stb/stb_%.c extern/stb/stb_%.h
	$(CCA) -c $< -o $@
%.o: src/%.cpp src/%.h
	$(CCA) -c $< -o $@

clean-temp:
	-rm -f *.o

clean:
	-rm -f *.o *.exe
