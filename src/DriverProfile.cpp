#include "custom/DriverProfile.h"

int DriverProfile::numProfiles = 0;

// Constructors

DriverProfile::DriverProfile()
{
  strncpy(name, "Default     ", 12);
  rgain1 = 0.6;
  rgain2 = 1.0;
  egain1 = 1.6;
  egain2 = 1.1;
  driveMode = RIGHTARCADE;
  dataSaved = false;
  numProfiles++;
  originalSlot = -1;
}

DriverProfile::DriverProfile(char *n, const int len, double r1, double r2, double e1, double e2, DriveMode mode)
{
    setName(n, 12);
    rgain1 = r1;
    rgain2 = r2;
    egain1 = e1;
    egain2 = e2;
    driveMode = mode;
    dataSaved = false;
    numProfiles++;
    originalSlot = -1;
}

// Destructor

DriverProfile::~DriverProfile()
{
    numProfiles--;
}

// Accessors / Mutators

string DriverProfile::getDriveModeStr() const
{
    if(driveMode == LEFTARCADE)
        return "Left Arcade";
    else if(driveMode == TANK)
        return "Tank";
    else if(driveMode == SPLIT)
        return "Split";
    else if(driveMode == RIGHTARCADE)
        return "Right Arcade";
    else
        return "Drive Mode not Set";
}

// Member Functions

bool DriverProfile::save(int slot)
{
    if(slot < 0 || slot > 99)
        return false;

    FILE *outfile = fopen(getFilename(slot).c_str(), "wb");
    fwrite((const char *)this, sizeof(DriverProfile), 1, outfile);
    if(!ferror(outfile))
    {
      dataSaved = true;
      fclose(outfile);
      return true;
    }
    fclose(outfile);
    return false;
}

bool DriverProfile::save()
{
    int slot = originalSlot; // save back into original slot
    if(slot == -1) // if this is a new profile,
        slot = numProfiles; // save into the next available slot
    return save(slot);
}

/*bool DriverProfile::del(int slot)
{
  return remove("/usd/test.txt");
}

bool DriverProfile::del()
{
  if(originalSlot != -1)
    return del(originalSlot);
  return false;
}*/

bool DriverProfile::load(int slot)
{
  FILE *infile = fopen(getFilename(slot).c_str(), "rb");
  if(infile == nullptr)
  {
    fclose(infile);
    return false;
  }
  fread((char *)this, sizeof(DriverProfile), 1, infile);
  if(!ferror(infile))
  {
    dataSaved = false;
    originalSlot = slot;
    fclose(infile);
    return true;
  }
  fclose(infile);
  return false;
}

// Static Member Functions

DriverProfile **DriverProfile::getAllProfiles()
{
    static DriverProfile *profiles[100];
    int writeIdx = 0;
    for(int i = 0; i < 100; i++)
    {
        profiles[writeIdx] = new DriverProfile;
        if(profiles[writeIdx]->load(i))
            writeIdx++;
        else
            delete profiles[writeIdx];
    }
    profiles[writeIdx++] = new DriverProfile; // give the option of creating a new profile
    return profiles;
}

string DriverProfile::getFilename(int s)
{
    char num[3];
    itoa(s, num, 10);
    string filename = "/usd/profile-";
    filename.append(num);
    filename.append(".dat");
    return filename;
}
