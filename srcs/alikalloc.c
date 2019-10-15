typedef struct	s_alikaloc_data
{
	void		*block;
	uint32_t	block_size;
	void		**zone_refs;//references on block
	uint32_t	*zone_refs_size;//size of each correspondant reference
	uint32_t	zref_size;//size of both previous
	uint32_t	zref_occupation;//quantity of references stored (begin at 0 for 1 ref)
}		t_alikaloc_data;

void	add_zone_ref(t_alikaloc_data *data, void *address, int size)
{
//append zone ref to the end of occupied array
	data->zone_refs[++data->zref_occupation] = adress;
	data->zone_refs_size[data->zref_occupation] = size;
	if (data->zref_occupation == data->zref_size)
		//realloc zone ref && zone ref size
}

void	del_zone_ref(t_alikaloc_data *data, void *adress)
{
//replace the removed zone ref by the last occupied
	int	i = -1;
	while (data->zone_refs[++i] != adress && i < data->zref_occupation)
		;
	if (data->zone_refs[i] != adress)
		exit(write(2, "FREEkae FAIL\n", 10));
	data->zone_refs[i] = data->zone_refs[data->zref_occupation];
	data->zone_refs_size[i] = data->zone_refs_size[data->zref_occupation--];
}

void	init_data(t_alikaloc_data *data)
{
	data->block_size = 200000;
	data->zref_size = 2000;
	if (!((data->blocks = malloc(data->block_size))
		&& (data->zone_refs = malloc(data->zref_size * sizeof(void*)))
		&& (data->zone_refs = malloc(data->zref_size * sizeof(uint32_t)))))
		exit(write(2, "MALLOC FAIL\n", 12));
	data->zref_occupation = -1;
}

t_alikaloc_data	*get_datalloc(void)
{
	static t_alikaloc_data	data = {};

	if (!data.big_blocks)
		init_data(&data);
	return (&data);
}

void	*alikaloc(int size)
{
	t_alikaloc_data	*data = get_datalloc();
	void		*zone;

	while (!(zone = find_empty_block(data, size)))
		//realloc block
	add_zone_ref(data, zone, size);
	return (zone);
}

void	alikafree(void *mem)
{
	if (mem)
		del_zone_ref(get_datalloc(), mem);
}

int main()
{
	
}
