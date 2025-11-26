compile:
	g++ *.cpp -o main.exe

run:
	./main.exe

do:
	g++ *.cpp -o main.exe; ./main.exe

asan:
	g++ -g -fsanitize=address *.cpp -o main.exe

asando:
	g++ -g -fsanitize=address *.cpp -o main.exe; ./main.exe

gdb:
	g++ -g *.cpp -o main.exe

gdbdo:
	g++ -g *.cpp -o main.exe; gdb main.exe

main:
	cp ~/makefiles/main.cpp .; emacs-gtk+x11 main.cpp &

