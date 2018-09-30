BASEDIR	:= $(dir $(firstword $(MAKEFILE_LIST)))
VPATH	:= $(BASEDIR)

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
# ROMFS is a folder to generate app's romfs
#---------------------------------------------------------------------------------
TARGET		:=	100_Boxes
SOURCES		:=	source
INCLUDES	:=	
ROMFS		:=	romfs

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS		+=	$(INCLUDES)
CXXFLAGS	+=	$(INCLUDES)
LDFLAGS		+=	$(WUT_NEWLIB_LDFLAGS) $(WUT_STDCPP_LDFLAGS) \
				-lSDL2_ttf -lfreetype -lpng -lSDL2_gfx -lSDL2_image -lSDL2 -ljpeg -lzip \
				-lcoreinit -lvpad -lsndcore2 -lsysapp -lproc_ui -lgx2 -lgfd -lzlib125 -lwhb

#---------------------------------------------------------------------------------
# get a list of objects
#---------------------------------------------------------------------------------
include $(DEVKITPRO)/portlibs/ppc/share/romfs-wiiu.mk
CFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
SFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))
OBJECTS		:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o) $(ROMFS_TARGET)

#---------------------------------------------------------------------------------
# objectives
#---------------------------------------------------------------------------------
$(TARGET).rpx: $(OBJECTS)

clean:
	rm -rf $(TARGET).rpx $(OBJECTS) $(OBJECTS:.o=.d)

#---------------------------------------------------------------------------------
# wut
#---------------------------------------------------------------------------------
include $(WUT_ROOT)/share/wut.mk

#---------------------------------------------------------------------------------
# portlibs
#---------------------------------------------------------------------------------
PORTLIBS	:=	$(DEVKITPRO)/portlibs/ppc
LDFLAGS		+=	-L$(PORTLIBS)/lib
CFLAGS		+=	-I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2
CXXFLAGS	+=	-I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2