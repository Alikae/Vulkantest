//What i need
//
//init_graphic_engine
//clear_frame_buffer
//draw_image(buffer, image, position/tilt)
//render_to_screen
//destroy_graphic_engine

#include "graphics.h"
#include "vector.h"
#include "assert.h"

#ifdef NDEBUG
const int enableValidationLayers = 0;
#else
const int enableValidationLayers = 1;
#endif

t_graphics_interface	*p_gfx(t_graphics_interface *inter)
{
	//singleton who stock graphics_interface params
	static t_graphics_interface	*interface = 0;

	if (!interface)
		interface = inter;
	return (interface);
}

void	init_window(GLFWwindow **p_window)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	*p_window = glfwCreateWindow(800, 600/**/, "Genekill", 0, 0);
}

int	checkValidationLayerSupport(const char **validationLayers)
{
	uint32_t	layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);
	VkLayerProperties	*availableLayers = (VkLayerProperties*)create_vector(sizeof(VkLayerProperties), layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
	int	i = -1;
	int	j;
	int	layerFound;
	while (validationLayers[++i])
	{
		layerFound = 0;
		j = -1;
		while(++j < layerCount)
			if (!strcmp(validationLayers[i], availableLayers[j].layerName))
			{
				layerFound = 1;
				break;
			}
		if (!layerFound)
			return (0);
	}
	return (1);
}

VKAPI_ATTR uint32_t VKAPI_CALL	debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
	printf("validation layer: %s\n",  pCallbackData->pMessage);
	return VK_FALSE;
}


void	create_vulkan_instance()
{
	if (enableValidationLayers)
		printf("debug mode\n");
	p_gfx(0)->validations_layers = (const char**)create_vector(sizeof(const char*) , 2);
	p_gfx(0)->validations_layers[0] = "VK_LAYER_KHRONOS_validation";
	p_gfx(0)->validations_layers[1] = 0;
	if (enableValidationLayers && !checkValidationLayerSupport(p_gfx(0)->validations_layers))
		exit(printf("validation layers requested, but not available!\n"));

	VkApplicationInfo	appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Genekill";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Lasagna Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo	createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = (uint32_t)(1/*VALIDATIONLAYER SIZE*/);
		createInfo.ppEnabledLayerNames = p_gfx(0)->validations_layers;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	const char** tmp;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (enableValidationLayers)
	{
	//push VKDBUG on the array
		tmp = (const char**)create_vector(sizeof(char*), glfwExtensionCount + 1);
		int i = -1;
		while (++i < glfwExtensionCount)
			tmp[i] = glfwExtensions[i];
		tmp[i++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		tmp[i] = 0;
		//free(glfwExtensions);
		glfwExtensions = tmp;
	}

	createInfo.enabledExtensionCount = glfwExtensionCount + 1/*updated size*/;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, 0, &(p_gfx(0)->instance)) != VK_SUCCESS)
		exit(printf("failed to create instance!"));

	uint32_t	extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);

	VkExtensionProperties	*extensions = (VkExtensionProperties*)create_vector(sizeof(VkExtensionProperties), extensionCount);
	printf("a\n");
	vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions);
	dprintf(2, "Available extensions:\n");
	int i = -1;
	while (++i < extensionCount)
		dprintf(2, "- %s\n", extensions[i].extensionName);
}

t_queue_family_indices	find_queue_families(VkPhysicalDevice device)
{
	t_queue_family_indices	indices;

	indices.graphics_family_has_value = 0;
	indices.present_family_has_value = 0;
	indices.is_complete = 0;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

	VkQueueFamilyProperties	*queue_families = create_vector(sizeof(VkQueueFamilyProperties), queueFamilyCount);
	printf("b\n");
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queue_families);
	int	j = -1;
	unsigned int	presentSupport;
	while (++j < queueFamilyCount)
	{
		if (queue_families[j].queueCount > 0 && queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_family = j;
			indices.graphics_family_has_value = 1;
		}
		presentSupport = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, j, p_gfx(0)->surface, &presentSupport);
		if (queue_families[j].queueCount > 0 && presentSupport)
		{
			indices.present_family = j;
			indices.present_family_has_value = 1;
		}
		if ((indices.is_complete = (indices.graphics_family_has_value && indices.present_family_has_value)))
			break;
	}
	return (indices);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, int size)
{
	int	i = -1;
	while (++i < size)
	{
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return (availableFormats[i]);
	}
	return (availableFormats[0]);
}

VkPresentModeKHR	chooseSwapPresentMode(const VkPresentModeKHR	*availablePresentModes, int size)
{
	int	i = -1;
	while (++i < size)
	{
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return (availablePresentModes[i]);
	}
	return (VK_PRESENT_MODE_FIFO_KHR);
}

uint32_t	max(int a, int b)
{
	return ((b > a) ? b : a);
}

uint32_t	min(int a, int b)
{
	return ((a > b) ? b : a);
}

VkExtent2D	chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities)
{
	if (capabilities->currentExtent.width != UINT32_MAX)
	{
		return (capabilities->currentExtent);
	}
	else
	{
		VkExtent2D	actualExtent = {800, 600/*UPDATE TO SCREENSIZE*/};
		actualExtent.width = max(capabilities->minImageExtent.width, min(capabilities->maxImageExtent.width, actualExtent.width));
		actualExtent.height = max(capabilities->minImageExtent.height, min(capabilities->maxImageExtent.height, actualExtent.height));

		return (actualExtent);
	}
}

unsigned int	checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, 0);
	VkExtensionProperties	*available_extensions = (VkExtensionProperties*)create_vector(sizeof(VkExtensionProperties), extensionCount);
	printf("c\n");
	vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, available_extensions);
	int	i = -1;
	int	j = 0;
	while (++i < extensionCount)
	{
		if (!strcmp(available_extensions[i].extensionName, p_gfx(0)->device_extensions[0]))//To update if more extensions
			j++;
	}
	return (j == 1/*nb device extensions*/);
}

SwapChainSupportDetails	querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, p_gfx(0)->surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, p_gfx(0)->surface, &formatCount, 0);
	if (formatCount != 0)
	{
		details.formats = (VkSurfaceFormatKHR*)create_vector(sizeof(VkSurfaceFormatKHR), formatCount); //to free...
		printf("c\n");
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, p_gfx(0)->surface, &formatCount, details.formats);
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, p_gfx(0)->surface, &presentModeCount, 0);
	if (presentModeCount != 0)
	{
		details.presentModes = (VkPresentModeKHR*)create_vector(sizeof(VkPresentModeKHR), presentModeCount); //to free...
		printf("d\n");
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, p_gfx(0)->surface, &presentModeCount, details.presentModes);
	}
	details.nb_present_modes = presentModeCount;
	details.nb_formats = formatCount;
	return (details);
}

int	is_device_suitable(VkPhysicalDevice device)
{
	//check for properties, features of device
	//check for existence of queue families
	t_queue_family_indices indices = find_queue_families(device);
	unsigned int	extensionsSupported = checkDeviceExtensionSupport(device);
	unsigned int	swapChainAdequate = 0;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = (swapChainSupport.nb_formats && swapChainSupport.nb_present_modes);
	}

	return (indices.is_complete && extensionsSupported && swapChainAdequate);
}

void	pick_physical_device()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(p_gfx(0)->instance, &deviceCount, 0);
	if (!deviceCount)
		exit(printf("failed to find GPUs with Vulkan support!"));
	VkPhysicalDevice	*devices = (VkPhysicalDevice*)create_vector(sizeof(VkPhysicalDevice), deviceCount);
	vkEnumeratePhysicalDevices(p_gfx(0)->instance, &deviceCount, devices);
	int	i = -1;
	while (++i < deviceCount)
	{
		if (is_device_suitable(devices[i]))
		{
			p_gfx(0)->physical_device = devices[i];
			break;
		}
	}
	if (p_gfx(0)->physical_device == VK_NULL_HANDLE)
		exit(printf("failed to find a suitable GPU!"));
}

void	create_logical_device()
{
	//Can make multiple queues for multithreading
	t_queue_family_indices indices = find_queue_families(p_gfx(0)->physical_device);

	VkDeviceQueueCreateInfo	*queue_create_infos = create_vector(sizeof(VkDeviceQueueCreateInfo), 2/*NB OF QUEUES*/);
	uint32_t		unique_queue_families[2] = {indices.graphics_family, indices.present_family};
	float queuePriority = 1.0f;
	int n = -1;
	while (++n < 2/*NB QUEUES*/)
	{
		VkDeviceQueueCreateInfo	queue_create_info = {0};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = unique_queue_families[n];
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queuePriority;
		memcpy(queue_create_infos + n, &queue_create_info, sizeof(VkDeviceQueueCreateInfo));
	}

	VkPhysicalDeviceFeatures deviceFeatures = {0};
	VkDeviceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queue_create_infos;
	createInfo.queueCreateInfoCount = 2/*NBQ*/;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = (uint32_t)(1/*nb_extensions to update*/);
	createInfo.ppEnabledExtensionNames = p_gfx(0)->device_extensions;

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = (uint32_t)(1/*validationLayers.size()*/);
		createInfo.ppEnabledLayerNames = p_gfx(0)->validations_layers;
	}
	else
		createInfo.enabledLayerCount = 0;
	if (vkCreateDevice(p_gfx(0)->physical_device, &createInfo, 0, &p_gfx(0)->device) != VK_SUCCESS)
		exit(printf("failed to create logical device!"));
	vkGetDeviceQueue(p_gfx(0)->device, indices.graphics_family, 0, &p_gfx(0)->graphics_queue);
	vkGetDeviceQueue(p_gfx(0)->device, indices.present_family, 0, &p_gfx(0)->present_queue);
}

void	create_surface()
{
	if (glfwCreateWindowSurface(p_gfx(0)->instance, p_gfx(0)->window, 0, &p_gfx(0)->surface) != VK_SUCCESS) {
		exit(printf("failed to create window surface!"));
	}
}

void	create_swap_chain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(p_gfx(0)->physical_device);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.nb_formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.nb_present_modes);
	VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);
	uint32_t	imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = p_gfx(0)->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	t_queue_family_indices indices = find_queue_families(p_gfx(0)->physical_device);
	uint32_t	*queueFamilyIndices = (uint32_t*)create_vector(sizeof(uint32_t), 2);
	queueFamilyIndices[0] = indices.graphics_family;
	queueFamilyIndices[1] = indices.present_family;

	if (indices.graphics_family != indices.present_family)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = 0; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(p_gfx(0)->device, &createInfo, 0, &p_gfx(0)->swapChain) != VK_SUCCESS)
		exit(printf("failed to create swap chain!"));

	vkGetSwapchainImagesKHR(p_gfx(0)->device, p_gfx(0)->swapChain, &imageCount, 0);
	p_gfx(0)->swap_chain_images = (VkImage*)create_vector(sizeof(VkImage), imageCount);
	vkGetSwapchainImagesKHR(p_gfx(0)->device, p_gfx(0)->swapChain, &imageCount, p_gfx(0)->swap_chain_images);
	p_gfx(0)->nb_swap_chain_images = imageCount;
	p_gfx(0)->swap_chain_image_format = surfaceFormat.format;
	p_gfx(0)->swap_chain_extent = extent;

}

void	create_image_views()
{
	p_gfx(0)->swap_chain_image_views = (VkImageView*)create_vector(sizeof(VkImageView), p_gfx(0)->nb_swap_chain_images);
	int	i = -1;
	while (++i < p_gfx(0)->nb_swap_chain_images)
	{
		VkImageViewCreateInfo createInfo = {0};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = p_gfx(0)->swap_chain_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = p_gfx(0)->swap_chain_image_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(p_gfx(0)->device, &createInfo, 0, &p_gfx(0)->swap_chain_image_views[i]) != VK_SUCCESS)
			exit(printf("failed to create image views!"));
	}
}

char	*read_file(const char *file_name, int *len)
{
	int fd = open(file_name, O_RDONLY);
	*len = lseek(fd, 0, SEEK_END);
	char *buf = create_vector(sizeof(char), *len);
	read(fd, buf, *len);
	close(fd);
	printf("%i bytes read\n", *len);
	return (buf);
}

VkShaderModule createShaderModule(const char *code, int len)
{
	VkShaderModuleCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = len;
	createInfo.pCode = (const uint32_t*)code;
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(p_gfx(0)->device, &createInfo, 0, &shaderModule) != VK_SUCCESS)
		exit(printf("failed to create shader module!"));
	return shaderModule;
}

void lstp(){}

void	create_graphics_pipeline()
{
	int	vert_len;
	int	frag_len;
	char *vertShaderCode = read_file("shaders/vert.spv", &vert_len);
	char *fragShaderCode = read_file("shaders/frag.spv", &frag_len);
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, vert_len);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, frag_len);
	free(vertShaderCode);
	free(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	//VkPipelineShaderStageCreateInfo shaderStages[2/*to update*/] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineShaderStageCreateInfo	*shaderStages = create_vector(sizeof(VkPipelineShaderStageCreateInfo), 2/*to update*/);
	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = 0; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = 0;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)p_gfx(0)->swap_chain_extent.width;
	viewport.height = (float)p_gfx(0)->swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};//scissor is for crop image zone where we draw
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = p_gfx(0)->swap_chain_extent;

	VkPipelineViewportStateCreateInfo viewportState = {0};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	
	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = 0; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {0};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	//For recreate pipeline elems like viewport size on draw time
	/*VkDynamicState dynamicStates[] = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {0};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;*/
	//

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = 0; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

	if (vkCreatePipelineLayout(p_gfx(0)->device, &pipelineLayoutInfo, 0, &p_gfx(0)->pipeline_layout) != VK_SUCCESS)
		exit(printf("failed to create pipeline layout!"));

	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	assert(!memcmp(&pipelineInfo, (unsigned char[sizeof(VkGraphicsPipelineCreateInfo)]){0}, sizeof(VkGraphicsPipelineCreateInfo)));
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = 0; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = 0; // Optional
	pipelineInfo.layout = p_gfx(0)->pipeline_layout;
	pipelineInfo.renderPass = p_gfx(0)->render_pass;
	//pipelineInfo.renderPass = 0xe000000000e;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	printf("there\n");
	lstp();
	if (vkCreateGraphicsPipelines(p_gfx(0)->device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &p_gfx(0)->graphics_pipeline) != VK_SUCCESS)
		exit(printf("failed to create graphics pipeline!"));

	vkDestroyShaderModule(p_gfx(0)->device, fragShaderModule, 0);
	vkDestroyShaderModule(p_gfx(0)->device, vertShaderModule, 0);
}

void	create_render_pass()
{
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = p_gfx(0)->swap_chain_image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	lstp();

	if (vkCreateRenderPass(p_gfx(0)->device, &renderPassInfo, 0, &p_gfx(0)->render_pass) != VK_SUCCESS)
		exit(printf("failed to create render pass!"));
	printf("%llx\n", p_gfx(0)->render_pass);
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != 0)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void	setup_debug_messenger()
{
	if (!enableValidationLayers)
		return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = 0; // Optional
	if (CreateDebugUtilsMessengerEXT(p_gfx(0)->instance, &createInfo, 0, &p_gfx(0)->debug_messenger) != VK_SUCCESS)
		exit(printf("failed to set up debug messenger!"));
}

void	init_vulkan()
{
	create_vulkan_instance();
	setup_debug_messenger();
	create_surface();
	pick_physical_device();
	create_logical_device();
	create_swap_chain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
}

void	create_graphics_interface()
{
	t_graphics_interface	*interface;

	if (!(interface = (t_graphics_interface*)malloc(sizeof(t_graphics_interface))))
		exit(1);
	p_gfx(interface);
	interface->physical_device = VK_NULL_HANDLE;
	interface->device_extensions = (const char**)create_vector(sizeof(const char*), 1/*nb needed extensions*/);
	interface->device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	init_window(&interface->window);
	init_vulkan();
}

void	destroy_graphics_interface()
{
	vkDestroyPipeline(p_gfx(0)->device, p_gfx(0)->graphics_pipeline, 0);
	vkDestroyPipelineLayout(p_gfx(0)->device, p_gfx(0)->pipeline_layout, 0);
	vkDestroyRenderPass(p_gfx(0)->device, p_gfx(0)->render_pass, 0);
	int	i = -1;
	while (++i < p_gfx(0)->nb_swap_chain_images)
		vkDestroyImageView(p_gfx(0)->device, p_gfx(0)->swap_chain_image_views[i], 0);
	vkDestroySwapchainKHR(p_gfx(0)->device, p_gfx(0)->swapChain, 0);
	vkDestroyDevice(p_gfx(0)->device, 0);
	vkDestroySurfaceKHR(p_gfx(0)->instance, p_gfx(0)->surface, 0);
	vkDestroyInstance(p_gfx(0)->instance, 0);
	glfwDestroyWindow(p_gfx(0)->window);
	glfwTerminate();
	free(p_gfx(0));
}

void	render_to_screen()
{
	//
}
