// cutscene.h
// matthew rosenfeld 24 may 1999


struct CSData;
struct CSChannel;
struct CSPacket;

void	do_cutscene_setup(EventPoint *the_ep);

void	CUTSCENE_chan_free(CSChannel& chan);
CSData* CUTSCENE_data_alloc();
void	CUTSCENE_data_free(CSData* data);
void	CUTSCENE_write(FILE *file_handle, CSData* data);
void	CUTSCENE_read(FILE *file_handle, CSData** dataref);


