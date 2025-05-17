# to the end of ~/.bashrc 
export PATH=$HOME/.local/bin:$PATH
export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$HOME/.local

# the next step

source ~/.bashrc
cd ~/prog/smart_webcam/
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_STANDARD=17
make -j$(nproc)


