CXX = g++
CXXFLAGS = -std=c++11 -Wall

ARCH := 'esp32:esp32:esp32'
PORT := '/dev/ttyUSB0'

arduino-compile:
	arduino-cli compile --fqbn $(ARCH) .

upload:
	arduino-cli upload -p $(PORT) --fqbn $(ARCH) .

