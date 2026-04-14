.NOTPARALLEL:

.SUFFIXES:
.SUFFIXES: .c .o .obj .a .lib .sym .dll .exe .def .imp

CC := gcc
CFLAGS :=

CXX := g++
CXXFLAGS :=

LDFLAGS :=

AR := ar
ARFLAGS := crv

KDLLAR := ../kdllar -cc $(CC) -noecho -nolxlite
KLD := ../kld -cc $(CC) -noecho -nolxlite

EMXIMP := emximp

MV := mv
RM := rm -f
CMP := diff -i
SORT := sort
GREP := grep

DLLPATH := ././././././././././
DLLNAMESHORT := dll_long
DLLNAMELONG := $(DLLPATH)$(DLLNAMESHORT)_name

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.imp: %.dll
	$(EMXIMP) -o $@ $<
	$(SORT) $@ > $@.sort
	$(MV) $@.sort $@
	$(CMP) $@ $@.ref

.PHONY: all

all: test-kdllar test-kld

.PHONY: test-kdllar

test-kdllar:
	$(MAKE) clean
	$(MAKE) dllexp.imp
	$(MAKE) dllsym.imp
	$(MAKE) $(DLLNAMELONG).imp

dllexp.dll: dll1.o dll2.a dll3.a
	$(KDLLAR) -noexport -o $@ dll1.o dll2.a dll3.a

dllsym.dll: dll2.a dll3.a dll.sym
	$(KDLLAR) -symprefix -symfile dll.sym -o $@ dll2.a dll3.a

$(DLLNAMELONG).imp: $(DLLNAMESHORT).dll
	$(EMXIMP) -o $@ $<
	$(SORT) $@ > $@.sort
	$(MV) $@.sort $@
	$(CMP) $@ $@.ref

$(DLLNAMESHORT).dll: dll2.a dll3.a
	$(KDLLAR) -o $(DLLNAMELONG).dll dll1.o dll2.a dll3.a

dll2.a: dll2.o
	$(AR) $(ARFLAGS) $@ $<

dll3.a: dll3.o
	$(AR) $(ARFLAGS) $@ $<

.PHONY: test-kld

test-kld: test-kld-aout test-kld-omf

.PHONY: test-kld-aout

test-kld-aout: test-kld-aout-dll test-kld-aout-exe

.PHONY: test-kld-aout-dll

test-kld-aout-dll:
	$(MAKE) clean
	$(MAKE) KDLLAR="$(KLD) -Zdll" dllexp.imp \
	  || echo "Expected failure of dllexp.imp! __declspec(dllexport) is not supported by ld."
	$(MAKE) KDLLAR="$(KLD) -Zdll" dllsym.imp
	$(MAKE) KDLLAR="$(KLD) -Zdll" $(DLLNAMELONG).imp

.PHONY: test-kld-aout-exe

test-kld-aout-exe:
	$(MAKE) clean
	$(MAKE) exe-aout.exe

EXE_AOUT_STR := This is emx-aout.exe
exe-aout.exe: exe.o
	$(KLD) -app WINDOWCOMPAT -d "$(EXE_AOUT_STR)" -o exe-aout.exe $<
	$(GREP) "NAME $(@:.exe=) WINDOWCOMPAT" $(@:.exe=.def)
	$(GREP) "$(EXE_AOUT_STR)" $@

.PHONY: test-kld-omf

test-kld-omf: test-kld-omf-dll test-kld-omf-exe

.PHONY: test-kld-omf-dll

test-kld-omf-dll:
	$(MAKE) clean
	$(MAKE) KDLLAR="$(KLD) -Zomf -Zdll" dllexp.imp
	$(MAKE) KDLLAR="$(KLD) -Zomf -Zdll" dllsym.imp
	$(MAKE) KDLLAR="$(KLD) -Zomf -Zdll" $(DLLNAMELONG).imp

.PHONY: test-kld-omf-exe

test-kld-omf-exe:
	$(MAKE) clean
	$(MAKE) exe-omf.exe

EXE_OMF_STR := This is emx-omf.exe

exe-omf.exe: exe.o
	$(KLD) -app WINDOWCOMPAT -d "$(EXE_OMF_STR)" -Zomf -o exe-omf.exe $<
	$(GREP) "NAME $(@:.exe=) WINDOWCOMPAT" $(@:.exe=.def)
	$(GREP) "$(EXE_OMF_STR)" $@

.PHONY: clean

clean:
	$(RM) *.o *.a *.lib *.dll *.exe *.def *.imp *.bak
