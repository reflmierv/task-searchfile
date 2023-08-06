LIBS = -lpthread

main : src/searchfile.o libs/libsearchfile.a
	g++ -o searchfile $^ $(LIBS)

%.o : %.c
	g++ -c $< -o $@

libs/libsearchfile.a: src/libsearchfile.o
	mkdir libs
	ar -crs $@ $<

clean:
	rm libs/*
	rm src/*.o
	rm searchfile
