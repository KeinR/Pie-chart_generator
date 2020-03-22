OBJS = main.o stb_image_write.o stb_truetype.o
CFLAGS = -Wall -Wno-sign-compare
CC = g++
INCLUDE = -Iextern

CCA := $(CC) $(CFLAGS) $(INCLUDE)

chartgen.exe: $(OBJS)
	$(CCA) -o $@ $(OBJS)

# depens
stb_%.o: extern/stb/stb_%.c extern/stb/stb_%.h
	$(CCA) -c $< -o $@
%.o: src/%.cpp src/%.h 
	$(CCA) -c $< -o $@

clean:
	-rm -f *.o *.exe
