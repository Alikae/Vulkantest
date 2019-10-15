#include "vector.h"

void	*create_vector(int size, int len)
{
	void	*vector;
	char	*byte;

	printf("size vec:%i\n", size);
	if (!(vector = malloc((len + 1) * size)))
		exit(1);
	byte = (char*)vector + size * (len + 1);
	while (--byte >= (char*)vector)//right_number?
		*byte = 0;
	return (vector);
}

void	delete_vector(void *vector)
{
	free(vector);
}
