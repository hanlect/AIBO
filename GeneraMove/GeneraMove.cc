#include "GeneraMove.h"
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <OPENR/OPENRAPI.h>

#include "entry.h"
#include "../Motion/MotionInterface.h"
#include <iostream>
#include <fstream>
#include <math.h>


//char* model="ERS7";
typedef unsigned char BYTE;

int grids_x = 4;
int grids_y = 4;

  /** Stati di Touched.*/
  enum
    {
      IDLE, /**< Non so facendo nulla.*/
      MOVING, /**< Mi sto muovendo.*/
    } mState;

GeneraMove::GeneraMove(){
	sph = 0;
	imageVec = NULL;
	fbkID = oprimitiveID_UNDEF;
}


/** Inizializzo le strutture di comunicazione inter-object e apro i
    sensori utilizzati*/
OStatus GeneraMove::DoInit(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoInit()\n"));

	NEW_ALL_SUBJECT_AND_OBSERVER;
	REGISTER_ALL_ENTRY;
	SET_ALL_READY_AND_NOTIFY_ENTRY;

	//
	// Apro la primitiva di gestione del sensore di tatto posteriore
	//
	OStatus result = OPENR::OpenPrimitive(BACKR7_LOCATOR , &mBackrID);
	if (result != oSUCCESS)
	{
		OSYSLOG1((osyslogERROR, "%s : %s %d",
				"GeneraMove::DoInit()",
				"OPENR::OpenPrimitive() FAILED", result));
	}

	//
	// Apro la primitiva di gestione della telecamera
	//
	result = OPENR::OpenPrimitive(FBK_LOCATOR, &fbkID);
	if (result != oSUCCESS)
	{
		OSYSLOG1((osyslogERROR, "%s : %s %d",
				"GeneraMove::DoInit()",
				"OPENR::OpenPrimitive() FAILED", result));
	}

	SetCameraParameter();

	SetCdtVectorData();

	return oSUCCESS;
}


/** Faccio partire le strutture di gestione della comunicazione
    inter-object.*/
OStatus GeneraMove::DoStart(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoStart()\n"));

	ENABLE_ALL_SUBJECT;
	ASSERT_READY_TO_ALL_OBSERVER;
	count=0;

	// Accendo i motori (per scrupolo)
	OPENR::SetMotorPower(opowerON);


	Motion::MotionCommand command;
	memset(&command, 0, sizeof(command));
	command.motion_cmd=Motion::MOTION_STAND_NEUTRAL;
	command.head_cmd=Motion::HEAD_LOOKAT;
	command.tail_cmd=Motion::TAIL_NO_CMD;
	command.head_lookat=vector3d(150,0,50);

	if (sph ==1){
	  subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
	  subject[sbjMotionControl]->NotifyObservers();

	  sph=0;
	}

	//Walk();

	return oSUCCESS;
} 


/** Fermo la comunicazione inter-object.*/
OStatus GeneraMove::DoStop(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoStop()\n"));

	DISABLE_ALL_SUBJECT;
	DEASSERT_READY_TO_ALL_OBSERVER;
	OPENR::SetMotorPower(opowerOFF);

	return oSUCCESS;
} 


/** Distrutto le strutture di comunicazione inter-object.*/
OStatus GeneraMove::DoDestroy(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoDestroy\()\n"));

	DELETE_ALL_SUBJECT_AND_OBSERVER;

	return oSUCCESS;
}  //DoDestroy() END




void GeneraMove::Walk()
{
	 	    Wait(static_cast<longword>(2000000000));
  	OSYSDEBUG(("Entrato in walk\n"));
	//int** grid_matrix = Grid(imageVec);
	int i = 0;
	while (i < 10)
	{
		int** grid_matrix = Grid(imageVec);
		
		// solo per testare momentaneamente
		if (i == 0){
		  grid_matrix[3][1] = 2;
		}
		else{
		  grid_matrix[3][1] = 4;
		} // fine test momentaneo, poi eliminare
		    
		if (grid_matrix[3][1] < 3)
		  {
		    Motion::MotionCommand command;
		    memset(&command, 0, sizeof(command));
		    command.motion_cmd=Motion::MOTION_WALK_TROT;
		    command.head_cmd=Motion::HEAD_LOOKAT;
		    command.tail_cmd=Motion::TAIL_NO_CMD;
		    command.head_lookat=vector3d(150,0,50);
		    command.vx=100;
		    command.vy=0;
		    command.va=0.5;
		    subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
		    subject[sbjMotionControl]->NotifyObservers();
		    Wait(static_cast<longword>(2000000000));
		  }
		else
		  {
		    Motion::MotionCommand command;
		    memset(&command, 0, sizeof(command));
		    command.motion_cmd=Motion::MOTION_STAND_NEUTRAL;
		    command.head_cmd=Motion::HEAD_LOOKAT;
		    command.tail_cmd=Motion::TAIL_NO_CMD;
		    command.head_lookat=vector3d(150,0,50);
		    subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
		    subject[sbjMotionControl]->NotifyObservers();
		    Wait(static_cast<longword>(2000000000));
		  }

		i++;
	}

}

/** Funzione invocata al ricevimento di un Assert Ready da parte di
    Motion.*/
void GeneraMove::Ready(const OReadyEvent& event){
	OSYSDEBUG(("GeneraMove:: ricevuto Assert Ready\n"));
	sph=1;
}

void GeneraMove::GetCamera(const ONotifyEvent& event) {

        imageVec = (OFbkImageVectorData*)event.Data(0);
        OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
	byte*          data = imageVec->GetData(ofbkimageLAYER_C);

	OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

	int width = cdtImage.Width();
	int height = cdtImage.Height();    
	int m = 0;
	int n = 0;
	//int pix_count[grids_x][grids_y];
	 int **pix_count = (int**) calloc(grids_x, sizeof(int*));
	 int **grid_matrix = (int**) calloc(grids_x, sizeof(int*));
	for (int i=0; i<grids_x; i++)
	{
		grid_matrix[i] = (int*) calloc(grids_y, sizeof(int));
		pix_count[i] = (int*) calloc(grids_y, sizeof(int));
	}
	int thrs = 150;

	int x, y;

	for (x=0; x < width; x++)
	{
		for (y=0; y < height; y++)
		{
			if (cdtImage.Pixel(x, y) & ocdtCHANNEL1)	// canale del grigio/pista
			{
				m = (int) floor( (float) (x * grids_x) / (float) width );
				n = (int) floor( (float) (y * grids_y) / (float) height );
				pix_count[m][n]++;
			}
		}
	}

	//calc_grid
	//int step_x = width/grids_x;
	//int step_y = height/grids_y;
	//int x_rett = 0;
	//int y_rett = 0;

	for (x=0; x < grids_x; x++)
	{
		for (y=0; y < grids_y; y++)
		{
			if (pix_count[x][y] > thrs)
				grid_matrix[x][y] = 0;
			else
				grid_matrix[x][y] = 100;
		}
	}

	//OSYSDEBUG(("pix count davanti: %d\n", pix_count[3][1]));
	//minefield
	//int max_x = sizeof(grid_matrix[0]) / sizeof(int);
	//int max_y = sizeof(grid_matrix) /sizeof(int);

	for (y=0; y < grids_y; y++)
	{
		for (x=0; x < grids_x; x++)
		{
			if (grid_matrix[x][y] == 100)
			{
				if ((x+1 < grids_x) && (grid_matrix[x+1][y] != 100))
					grid_matrix[x+1][y]+=1;
                if ((y+1 < grids_y) && (grid_matrix[x][y+1] != 100))
                    grid_matrix[x][y+1]+=1;
                if ((x+1 < grids_x) && (y+1 < grids_y) && (grid_matrix[x+1][y+1] != 100))
                    grid_matrix[x+1][y+1]+=1;
                if ((y-1 >= 0) && (x-1 > 0) && (grid_matrix[x-1][y-1] != 100))
                    grid_matrix[x-1][y-1]+=1;
                if ((y-1 >= 0) && (grid_matrix[x][y-1] != 100))
                    grid_matrix[x][y-1]+=1;
                if ((x-1 >= 0) && (grid_matrix[x-1][y] != 100))
                    grid_matrix[x-1][y]+=1;
                if ((y-1 >= 0) && (x+1 < grids_x) && (grid_matrix[x+1][y-1] != 100))
                    grid_matrix[x+1][y-1]+=1;
                if ((x-1 >= 0) && (y+1 < grids_y) && (grid_matrix[x-1][y+1] != 100))
                    grid_matrix[x-1][y+1]+=1;
			}
		}
	}

	OSYSDEBUG(("grid matrix davanti: %d\n", grid_matrix[3][1]));

	 Motion::MotionCommand command;
		    memset(&command, 0, sizeof(command));


		    if (grid_matrix[3][1] < 3 && grid_matrix[2][1] < 3)
		      {
			OSYSDEBUG(("entrato IF: %d\n", grid_matrix[3][1]));

			command.motion_cmd=Motion::MOTION_WALK_TROT;
			command.head_cmd=Motion::HEAD_LOOKAT;
			command.tail_cmd=Motion::TAIL_NO_CMD;
			command.head_lookat=vector3d(150,0,50);
			command.vx=100;
			command.vy=0;
			command.va=0.005;
			if (sph ==1){
			  subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
			  subject[sbjMotionControl]->NotifyObservers();
			  sph=0;
			}
			Wait(static_cast<longword>(2000000000));
		      }
		    else if (grid_matrix[3][1] < 3 && grid_matrix[3][0] < 3)
		      {
			OSYSDEBUG(("entrato IF: %d\n", grid_matrix[3][1]));

			command.motion_cmd=Motion::MOTION_WALK_TROT;
			command.head_cmd=Motion::HEAD_LOOKAT;
			command.tail_cmd=Motion::TAIL_NO_CMD;
			command.head_lookat=vector3d(150,0,50);
			command.vx=100;
			command.vy=0;
			command.va=0.835;
			if (sph ==1){
			  subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
			  subject[sbjMotionControl]->NotifyObservers();
			  sph=0;
			}
			Wait(static_cast<longword>(1000000000));
		      }
		    else
		      {
			OSYSDEBUG(("entrato ELSE\n"));
			command.motion_cmd=Motion::MOTION_STAND_NEUTRAL;
			command.head_cmd=Motion::HEAD_LOOKAT;
			command.tail_cmd=Motion::TAIL_NO_CMD;
			command.head_lookat=vector3d(150,0,50);
			command.vx=0;
			command.vy=0;
			command.va=0;
			if (sph ==1){
			  subject[sbjMotionControl]->SetData(&command,sizeof(Motion::MotionCommand));
			  subject[sbjMotionControl]->NotifyObservers();
			  sph=0;
			}

	}



	observer[event.ObsIndex()]->AssertReady();
}

/** Funzione che setta i parametri della camera. */
void GeneraMove::SetCameraParameter() {

	OPrimitiveControl_CameraParam shutter(ocamparamSHUTTER_MID);
	OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_SHUTTER_SPEED, &shutter, sizeof(shutter ) , 0 , 0);

	OPrimitiveControl_CameraParam gain(ocamparamGAIN_HIGH);
	OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_GAIN, &gain , sizeof(gain), 0,0);

	OPrimitiveControl_CameraParam wb(ocamparamWB_INDOOR_MODE) ;
	OPENR::ControlPrimitive (fbkID,oprmreqCAM_SET_WHITE_BALANCE, &wb, sizeof(wb),0,0);

	// Bilanciamento del bianco automatico
	OPENR::ControlPrimitive (fbkID, oprmreqCAM_AWB_ON, 0, 0, 0, 0);
	// Bilanciamento esposizione automatico
	OPENR::ControlPrimitive (fbkID, oprmreqCAM_AE_ON, 0, 0, 0, 0);

}

void
GeneraMove::SetCdtVectorData()
{
	OStatus result;
	MemoryRegionID  cdtVecID;
	OCdtVectorData* cdtVec;
	OCdtInfo*       cdtPink;
	OCdtInfo*       cdtGray;
	OCdtInfo*       cdtWhite;

	result = OPENR::NewCdtVectorData(&cdtVecID, &cdtVec);
	if (result != oSUCCESS) {
		OSYSLOG1((osyslogERROR, "%s : %s %d",
				"GeneraMove::SetCdtVectorData()",
				"OPENR::NewCdtVectorData() FAILED", result));
		return;
	}

	// numero di canali da usare
	cdtVec->SetNumData(2);

	// setting della CDT per la palla
	cdtPink = cdtVec->GetInfo(0);
	cdtPink->Init(fbkID, ocdtCHANNEL0);

	//
	// cdtPink->Set(Y_segment, Cr_max,  Cr_min, Cb_max, Cb_min)
	//
	cdtPink->Set( 1, 216, 203, 110, 105);
	cdtPink->Set( 2, 250, 207, 119, 109);
	cdtPink->Set( 3, 240, 238, 120, 114);
	cdtPink->Set( 4, 207, 207, 121, 121);

	cdtPink->Set( 0, 230, 150, 190, 120);
	cdtPink->Set( 5, 230, 150, 190, 120);
	cdtPink->Set( 6, 230, 150, 190, 120);
	cdtPink->Set( 7, 230, 150, 190, 120);
	cdtPink->Set( 8, 230, 150, 190, 120);
	cdtPink->Set( 9, 230, 150, 190, 120);
	cdtPink->Set(10, 230, 150, 190, 120);
	cdtPink->Set(11, 230, 150, 190, 120);
	cdtPink->Set(12, 230, 150, 190, 120);
	cdtPink->Set(13, 230, 150, 190, 120);
	cdtPink->Set(14, 230, 150, 190, 120);
	cdtPink->Set(15, 230, 150, 190, 120);
	cdtPink->Set(16, 230, 150, 190, 120);
	cdtPink->Set(17, 230, 150, 190, 120);
	cdtPink->Set(18, 230, 150, 190, 120);
	cdtPink->Set(19, 230, 150, 190, 120);
	cdtPink->Set(20, 230, 160, 190, 120);
	cdtPink->Set(21, 230, 160, 190, 120);
	cdtPink->Set(22, 230, 160, 190, 120);
	cdtPink->Set(23, 230, 160, 190, 120);
	cdtPink->Set(24, 230, 160, 190, 120);
	cdtPink->Set(25, 230, 160, 190, 120);
	cdtPink->Set(26, 230, 160, 190, 120);
	cdtPink->Set(27, 230, 160, 190, 120);
	cdtPink->Set(28, 230, 160, 190, 120);
	cdtPink->Set(29, 230, 160, 190, 120);
	cdtPink->Set(30, 230, 160, 190, 120);
	cdtPink->Set(31, 230, 160, 190, 120);

	// setting della CDT per il bordo grigio del tracciato
	cdtGray = cdtVec->GetInfo(1);
	cdtGray->Init(fbkID, ocdtCHANNEL1);

	//
	// cdtGray->Set(Y_segment, Cr_max,  Cr_min, Cb_max, Cb_min)
	//
	cdtGray->Set( 0, 114, 83, 173, 150);
	cdtGray->Set( 1, 114, 83, 173, 150);
	cdtGray->Set( 2, 114, 83, 173, 150);
	cdtGray->Set( 3, 114, 83, 173, 150);
	cdtGray->Set( 4, 114, 83, 173, 150);
	cdtGray->Set( 5, 114, 83, 173, 150);
	cdtGray->Set( 6, 114, 83, 173, 150);
	cdtGray->Set( 7, 114, 83, 173, 150);
	cdtGray->Set( 8, 114, 83, 173, 150);
	cdtGray->Set( 9, 114, 83, 173, 150);
	cdtGray->Set(10, 114, 83, 173, 150);
	cdtGray->Set(11, 114, 83, 173, 150);
	cdtGray->Set(12, 114, 83, 173, 150);
	cdtGray->Set(13, 114, 83, 173, 150);
	cdtGray->Set(14, 114, 83, 173, 150);
	cdtGray->Set(15, 114, 83, 173, 150);
	cdtGray->Set(16, 114, 83, 173, 150);
	cdtGray->Set(17, 114, 83, 173, 150);
	cdtGray->Set(18, 114, 83, 173, 150);
	cdtGray->Set(19, 114, 83, 173, 150);
	cdtGray->Set(20, 114, 83, 173, 150);
	cdtGray->Set(21, 114, 83, 173, 150);
	cdtGray->Set(22, 114, 83, 173, 150);
	cdtGray->Set(23, 114, 83, 173, 150);
	cdtGray->Set(24, 114, 83, 173, 150);
	cdtGray->Set(25, 114, 83, 173, 150);
	cdtGray->Set(26, 114, 83, 173, 150);
	cdtGray->Set(27, 114, 83, 173, 150);
	cdtGray->Set(28, 114, 83, 173, 150);
	cdtGray->Set(29, 114, 83, 173, 150);
	cdtGray->Set(30, 114, 83, 173, 150);
	cdtGray->Set(31, 114, 83, 173, 150);



	// setting della CDT per il bordo bianco del tracciato
/*	cdtWhite = cdtVec->GetInfo(3);
	cdtWhite->Init(fbkID, ocdtCHANNEL3);

	//
	// cdtWhite->Set(Y_segment, Cr_max,  Cr_min, Cb_max, Cb_min)
	//
	cdtWhite->Set( 0, 101, 96, 155, 150);
	cdtWhite->Set( 1, 101, 96, 155, 150);
	cdtWhite->Set( 2, 101, 96, 155, 150);
	cdtWhite->Set( 3, 101, 96, 155, 150);
	cdtWhite->Set( 4, 101, 96, 155, 150);
	cdtWhite->Set( 5, 101, 96, 155, 150);
	cdtWhite->Set( 6, 101, 96, 155, 150);
	cdtWhite->Set( 7, 101, 96, 155, 150);
	cdtWhite->Set( 8, 101, 96, 155, 150);
	cdtWhite->Set( 9, 101, 96, 155, 150);
	cdtWhite->Set(10, 101, 96, 155, 150);
	cdtWhite->Set(11, 101, 96, 155, 150);
	cdtWhite->Set(12, 101, 96, 155, 150);
	cdtWhite->Set(13, 101, 96, 155, 150);
	cdtWhite->Set(20, 101, 96, 155, 150);
	cdtWhite->Set(21, 101, 96, 155, 150);
	cdtWhite->Set(22, 101, 96, 155, 150);
	cdtWhite->Set(23, 101, 96, 155, 150);
	cdtWhite->Set(24, 101, 96, 155, 150);
	cdtWhite->Set(25, 101, 96, 155, 150);
	cdtWhite->Set(26, 101, 96, 155, 150);
	cdtWhite->Set(27, 101, 96, 155, 150);
	cdtWhite->Set(28, 101, 96, 155, 150);
	cdtWhite->Set(29, 101, 96, 155, 150);
	cdtWhite->Set(30, 101, 96, 155, 150);
	cdtWhite->Set(31, 101, 96, 155, 150);
	cdtWhite->Set( 15, 102, 96, 153, 147);
	cdtWhite->Set( 17, 101, 96, 155, 154);
	cdtWhite->Set( 16, 101, 95, 155, 150);
	cdtWhite->Set( 19, 97, 95, 155, 153);
	cdtWhite->Set( 18, 99, 96, 155, 153);
	cdtWhite->Set( 14, 105, 101, 152, 144);*/


	result = OPENR::SetCdtVectorData(cdtVecID);
	if (result != oSUCCESS) {
		OSYSLOG1((osyslogERROR, "%s : %s %d",
				"GeneraMove::SetCdtVectorData()",
				"OPENR::SetCdtVectorData() FAILED", result));
	}

	result = OPENR::DeleteCdtVectorData(cdtVecID);
	if (result != oSUCCESS) {
		OSYSLOG1((osyslogERROR, "%s : %s %d",
				"GeneraMove::SetCdtVectorData()",
				"OPENR::DeleteCdtVectorData() FAILED", result));
	}
}

void
GeneraMove::RegionGrowing(OFbkImageVectorData* imageVec)
{
	OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
	byte*          data = imageVec->GetData(ofbkimageLAYER_C);

	OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

	int width = cdtImage.Width();
	int height = cdtImage.Height();

	int x, y;

	// prima passata - espansione
	// se un pixel ha colore target, coloro i pixel adiacenti dello stesso colore
	for (x=0; x < width; x++)
	{
		for (y=0; y < height; y++)
		{
			if (cdtImage.Pixel(x, y) & ocdtCHANNEL0)
			{

			}
		}
	}
}

bool
GeneraMove::InsideTrack(OFbkImageVectorData* imageVec, int topLine, int linesToCheck)
{
	bool isInside = false;
	bool isGray = false;
	bool isWhite = false;

	OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
	byte*          data = imageVec->GetData(ofbkimageLAYER_C);

	OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

	int width = cdtImage.Width();
	int height = cdtImage.Height();

	int x, y;

	for (y=topLine; (y < height) && (linesToCheck > 0); y++)
	{
		for (x=0; x < width; x++)
		{
			if (cdtImage.Pixel(x, y) & ocdtCHANNEL1)
			{
				isGray = true;
			}
			if ((cdtImage.Pixel(x, y) & ocdtCHANNEL2) && isGray)
			{
				isWhite = true;
			}
		}

		if (isGray && isWhite)
		{
			linesToCheck--;
		}

		isGray = isWhite = false;
	}

	if (linesToCheck == 0)
	{
		isInside = true;
	}

	return isInside;
}

int**
GeneraMove::Grid(OFbkImageVectorData* imageVec)
{
  sph = 0;
	OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
	byte*          data = imageVec->GetData(ofbkimageLAYER_C);

	OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

	int width = cdtImage.Width();
	int height = cdtImage.Height();
	int m = 0;
	int n = 0;
	int pix_count[grids_x][grids_y];
	 int **grid_matrix = (int**) calloc(grids_x, sizeof(int*));
	for (int i=0; i<grids_x; i++)
	{
		grid_matrix[i] = (int*) calloc(grids_y, sizeof(int));
	}
	int thrs = 150;

	int x, y;

	for (x=0; x < width; x++)
	{
		for (y=0; y < height; y++)
		{
			if (cdtImage.Pixel(x, y) & ocdtCHANNEL1)	// canale del grigio/pista
			{
				m = (int) floor( (float) (x * grids_x) / (float) width );
				n = (int) floor( (float) (y * grids_y) / (float) height );
				pix_count[m][n]++;
			}
		}
	}

	//calc_grid
	//int step_x = width/grids_x;
	//int step_y = height/grids_y;
	//int x_rett = 0;
	//int y_rett = 0;

	for (x=0; x < grids_x; x++)
	{
		for (y=0; y < grids_y; y++)
		{
			if (pix_count[x][y] > thrs)
				grid_matrix[x][y] = 0;
			else
				grid_matrix[x][y] = 100;
		}
	}

	//minefield
	//int max_x = sizeof(grid_matrix[0]) / sizeof(int);
	//int max_y = sizeof(grid_matrix) /sizeof(int);
	OSYSDEBUG(("Arrivato fino a Minefield"));
	for (y=0; y < grids_y; y++)
	{
		for (x=0; x < grids_x; x++)
		{
			if (grid_matrix[x][y] == 100)
			{
				if ((x+1 < grids_x) && (grid_matrix[x+1][y] != 100))
					grid_matrix[x+1][y]+=1;
                if ((y+1 < grids_y) && (grid_matrix[x][y+1] != 100))
                    grid_matrix[x][y+1]+=1;
                if ((x+1 < grids_x) && (y+1 < grids_y) && (grid_matrix[x+1][y+1] != 100))
                    grid_matrix[x+1][y+1]+=1;
                if ((y-1 >= 0) && (x-1 > 0) && (grid_matrix[x-1][y-1] != 100))
                    grid_matrix[x-1][y-1]+=1;
                if ((y-1 >= 0) && (grid_matrix[x][y-1] != 100))
                    grid_matrix[x][y-1]+=1;
                if ((x-1 >= 0) && (grid_matrix[x-1][y] != 100))
                    grid_matrix[x-1][y]+=1;
                if ((y-1 >= 0) && (x+1 < grids_x) && (grid_matrix[x+1][y-1] != 100))
                    grid_matrix[x+1][y-1]+=1;
                if ((x-1 >= 0) && (y+1 < grids_y) && (grid_matrix[x-1][y+1] != 100))
                    grid_matrix[x-1][y+1]+=1;
			}
		}
	}
	sph = 1;
	return grid_matrix;
}
