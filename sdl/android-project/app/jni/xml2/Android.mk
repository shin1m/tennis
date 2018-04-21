LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LIBXML2_PATH := $(LOCAL_PATH)/../../../libxml2

LOCAL_SRC_FILES := \
	$(LIBXML2_PATH)/SAX.c \
	$(LIBXML2_PATH)/entities.c \
	$(LIBXML2_PATH)/encoding.c \
	$(LIBXML2_PATH)/error.c \
	$(LIBXML2_PATH)/parserInternals.c \
	$(LIBXML2_PATH)/parser.c \
	$(LIBXML2_PATH)/tree.c \
	$(LIBXML2_PATH)/hash.c \
	$(LIBXML2_PATH)/list.c \
	$(LIBXML2_PATH)/xmlIO.c \
	$(LIBXML2_PATH)/xmlmemory.c \
	$(LIBXML2_PATH)/uri.c \
	$(LIBXML2_PATH)/valid.c \
	$(LIBXML2_PATH)/xlink.c \
	$(LIBXML2_PATH)/HTMLparser.c \
	$(LIBXML2_PATH)/HTMLtree.c \
	$(LIBXML2_PATH)/debugXML.c \
	$(LIBXML2_PATH)/xpath.c \
	$(LIBXML2_PATH)/xpointer.c \
	$(LIBXML2_PATH)/xinclude.c \
	$(LIBXML2_PATH)/nanohttp.c \
	$(LIBXML2_PATH)/nanoftp.c \
	$(LIBXML2_PATH)/DOCBparser.c \
	$(LIBXML2_PATH)/catalog.c \
	$(LIBXML2_PATH)/globals.c \
	$(LIBXML2_PATH)/threads.c \
	$(LIBXML2_PATH)/c14n.c \
	$(LIBXML2_PATH)/xmlstring.c \
	$(LIBXML2_PATH)/xmlregexp.c \
	$(LIBXML2_PATH)/xmlschemas.c \
	$(LIBXML2_PATH)/xmlschemastypes.c \
	$(LIBXML2_PATH)/xmlunicode.c \
	$(LIBXML2_PATH)/xmlreader.c \
	$(LIBXML2_PATH)/relaxng.c \
	$(LIBXML2_PATH)/dict.c \
	$(LIBXML2_PATH)/SAX2.c \
	$(LIBXML2_PATH)/legacy.c \
	$(LIBXML2_PATH)/chvalid.c \
	$(LIBXML2_PATH)/pattern.c \
	$(LIBXML2_PATH)/xmlsave.c \
	$(LIBXML2_PATH)/xmlmodule.c \
	$(LIBXML2_PATH)/xmlwriter.c \
	$(LIBXML2_PATH)/schematron.c

LOCAL_C_INCLUDES += $(LIBXML2_PATH)/include

LOCAL_MODULE := xml2
include $(BUILD_SHARED_LIBRARY)
