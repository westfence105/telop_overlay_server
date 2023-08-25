CXX   := g++
MKDIR := mkdir
RM    := rm
BINDIR   := bin
BUILDDIR := build

CFLAGS  += -Isrc/server -std=c++2a
LDFLAGS += -lpthread

all: $(BUILDDIR) $(BINDIR) $(BINDIR)/server$(EXT)

$(BINDIR)/server$(EXT): $(addprefix $(BUILDDIR)/,main.o HttpServer.o HttpRequest.o HttpResponse.o)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BUILDDIR):
	$(MKDIR) $@

$(BINDIR):
	$(MKDIR) $@

$(BUILDDIR)/%.o: src/server/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

clean:
	$(RM) -r $(BUILDDIR) $(BINDIR)

.PHONY: clean
