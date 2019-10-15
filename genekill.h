#ifndef GENEKILL_H
# define GENEKILL_H

typedef struct	s_game
{
	int	debug_fd;
	int	running;
}		t_game;

void	launch_game(t_game*);

#endif
