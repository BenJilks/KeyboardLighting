
all: lighting image text

bin:
	mkdir bin

lighting: bin
	g++ -std=c++20 keyboard_lighting.cpp Keyboard.cpp utils.cpp -o bin/keyboard_lighting -g -Dlibusb -lusb-1.0

image: bin
	g++ -std=c++20 keyboard_image.cpp Keyboard.cpp -o bin/keyboard_image -g -Dlibusb -lusb-1.0

text: bin
	g++ -std=c++20 keyboard_text.cpp Keyboard.cpp -o bin/keyboard_text -g -Dlibusb -lusb-1.0

