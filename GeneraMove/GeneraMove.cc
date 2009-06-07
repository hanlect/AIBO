#include "GeneraMove.h"
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include <OPENR/OPENRAPI.h>
#include "entry.h"
#include "../Motion/MotionInterface.h"
#include "EasyBMP.h"
#include "EasyBMP_BMP.h"
#include <iostream>
#include <fstream>


//char* model="ERS7";
typedef unsigned char BYTE;

GeneraMove::GeneraMove(){

	//
	// Comando di movimento
	//
	/*memset(&mMoveCommand, 0, sizeof(mMoveCommand));
	//mMoveCommand.motion_cmd=Motion::MOTION_KICK_HEAD_L;
	mMoveCommand.head_cmd=Motion::HEAD_LOOKAT;
	mMoveCommand.tail_cmd=Motion::TAIL_NO_CMD;

	mMoveCommand.head_lookat=vector3d(200,0,50);*/

	memset(&mMoveCommand, 0, sizeof(mMoveCommand));
	mMoveCommand.motion_cmd=Motion::MOTION_WALK_TROT;
	mMoveCommand.head_cmd=Motion::HEAD_LOOKAT;
	mMoveCommand.tail_cmd=Motion::TAIL_NO_CMD;
	mMoveCommand.head_lookat=vector3d(150,0,50);
	mMoveCommand.vx = 100;
	mMoveCommand.vy = 30;
	mMoveCommand.va = 0;

	//
	// Comando per metterlo in posizione di riposo
	//
	memset(&mStandCommand, 0, sizeof(mStandCommand));
	mStandCommand.motion_cmd=Motion::MOTION_STAND_NEUTRAL;
	mStandCommand.head_cmd=Motion::HEAD_LOOKAT;
	mStandCommand.tail_cmd=Motion::TAIL_NO_CMD;
	mStandCommand.head_lookat=vector3d(150,0,50);

	// Cammina



	sph = 1;
	imageVec = NULL;
	mBMP = new BMP();
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
	//
	// Accendo i motori (per scrupolo)
	//
	OPENR::SetMotorPower(opowerON);

	//
	// Inclino la testa verso il basso
	//
	subject[sbjMotionControl]->SetData(&mMoveCommand,sizeof(Motion::MotionCommand));
	subject[sbjMotionControl]->NotifyObservers();

//	subject[sbjMotionControl]->SetData(&mWalkCommand,sizeof(Motion::MotionCommand));
//	subject[sbjMotionControl]->NotifyObservers();


	//
	// Avvio un timer con scadenza 200 msec
	//
	int dummy=0;
	EventID sincroEvent = EventID();
	RelativeTime period(0, 200);
	TimeEventInfoWithRelativeTime timeInfo(TimeEventInfo::NonPeriodic, period);
	sError error=SetTimeEvent(&timeInfo,myOID_,Extra_Entry[entryTimerEnd],&dummy,sizeof(dummy),&sincroEvent);
	if(error!=sSUCCESS)
		OSYSLOG1((osyslogERROR, "::DoStart() : ERROR: Sincronization Timer not started\n"));

	return oSUCCESS;
}  //DoStart() END


/** Fermo la comunicazione inter-object.*/
OStatus GeneraMove::DoStop(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoStop()\n"));

	DISABLE_ALL_SUBJECT;
	DEASSERT_READY_TO_ALL_OBSERVER;
	OPENR::SetMotorPower(opowerOFF);

	return oSUCCESS;
}  //DoStop() END


/** Distrutto le strutture di comunicazione inter-object.*/
OStatus GeneraMove::DoDestroy(const OSystemEvent& event){
	OSYSDEBUG(("GeneraMove::DoDestroy\()\n"));

	DELETE_ALL_SUBJECT_AND_OBSERVER;

	return oSUCCESS;
}  //DoDestroy() END

/** Metodo invocato allo scadere del timer.*/
void GeneraMove::TimerEnd(void* msg){


	//
	// Leggo il valore del sensore posteriore.
	//
	OSensorValue sensorValue;
	OPENR::GetSensorValue(mBackrID, &sensorValue);

	//
	// Se il sensore e' stato toccato
	//
	if(sensorValue.value >= 14)
	{
		OSYSDEBUG(("Sensore toccato.\n"));

		char dir[128];
		char name[128];
		sph = 0;

		strcpy( dir , "/MS/OPEN-R/MW/DATA/P/" );
		sprintf(name, "%sLAYM%d.BMP", dir, count);
		mBMP->SaveYCrCb2RGB(name, imageVec, ofbkimageLAYER_M);
		//mBMP->SaveLayerC(dir, imageVec);
		mBMP->SaveLayerCmod(dir, imageVec, count);
		mBMP->SaveLayerCcolors(dir, imageVec, count);

		sph = 1;

		OSYSDEBUG(("Immagini salvate.\n"));

		//
		// region growing algorithm
		//
//		RegionGrowing(imageVec);



		// declare and read the bitmap
/*		eBMP Image, ImageTemp;
		sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYC%d.BMP", count);
		Image.ReadFromFile( name );
		ImageTemp.ReadFromFile( name );

		// prima passata - espansione
		// se un pixel ha colore target, coloro i pixel adiacenti dello stesso colore
		for( int i=1 ; i < Image.TellWidth()-1 ; i++)
		{
			for( int j=1 ; j < Image.TellHeight()-1 ; j++)
			{
				if (Image(i,j)->Red == 255)
				{
					ImageTemp(i-1,j-1)->Red = ImageTemp(i,j-1)->Red = ImageTemp(i+1,j-1)->Red =
						ImageTemp(i-1,j)->Red = ImageTemp(i+1,j)->Red =
							ImageTemp(i-1,j+1)->Red = ImageTemp(i,j+1)->Red = ImageTemp(i+1,j+1)->Red = 255;

					ImageTemp(i-1,j-1)->Green = ImageTemp(i,j-1)->Green = ImageTemp(i+1,j-1)->Green =
						ImageTemp(i-1,j)->Green = ImageTemp(i+1,j)->Green =
							ImageTemp(i-1,j+1)->Green = ImageTemp(i,j+1)->Green = ImageTemp(i+1,j+1)->Green = 255;

					ImageTemp(i-1,j-1)->Blue = ImageTemp(i,j-1)->Blue = ImageTemp(i+1,j-1)->Blue =
						ImageTemp(i-1,j)->Blue = ImageTemp(i+1,j)->Blue =
							ImageTemp(i-1,j+1)->Blue = ImageTemp(i,j+1)->Blue = ImageTemp(i+1,j+1)->Blue = 255;
				}
			}
		}

		// copio ImageTemp in Image
		RangedPixelToPixelCopy( ImageTemp, 0, ImageTemp.TellWidth(), ImageTemp.TellHeight(), 0, Image, 0, 0 );

		// seconda passata - erosione
		// se un pixel ha colore NON target, coloro i pixel adiacenti dello stesso colore
		for( int i=1 ; i < Image.TellWidth()-1 ; i++)
		{
			for( int j=1 ; j < Image.TellHeight()-1 ; j++)
			{
				if (Image(i,j)->Red == 0)
				{
					ImageTemp(i-1,j-1)->Red = ImageTemp(i,j-1)->Red = ImageTemp(i+1,j-1)->Red =
						ImageTemp(i-1,j)->Red = ImageTemp(i+1,j)->Red =
							ImageTemp(i-1,j+1)->Red = ImageTemp(i,j+1)->Red = ImageTemp(i+1,j+1)->Red = 0;

					ImageTemp(i-1,j-1)->Green = ImageTemp(i,j-1)->Green = ImageTemp(i+1,j-1)->Green =
						ImageTemp(i-1,j)->Green = ImageTemp(i+1,j)->Green =
							ImageTemp(i-1,j+1)->Green = ImageTemp(i,j+1)->Green = ImageTemp(i+1,j+1)->Green = 0;

					ImageTemp(i-1,j-1)->Blue = ImageTemp(i,j-1)->Blue = ImageTemp(i+1,j-1)->Blue =
						ImageTemp(i-1,j)->Blue = ImageTemp(i+1,j)->Blue =
							ImageTemp(i-1,j+1)->Blue = ImageTemp(i,j+1)->Blue = ImageTemp(i+1,j+1)->Blue = 0;
				}
			}
		}

		// copio ImageTemp in Image
		RangedPixelToPixelCopy( ImageTemp, 0, ImageTemp.TellWidth(), ImageTemp.TellHeight(), 0, Image, 0, 0 );

		// terza passata
		// riempio le regioni "vuote" scorrendo sulle colonne

		int start, p;

		for( int j=0 ; j < Image.TellHeight() ; j++)
		{
			for( int i=0 ; i < Image.TellWidth() ; i++)
			{
				if (Image(i,j)->Red == 255)
				{
					p = i+1;
					start = p;
					while((Image(p,j)->Red == 0) && (p < Image.TellWidth()-1))
					{
						p++;
					}
					if (p-i > 10)
						p = start;
					while( p > i )
					{
						ImageTemp(p-1,j)->Red   = (BYTE) 255;
						ImageTemp(p-1,j)->Green = (BYTE) 255;
						ImageTemp(p-1,j)->Blue  = (BYTE) 255;
						p--;
					}
				}
			}
		}

		// write the output file
		sprintf(name, "/MS/OPEN-R/MW/DATA/P/LAYC%d_rg.BMP", count);
		ImageTemp.WriteToFile( name );


		// calcolo della distanza in mm
		int bhMin, bhMax, bwMin, bwMax, ballHeight, ballWidth;
		float f, ballDim, realBallDim, dist;

		bhMin = ImageTemp.TellHeight() - 1;
		bwMin = ImageTemp.TellWidth() - 1;
		bhMax = 0;
		bwMax = 0;

		for( int j=0 ; j < Image.TellHeight() ; j++)
		{
			for( int i=0 ; i < Image.TellWidth() ; i++)
			{
				if (ImageTemp(i,j)->Red == 255)
				{
					if (j > bhMax)
						bhMax = j;
					if (j < bhMin)
						bhMin = j;
					if (i > bwMax)
						bwMax = i;
					if (i < bwMin)
						bwMin = i;
				}
			}
		}

		ballHeight = bhMax - bhMin;
		ballWidth = bwMax - bwMin;
		ballDim = (ballHeight + ballWidth) * 0.5;

		OSYSDEBUG(("%d x %d\ndiametro palla img = %d pixel | ", ballWidth, ballHeight, (int)ballDim));

		f = 3.27f;   // focale
		realBallDim = 70.0f;   // diametro della palla reale in mm
		ballDim = 0.0352f * ballDim;   // conversione della dimensione da pixel a mm
		dist = (f * realBallDim) / ballDim;

		OSYSDEBUG(("%f mm\ndistanza dalla palla = %f mm\n", ballDim, dist));
*/

		//
		// Mando il comando di movimento.
		//
		//      subject[sbjMotionControl]->SetData(&mMoveCommand,sizeof(Motion::MotionCommand));
		//      subject[sbjMotionControl]->NotifyObservers();

		//
		// Aspetto 1 secondo.
		//
		Wait(static_cast<longword>(1000000000));
		count++;
		//
		// Mando il comando di riposo
		//
		//subject[sbjMotionControl]->SetData(&mStandCommand,sizeof(Motion::MotionCommand));
		//subject[sbjMotionControl]->NotifyObservers();
	}

	//
	// Faccio ripartire il timer.
	//
	int dummy=0;
	EventID sincroEvent = EventID();
	RelativeTime period(0, 200);
	TimeEventInfoWithRelativeTime timeInfo(TimeEventInfo::NonPeriodic, period);
	sError error=SetTimeEvent(&timeInfo,myOID_,Extra_Entry[entryTimerEnd],&dummy,sizeof(dummy),&sincroEvent);
	if(error!=sSUCCESS)
		OSYSLOG1((osyslogERROR, "::DoStart() : ERROR: Sincronization Timer not started\n"));
	OSYSDEBUG(("ESCO DAL TIMER\n"));
}

/** Funzione invocata al ricevimento di un Assert Ready da parte di
    Motion.*/
void GeneraMove::Ready(const OReadyEvent& event){
	OSYSDEBUG(("GeneraMove:: ricevuto Assert Ready\n"));
}

void GeneraMove::GetCamera(const ONotifyEvent& event) {

	if(sph == 1) imageVec = (OFbkImageVectorData*)event.Data(0);

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
	cdtVec->SetNumData(1);

	// setting della CDT per la palla
/*	cdtPink = cdtVec->GetInfo(0);
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
	cdtPink->Set(31, 230, 160, 190, 120);*/

	// setting della CDT per il bordo grigio del tracciato
	cdtGray = cdtVec->GetInfo(0);
	cdtGray->Init(fbkID, ocdtCHANNEL0);

	//
	// cdtGray->Set(Y_segment, Cr_max,  Cr_min, Cb_max, Cb_min)
	//
/*	cdtGray->Set( 0, 230, 150, 190, 120);
	cdtGray->Set( 1, 230, 150, 190, 120);
	cdtGray->Set( 2, 116, 98, 151, 121);
	cdtGray->Set( 3, 117, 97, 155, 126);
	cdtGray->Set( 4, 85, 78, 168, 160);
	cdtGray->Set( 5, 230, 150, 190, 120);
	cdtGray->Set( 6, 230, 150, 190, 120);
	cdtGray->Set( 7, 230, 150, 190, 120);
	cdtGray->Set( 8, 115, 101, 150, 132);
	cdtGray->Set( 9, 113, 99, 146, 121);
	cdtGray->Set(10, 112, 101, 147, 124);
	cdtGray->Set(11, 116, 98, 151, 124);
	cdtGray->Set(12, 117, 98, 155, 126);
	cdtGray->Set(13, 109, 97, 155, 132);
	cdtGray->Set(14, 99, 99, 151, 151);
	cdtGray->Set(15, 99, 99, 154, 154);
//	cdtGray->Set(16, 230, 150, 190, 120);
	cdtGray->Set(17, 85, 78, 168, 163);
//	cdtGray->Set(18, 230, 150, 190, 120);
	cdtGray->Set(19, 84, 81, 166, 160);
	cdtGray->Set(20, 230, 160, 190, 120);
	cdtGray->Set(21, 230, 160, 190, 120);
	cdtGray->Set(22, 230, 160, 190, 120);
	cdtGray->Set(23, 230, 160, 190, 120);
	cdtGray->Set(24, 230, 160, 190, 120);
	cdtGray->Set(25, 230, 160, 190, 120);
	cdtGray->Set(26, 230, 160, 190, 120);
	cdtGray->Set(27, 230, 160, 190, 120);
	cdtGray->Set(28, 230, 160, 190, 120);
	cdtGray->Set(29, 230, 160, 190, 120);
	cdtGray->Set(30, 230, 160, 190, 120);
	cdtGray->Set(31, 230, 160, 190, 120);*/

/*	cdtGray->Set( 5, 116, 114, 127, 124);
	cdtGray->Set( 3, 119, 119, 131, 131);
	cdtGray->Set( 7, 112, 108, 133, 127);
	cdtGray->Set( 10, 107, 101, 145, 128);
	cdtGray->Set( 9, 105, 105, 137, 137);
	cdtGray->Set( 11, 111, 108, 128, 127);
	cdtGray->Set( 8, 111, 111, 133, 133);
	cdtGray->Set( 6, 112, 112, 133, 126);*/

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
/*	cdtWhite = cdtVec->GetInfo(2);
	cdtWhite->Init(fbkID, ocdtCHANNEL2);

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
