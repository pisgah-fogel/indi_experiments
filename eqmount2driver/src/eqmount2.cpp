#include "eqmount2.h"

#include "indicom.h"

#include <cmath>
#include <memory>

// TODO: use indicom library to communicate with the arduino

static std::unique_ptr<Eqmount2> sEqMount(new Eqmount2());

/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties(const char *dev)
{
    sEqMount->ISGetProperties(dev);
}

/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    sEqMount->ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    sEqMount->ISNewText(dev, name, texts, names, n);
}

/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    sEqMount->ISNewNumber(dev, name, values, names, n);
}

/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    sEqMount->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice(XMLEle *root)
{
    sEqMount->ISSnoopDevice(root);
}

Eqmount2::Eqmount2()
{
    // We add an additional debug level so we can log verbose scope status
    DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");
}

/**************************************************************************************
** We init our properties here. The only thing we want to init are the Debug controls
***************************************************************************************/
bool Eqmount2::initProperties()
{
    // ALWAYS call initProperties() of parent first
    INDI::Telescope::initProperties();

    /* Tracking Mode */
    //AddTrackMode("TRACK_SIDEREAL", "Sidereal", true);
    //AddTrackMode("TRACK_SOLAR", "Solar"); // TODO
    //AddTrackMode("TRACK_LUNAR", "Lunar"); // TODO

    // Slew Rates
    //strncpy(SlewRateS[0].label, "1x", MAXINDILABEL);
    //strncpy(SlewRateS[1].label, "8x", MAXINDILABEL);
    //strncpy(SlewRateS[2].label, "16x", MAXINDILABEL);
    //strncpy(SlewRateS[3].label, "64x", MAXINDILABEL);
    //strncpy(SlewRateS[4].label, "128x", MAXINDILABEL);
    //strncpy(SlewRateS[5].label, "256x", MAXINDILABEL);
    //strncpy(SlewRateS[6].label, "512x", MAXINDILABEL);
    //strncpy(SlewRateS[7].label, "MAX", MAXINDILABEL);
    //IUResetSwitch(&SlewRateSP);
    //SlewRateS[0].s = ISS_ON; // TODO: higher slew rate by default

    TrackRateN[AXIS_RA].min = TRACKRATE_SIDEREAL - 0.01;
    TrackRateN[AXIS_RA].max = TRACKRATE_SIDEREAL + 0.01;
    TrackRateN[AXIS_DE].min = -0.01;
    TrackRateN[AXIS_DE].max = 0.01;

    TrackState = SCOPE_IDLE;

    // Add Debug control so end user can turn debugging/loggin on and off
    addDebugControl();

    // Enable simulation mode so that serial connection in INDI::Telescope does not try
    // to attempt to perform a physical connection to the serial port.
    setSimulation(true); // TODO: Remove this

    // Set telescope capabilities. 0 is for the the number of slew rates that we support. We have none for this simple driver.
    // TODO: implement TELESCOPE_CAN_SYNC 
    SetTelescopeCapability(TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT | TELESCOPE_CAN_CONTROL_TRACK | TELESCOPE_HAS_TRACK_RATE, 2);

    return true;
}

bool Eqmount2::SetTrackRate(double raRate, double deRate)
{
    // Warning: Setting TrackRate while tracking is running can cause a crash
    // TODO
    // raRate	RA tracking rate in arcsecs/s
    // deRate	DEC tracking rate in arcsecs/s
    // Return true if successfull
    LOGF_INFO("Set Track rate RA: %d - DEC: %d", raRate, deRate);
    return true;
}

bool Eqmount2::SetTrackEnabled(bool enabled)
{
    //TODO TrackRateN[AXIS_RA].value and TrackRateN[AXIS_DE].value not set ? I need them to track
    if (enabled) {
        //SetTrackMode(IUFindOnSwitchIndex(&TrackModeSP));
        //if (TrackModeS[TRACK_CUSTOM].s == ISS_ON)
        SetTrackRate(TrackRateN[AXIS_RA].value, TrackRateN[AXIS_DE].value);
        LOG_INFO("Tracking on");
    }
    else
        LOG_INFO("Tracking off");
    
    return true;
}

// TODO: Write Sync(): Set the telescope current RA and DEC coordinates to the supplied RA and DEC coordinates.

/**************************************************************************************
** INDI is asking us to check communication with the device via a handshake
***************************************************************************************/
bool Eqmount2::Handshake()
{
    // When communicating with a real mount, we check here if commands are receieved
    // and acknolowedged by the mount. For Eqmount2, we simply return true.
    LOG_INFO("Handshake");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *Eqmount2::getDefaultName()
{
    return "EQmount2 - Experimental";
}

/**************************************************************************************
** Client is asking us to slew to a new position
***************************************************************************************/
bool Eqmount2::Goto(double ra, double dec)
{
    targetRA  = ra;
    targetDEC = dec;
    char RAStr[64]={0}, DecStr[64]={0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, targetRA, 2, 3600);
    fs_sexa(DecStr, targetDEC, 2, 3600);

    // Mark state as slewing
    TrackState = SCOPE_SLEWING;

    // Inform client we are slewing to a new position
    LOGF_INFO("Slewing to RA: %s - DEC: %s", RAStr, DecStr);

    // Success!
    return true;
}

/**************************************************************************************
** Client is asking us to abort our motion
***************************************************************************************/
bool Eqmount2::Abort()
{
    LOG_INFO("Abort");
    return true;
}

/**************************************************************************************
** Client is asking us to report telescope status
***************************************************************************************/
bool Eqmount2::ReadScopeStatus()
{
    static struct timeval ltv { 0, 0 };
    struct timeval tv { 0, 0 };
    double dt = 0, da_ra = 0, da_dec = 0, dx = 0, dy = 0;
    int nlocked;

    /* update elapsed time since last poll, don't presume exactly POLLMS */
    gettimeofday(&tv, nullptr);

    if (ltv.tv_sec == 0 && ltv.tv_usec == 0)
        ltv = tv;

    dt  = tv.tv_sec - ltv.tv_sec + (tv.tv_usec - ltv.tv_usec) / 1e6;
    ltv = tv;

    // Calculate how much we moved since last time
    da_ra  = SLEW_RATE * dt;
    da_dec = SLEW_RATE * dt;

    /* Process per current state. We check the state of EQUATORIAL_EOD_COORDS_REQUEST and act acoordingly */
    switch (TrackState)
    {
        case SCOPE_SLEWING:
            // Wait until we are "locked" into positon for both RA & DEC axis
            nlocked = 0;

            // Calculate diff in RA
            dx = targetRA - currentRA;

            // If diff is very small, i.e. smaller than how much we changed since last time, then we reached target RA.
            if (fabs(dx) * 15. <= da_ra)
            {
                currentRA = targetRA;
                nlocked++;
            }
            // Otherwise, increase RA
            else if (dx > 0)
                currentRA += da_ra / 15.;
            // Otherwise, decrease RA
            else
                currentRA -= da_ra / 15.;

            // Calculate diff in DEC
            dy = targetDEC - currentDEC;

            // If diff is very small, i.e. smaller than how much we changed since last time, then we reached target DEC.
            if (fabs(dy) <= da_dec)
            {
                currentDEC = targetDEC;
                nlocked++;
            }
            // Otherwise, increase DEC
            else if (dy > 0)
                currentDEC += da_dec;
            // Otherwise, decrease DEC
            else
                currentDEC -= da_dec;

            // Let's check if we recahed position for both RA/DEC
            if (nlocked == 2)
            {
                // Let's set state to TRACKING
                TrackState = SCOPE_TRACKING;

                LOG_INFO("Telescope slew is complete. Tracking...");
            }
            break;

        default:
            break;
    }

    char RAStr[64]={0}, DecStr[64]={0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, currentRA, 2, 3600);
    fs_sexa(DecStr, currentDEC, 2, 3600);

    DEBUGF(DBG_SCOPE, "Current RA: %s Current DEC: %s", RAStr, DecStr);

    NewRaDec(currentRA, currentDEC);
    return true;
}
