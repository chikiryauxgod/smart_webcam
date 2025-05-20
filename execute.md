# smart_webcam

# install grpc by the github.com official repo

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

# launch 

./main_video_server      
python3 server.py 50052    
./main_telegram_bot   
