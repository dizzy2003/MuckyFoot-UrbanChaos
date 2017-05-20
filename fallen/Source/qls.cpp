//
// Quick load and save from within a level.
//

#include "game.h"


//
// The filename we use as our savegame.
//

#define QLS_FNAME "data\\quicksave.dat"




void QLS_init()
{
	//
	// Get rid of the file.
	//

	FileDelete(QLS_FNAME);
}



void QLS_available()
{
	FILE *handle = MF_Fopen(QLS_FNAME, "rb");

	if (handle)
	{
		MF_Fclose(handle);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




void QLS_save()
{
	FILE *handle = MF_Fopen(QLS_FNAME);

	if (!handle)
	{
		return;
	}

	//
	// Save out the version number.
	//

	SLONG version = 1;

	if (fwrite(&version, sizeof(SLONG), 1, handle) != 1) goto file_error;

	//
	// Save out the the_game structure.
	//

	if (fwrite(&the_game, sizeof(the_game), 1, handle) != 1) goto file_error;

	





	MF_Fclose(handle);

	return;

  file_error:;

	MF_Fclose(handle);

	return;
}









SLONG QLS_load()
{
}





#endif
