#ifndef GRAPHICS_H
# define GRAPHICS_H

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <fcntl.h>
# include <unistd.h>

typedef struct	SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR	capabilities;
	VkSurfaceFormatKHR		*formats;
	int				nb_formats;
	VkPresentModeKHR		*presentModes;
	int				nb_present_modes;
}		SwapChainSupportDetails;

typedef struct	s_queue_family_indices
{
	uint32_t	graphics_family;
	short		graphics_family_has_value;
	uint32_t	present_family;
	short		present_family_has_value;
	short		is_complete;
}		t_queue_family_indices;

typedef struct	s_graphics_interface
{
	GLFWwindow		*window;
	VkInstance		instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice	physical_device;
	VkDevice		device;
	const char		**validations_layers;
	const char		**device_extensions;
	VkQueue			graphics_queue;
	VkQueue			present_queue;
	VkSurfaceKHR		surface;
	VkSwapchainKHR		swapChain;
	uint32_t		nb_swap_chain_images;
	VkImage			*swap_chain_images;
	VkImageView		*swap_chain_image_views;
	VkFormat		swap_chain_image_format;
	VkExtent2D		swap_chain_extent;
	VkRenderPass		render_pass;
	VkPipelineLayout	pipeline_layout;
	VkPipeline		graphics_pipeline;
}		t_graphics_interface;

//singleton who store params
t_graphics_interface	*p_gfx(t_graphics_interface*);

void			create_graphics_interface();
void			destroy_graphics_interface();
void			init_window();

#endif
