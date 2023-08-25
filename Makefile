CXX   := g++
MKDIR := mkdir
RM    := rm
BINDIR   := bin
BUILDDIR := build

CFLAGS  += -Isrc/server -std=c++2a
LDFLAGS += -lpthread

all: $(BUILDDIR) $(BINDIR) $(BINDIR)/server$(EXT)

$(BINDIR)/server$(EXT): $(addprefix $(BUILDDIR)/server_,main.o HttpServer.o HttpRequest.o HttpResponse.o) $(addprefix $(BUILDDIR)/util_,PNG.o)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BUILDDIR):
	$(MKDIR) $@

$(BINDIR):
	$(MKDIR) $@

$(BUILDDIR)/server_%.o: src/server/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

$(BUILDDIR)/util_%.o: src/util/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

clean:
	$(RM) -r $(BUILDDIR) $(BINDIR)

.PHONY: clean
