VULKAN_SDK_PATH =       /home/god/prog/libs/sdk.1.1.121.0/x86_64

LDFLAGS         =       -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
CFLAGS		=	-I$(VULKAN_SDK_PATH)/include

SRCS	=	srcs/main.c \
		srcs/launch_game.c \
		srcs/graphics_engine.c \
		srcs/graphics.c \
		srcs/vector.c \

all:
	gcc  $(CFLAGS) $(SRCS) -I. $(LDFLAGS)
