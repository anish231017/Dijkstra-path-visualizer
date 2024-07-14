all: compile link

compile:
	g++ -c main.cpp -I"C:\Users\anish\Downloads\SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit\SFML-2.6.1\include" -DSFML_STATIC

link: 
	g++ main.o -o main -L"C:\Users\anish\Downloads\SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit\SFML-2.6.1\lib" -lsfml-graphics -lsfml-window -lsfml-system

clean: 
	rm -f main *.o


