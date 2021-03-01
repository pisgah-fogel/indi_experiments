all: app

app: main.cpp
	g++ main.cpp -o app `pkg-config --cflags --libs opencv4 cfitsio`

clean:
	rm -r app
