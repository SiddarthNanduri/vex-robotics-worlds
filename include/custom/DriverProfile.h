#ifndef DRIVERPROFILE_H_INCLUDED
#define DRIVERPROFILE_H_INCLUDED

#include "api.h"

#include <string>
#include <stdio.h>
using namespace std;

#include <cstdlib>

enum DriveMode {LEFTARCADE, TANK, SPLIT, RIGHTARCADE};

class DriverProfile
{
public:
    // Constructors
    DriverProfile();
    DriverProfile(char *, const int, double, double, double, double, DriveMode);

    // Destructor
    ~DriverProfile();

    // Accessors / Mutators
    void setName(char *n, int len) { strncpy(name, n, len); dataSaved = false; }
    const char *getName() const { return &name[0]; }
    void setEgain1(double gain) { egain1 = gain; dataSaved = false; }
    double getEgain1() const { return egain1; }
    void setEgain2(double gain) { egain2 = gain; dataSaved = false; }
    double getEgain2() const { return egain2; }
    void setRgain1(double gain) { rgain1 = gain; dataSaved = false; }
    double getRgain1() const { return rgain1; }
    void setRgain2(double gain) { rgain2 = gain; dataSaved = false; }
    double getRgain2() const { return rgain2; }
    void setDriveMode(DriveMode mode) { driveMode = mode; dataSaved = false; }
    DriveMode getDriveMode() const { return driveMode; }
    string getDriveModeStr() const;
    bool getDataSaved() const { return dataSaved; }
    int getOriginalSlot() const { return originalSlot; }

    // Static Accessors / Mutators
    static int getNumProfiles() { return numProfiles; }

    // Member Functions
    bool save(int);
    bool save();
    //bool del(int); //can't get it to work yet
    //bool del();
    bool load(int);

    // Static Member Functions
    static DriverProfile **getAllProfiles();
private:
    static string getFilename(int);
    static int numProfiles;
    char name[12];
    double egain1, egain2, rgain1, rgain2; // egains are for expo, rgains are for direct rate (some drive modes only use gain2, like TANK)
    DriveMode driveMode;
    bool dataSaved;
    int originalSlot;
};

#endif // DRIVERPROFILE_H_INCLUDED
