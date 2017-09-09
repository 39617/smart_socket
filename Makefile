all: smart-socket

CONTIKI=contiki_multiple_interface

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
# Debug - TODO: retirar
CFLAGS += -g

# Errors
PROJECT_SOURCEFILES += ./error_codes.c

# automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
ifndef TARGET
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
ifeq ($(TARGET), native)
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c' ! -name 'res-plugtest*'))
endif
endif

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPS += er-coap
APPS += rest-engine
APPS += netctrl


CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL=0

# 0 - No Optimisation: -O0
# 1 - Optimisation:    -Os
# x - Optimisation:    -O2
SMALL=0


include $(CONTIKI)/Makefile.include