!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.$(TARGET_SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)wld$(SEP)$(CONFIG)
CPU				= 386

OPTIONS			= -bt=$(WATCOM_BT) -q -zp4 -za99

WLD2ZON_EXE		= wld2zon$(EXE_SUFFIX)
WLDDBG_EXE		= wlddbg$(EXE_SUFFIX)

!ifeqi CONFIG debug
OPTIONS			+= -d2
!endif

# !inject 1 NO_JPEG NO_JUDAS NO_SCITECH NO_TOPFLC
# INC_NOTARGETS=1
# !include $(ROOT)$(SEP)3rdparty.mif
# !undef INC_NOTARGETS

# LIBS = $(ZLIB_LIB)
# OPTIONS += $(INCLUDE)

SOURCES_WLD2ZON = wld2zon.c vpe.c wld.c
SOURCES_WLDDBG = wlddbg.c vpe.c wld.c

OBJS_WLD2ZON = $(SOURCES_WLD2ZON:.c=.obj)
OBJS_WLDDBG = $(SOURCES_WLDDBG:.c=.obj)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)wld mkdir $(OUTDIR_BASE)$(SEP)wld
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: $(WLD2ZON_EXE) $(WLDDBG_EXE) .SYMBOLIC
	@%null

$(WLD2ZON_EXE): $(OUTDIR)$(SEP)$(WLD2ZON_EXE) .SYMBOLIC
	@%null

$(WLDDBG_EXE): $(OUTDIR)$(SEP)$(WLDDBG_EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(WLD2ZON_EXE): $(OBJS_WLD2ZON)

$(OUTDIR)$(SEP)$(WLDDBG_EXE): $(OBJS_WLDDBG)

$(OUTDIR)$(SEP)$(WLD2ZON_EXE) $(OUTDIR)$(SEP)$(WLDDBG_EXE):
	*wlink &
		$(WLINK_TARGET_SYS) &
		option quiet &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		option map=$^* &
#		path $(OUTDIR) &
		file { $< }
#		libfile { $(LIBS) }

.obj: $(OUTDIR)
.c: .

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(WLD2ZON_EXE) $(DELETE) $(OUTDIR)$(SEP)$(WLD2ZON_EXE)
	@if exist $(OUTDIR)$(SEP)$(WLDDBG_EXE) $(DELETE) $(OUTDIR)$(SEP)$(WLDDBG_EXE)

# INC_NOVARS=1
# !include $(ROOT)$(SEP)3rdparty.mif
# !undef INC_NOVARS
