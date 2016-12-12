targets= fakelogin
TOCLEAN += $(targets)

fakelogin_objs = login.o
TOCLEAN += $(fakelogin_objs)

.PHONY: all clean

all: $(targets)

clean:
	$(RM) $(TOCLEAN)

fakelogin: $(fakelogin_objs)
	$(CC) $(LDFLAGS) -o $@ $(fakelogin_objs)
