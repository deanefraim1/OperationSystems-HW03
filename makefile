CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Werror -pedantic-errors
CXXLINK = $(CXX)
OBJS = main.o Session.o PecketStructs.o Address.o FileManager.o Helpers.o
TARGET = ttftps
RM = rm -f
# Creating the  executable
Bank: $(OBJS)
	$(CXXLINK) $(CXXFLAGS) -o Bank $(OBJS)
# Creating the object files
main.o: main.cpp
Session.o: Session.cpp Session.hpp PecketStructs.hpp Address.hpp FileManager.hpp Helpers.hpp
PecketStructs.o: PecketStructs.cpp PecketStructs.hpp
Address.o: Address.cpp Address.hpp Helpers.hpp
FileManager.o: FileManager.cpp FileManager.hpp Helpers.hpp
Helpers.o: Helpers.cpp Helpers.hpp PecketStructs.hpp Address.hpp Session.hpp
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

