if [ ! -d build/indi ]
then
    echo "No build directory found - Downloading INDI from github..."
    mkdir build
    git clone https://github.com/indilib/indi.git build/indi
    mkdir build/indi/examples/eqmount2
    echo "add_subdirectory(eqmount2)" >> build/indi/examples/CMakeLists.txt

    echo "Generating Makefiles (cmake)"
    mkdir -p build/indi/build/indi
    cd build/indi/build/indi
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ../..
    cd ../../../../
else
    echo "Local INDI dev version found."
fi

echo "Updating our files"
cp -r src/* build/indi/examples/eqmount2/

echo "Cleaning..."
rm -f build/indi/build/indi/examples/eqmount2/eqmount2

echo "Compiling INDI..."
cd build/indi/build/indi
make
cd ../../../../
#sudo make install
echo "Compilation finished"

if [ -x build/indi/build/indi/examples/eqmount2/eqmount2 ]
then
    echo "Successfully built"
    echo "Running server"
    indiserver -v build/indi/build/indi/examples/eqmount2/eqmount2
else
    echo "ERROR: Something went wrong eqmount2 have not been created"
fi

echo "done."