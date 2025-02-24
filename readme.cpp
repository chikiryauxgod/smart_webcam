g++ main.cpp -o server -std=c++17 -Wall -O2 $(pkg-config --cflags --libs opencv4) -lboost_system -lssl -lcrypto -pthread
