XERCESCROOT  := /usr/local/include/xercesc

CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		readXMLData.o

LIBS = -L$(XERCESCROOT)/lib -lxerces-c 

TARGET =	readXMLData

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
