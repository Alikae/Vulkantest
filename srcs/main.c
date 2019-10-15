#include "genekill.h"

void	parse_options(int ac, char **av, t_game *game)
{
	if (ac > 1 && av[1][0] == '-' && av[1][1] == 'd')
		game->debug_fd = 2;
}

void	exit_game(t_game *game)
{
}

void	init_game(t_game *game)
{
	game->debug_fd = 0;
	game->running = 1;
}

int main(int ac, char **av)
{
	t_game	game;

	init_game(&game);
	parse_options(ac, av, &game);
	launch_game(&game);
	exit_game(&game);
	return (0);
}
