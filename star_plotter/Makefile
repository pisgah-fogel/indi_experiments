all: app fitsviewer

app: main.cpp StarDetector.hpp fits.hpp
	g++ main.cpp -o app `pkg-config --cflags --libs opencv4 cfitsio`

fitsviewer: fitsviewer.cpp
	g++ fitsviewer.cpp -o fitsviewer `pkg-config --cflags --libs opencv4 cfitsio`

clean:
	rm -r app fitsviewer
