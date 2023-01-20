CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Werror -pedantic-errors
CXXLINK = $(CXX)
OBJS = main.o Session.o PacketStructs.o Address.o FileManager.o Helpers.o
TARGET = ttftps
RM = rm -f
# Creating the  executable
ttftps: $(OBJS)
	$(CXXLINK) $(CXXFLAGS) -o ttftps $(OBJS)
# Creating the object files
main.o: main.cpp
Session.o: Session.cpp Session.hpp PacketStructs.hpp Address.hpp FileManager.hpp Helpers.hpp
PacketStructs.o: PacketStructs.cpp PacketStructs.hpp
Address.o: Address.cpp Address.hpp Helpers.hpp
FileManager.o: FileManager.cpp FileManager.hpp Helpers.hpp
Helpers.o: Helpers.cpp Helpers.hpp PacketStructs.hpp Address.hpp Session.hpp
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*