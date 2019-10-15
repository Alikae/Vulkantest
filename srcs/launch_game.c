#include "genekill.h"
#include "graphics_engine.h"

static void	init_engines()
{
	//
	init_graphics_engine();
}

static void	destroy_engines()
{
	//
	destroy_graphics_engine();
}

void	launch_game(t_game *game)
{
	//init ALL
	init_engines();
	//load DATA
	while (game->running && !glfwWindowShouldClose(p_gfx(0)->window)/*refactor of course*/)
	{
		//collect_input(game);
		glfwPollEvents();
		//update_world(game);
		//update_sounds(game);
		//draw_world(game);
	}
}
