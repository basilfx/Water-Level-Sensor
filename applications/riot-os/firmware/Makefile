# name of your application
APPLICATION = water-sensor-test

# If no BOARD is found in the environment, use this default:
BOARD ?= slstk3401a

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

FEATURES_REQUIRED += periph_gpio periph_spi

USEMODULE += periph_i2c
USEMODULE += shell
USEMODULE += xtimer

USEPKG += u8g2

include $(RIOTBASE)/Makefile.include
