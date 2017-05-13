
#ifdef HAVE_CONFIG_H
#include "mbconfig.h"
#endif

#include <vector>
#include <drive.h>
#include <sstream>
#include <myassert.h>
#include <except.h>
#include <mynewmem.h>
#include <dataman.h>

#include "module-gsoc-puneet-mahajan.h"

class SwitchDriveCaller : public DriveCaller
{
public:
	explicit SwitchDriveCaller(const DriveHandler* pDH, 
				
                const std::string& velocity,
                const std::vector<DriveOwner>& drives);
	virtual ~SwitchDriveCaller();
	bool bIsDifferentiable(void) const;
	virtual std::ostream& Restart(std::ostream& out) const;
	doublereal dGet(const doublereal& dVar) const;
	virtual doublereal dGetP(const doublereal& dVar) const;
	DriveCaller* pCopy(void) const;

private:
	doublereal pGetDrive() const;
	const DataManager* pDM;
	const std::string velocity;
	typedef std::vector<DriveOwner>::const_iterator iterator;
	const std::vector<DriveOwner> rgDrives;
};

struct SwitchDriveDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

SwitchDriveCaller::SwitchDriveCaller(const DriveHandler* pDH,
	const std::string& velocity, const std::vector<DriveOwner>& drives):
  DriveCaller(pDH),
  velocity(velocity),
  rgDrives(drives)
{
	NO_OP;
}

SwitchDriveCaller::~SwitchDriveCaller()
{
	NO_OP;
}

doublereal SwitchDriveCaller::pGetDrive() const
{

	iterator i = rgDrives.begin();
	doublereal force = i->dGet();
        std::istringstream in(velocity);
        InputStream In(in);
        doublereal velocity =  DriveCaller::pDrvHdl->dGet(In);
    const doublereal mu_kinetic = 0.4;
    
    int init_velocity_sign;
    if ( velocity != 0.0 )
    {
		init_velocity_sign = velocity/abs(velocity);
	}
	else
	{
		init_velocity_sign = 0.0;
	}

    if (init_velocity_sign * velocity > 0.0)
    {
        force =  - mu_kinetic * 9.81; 
    }
    else
    {
        force = 0.0;
    }
    
    // if (velocity < 0.0){
    // 	force =  9.81 * mu_kinetic; 
    // 	return -force;
    // }
    // else if (velocity > 0.0){
    // 	force =  -9.81 * mu_kinetic; 
    // 	return -force;    
    // }
    // else{
    // 	force = 0.0;
    // 	return -force;    
    // }
    
	return force;
}

doublereal SwitchDriveCaller::dGet(const doublereal& dVar) const
{
	return pGetDrive();
}

doublereal SwitchDriveCaller::dGetP(const doublereal& dVar) const
{
	return pGetDrive();
}

bool SwitchDriveCaller::bIsDifferentiable(void) const
{
	for (iterator i = rgDrives.begin(); i != rgDrives.end(); ++i) {
		if (!i->bIsDifferentiable()) {
			return false;
		}
	}

	return true;
}

/* Restart */
std::ostream&
SwitchDriveCaller::Restart(std::ostream& out) const
{
	return out;
}


DriveCaller*
SwitchDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;

	SAFENEWWITHCONSTRUCTOR(pDC,
			SwitchDriveCaller,
			SwitchDriveCaller(pGetDrvHdl(), velocity, rgDrives));

	return pDC;
}

DriveCaller *
SwitchDriveDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "friction_puneet");

	const DriveHandler* pDrvHdl = 0;

	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	/* driver legato ad un grado di liberta' nodale */
	if (pDM == 0) {
		silent_cerr("sorry, since the driver is not owned by a DataManager" << std::endl
			<< "no DOF dependent drivers are allowed;" << std::endl
			<< "aborting..." << std::endl);
		throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	DriveCaller *pDC = 0;

	const integer iNumDrives = 1;

	std::vector<DriveOwner> rgDrives;

	rgDrives.reserve(iNumDrives);

	rgDrives.push_back(HP.GetDriveCaller());

        std::string velocity(HP.GetStringWithDelims());

        bool bString(false);
        for (std::string::iterator i = velocity.begin(); i != velocity.end();) {
                if (isspace(*i)) {
                        if (!bString) {
                                bString = true;
                                ++i;

                        } else {
                                i = velocity.erase(i);
                        }

                } else {
                        if (bString) {
                                bString = false;
                        }
                        ++i;
                }
        }

	SAFENEWWITHCONSTRUCTOR(pDC,
		SwitchDriveCaller,
		SwitchDriveCaller(pDrvHdl, velocity, rgDrives));

	return pDC;
}

bool
switch_drive_set()
{
	DriveCallerRead	*rf = new SwitchDriveDCR;

	if (!SetDriveCallerData("friction_puneet", rf)) {
		delete rf;
		return false;
	}

	return true;
}

#ifndef STATIC_MODULES

extern "C" int
module_init(const char *module_name, void *pdm, void *php)
{
	if (!switch_drive_set()) {
		silent_cerr("friction_puneet: "
			"module_init(" << module_name << ") "
			"failed" << std::endl);
		return -1;
	}

	return 0;
}

#endif 
