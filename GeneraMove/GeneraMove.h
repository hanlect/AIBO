#ifndef PROVACM_H_DEFINED
#define PROVACM_H_DEFINED
#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/RCRegion.h>
#include <OPENR/OFbkImage.h>
#include "def.h"
#include "BMP.h"
#include "R_list.h"

// Contiene le costanti di gestione del movimento di motion.
#include "../Motion/MotionInterface.h"

using namespace std;

// Primitiva di gestione del sensore di tatto posteriore
static const char* const BACKR7_LOCATOR = "PRM:/t2-Sensor:t2"; // REAR

// Primitiva della telecamera
static const char* const FBK_LOCATOR = "PRM:/r1/c1/c2/c3/i1-FbkImageSensor:F1";

/** Alla pressione del sensore di tatto posteriore effettua un tiro
    con la testa.*/
class GeneraMove : public OObject {
public:

	/** Costruttore di default.*/
	GeneraMove();
	/** Distruttore di default (virtual in quanto figlio di OObject).*/
	virtual ~GeneraMove() {};

	/** Array dei subject dell'oggetto.*/
	OSubject*    subject[numOfSubject];
	/** Array degli observer dell'oggetto.*/
	OObserver*   observer[numOfObserver];

	/** Entry Point standard: inizializza le strutture di comunicazione
      inter-object.*/
	virtual OStatus DoInit   (const OSystemEvent& event);
	/** Entry Point standard: attiva la comunicazione inter-object.*/
	virtual OStatus DoStart  (const OSystemEvent& event);
	/** Entry Point standard: disattiva la comunicazione inter-object.*/
	virtual OStatus DoStop   (const OSystemEvent& event);
	/** Entry Point standard: distrugge le strutture di comunicazione
      inter-object.*/
	virtual OStatus DoDestroy(const OSystemEvent& event);

	/** Invocata al ricevimento di un Assert Ready da parte di Motion.*/
	void Ready(const OReadyEvent& event);

	/** Extra Entry Ponit: invocata allo scadere del timer.*/
	void TimerEnd(void* msg);

	/** Gets Camera Frame */
	void GetCamera(const ONotifyEvent& event);
	void SetCameraParameter();

	void SetCdtVectorData();
	void RegionGrowing(OFbkImageVectorData* imageVec);
	bool InsideTrack(OFbkImageVectorData* imageVec, int topLine, int linesToCheck);

private:
	/** Oggetto per la gestione della primitiva del sensore di tatto
      posteriore.*/
	OPrimitiveID mBackrID;
	OPrimitiveID fbkID;

	/** Movimento "tira la palla con la testa a sinistra".*/
	Motion::MotionCommand mMoveCommand;
	/** Movimento riposo.*/
	//Motion::MotionCommand mStandCommand;
	// Cammina
	//Motion::MotionCommand mWalkCommand;

	//* Current image frame */
	OFbkImageVectorData* imageVec;

	//* Semaphore */
	int sph;
	int count;

	//* BMP Writer */
	BMP* mBMP;
};

#endif
