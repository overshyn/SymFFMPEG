# ============================================================================
#  Name	 : build_help.mk
#  Part of  : FFMPEGPlayer
# ============================================================================
#  Name	 : build_help.mk
#  Part of  : FFMPEGPlayer
#
#  Description: This make file will build the application help file (.hlp)
# 
# ============================================================================

do_nothing :
	@rem do_nothing

# build the help from the MAKMAKE step so the header file generated
# will be found by cpp.exe when calculating the dependency information
# in the mmp makefiles.

MAKMAKE : FFMPEGPlayer_0xE28ECF2D.hlp
FFMPEGPlayer_0xE28ECF2D.hlp : FFMPEGPlayer.xml FFMPEGPlayer.cshlp Custom.xml
	cshlpcmp FFMPEGPlayer.cshlp
ifeq (WINSCW,$(findstring WINSCW, $(PLATFORM)))
	md $(EPOCROOT)epoc32\$(PLATFORM)\c\resource\help
	copy FFMPEGPlayer_0xE28ECF2D.hlp $(EPOCROOT)epoc32\$(PLATFORM)\c\resource\help
endif

BLD : do_nothing

CLEAN :
	del FFMPEGPlayer_0xE28ECF2D.hlp
	del FFMPEGPlayer_0xE28ECF2D.hlp.hrh

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE : do_nothing
		
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo FFMPEGPlayer_0xE28ECF2D.hlp

FINAL : do_nothing
