#include "custom/AutonData.h"

int AutonData::numObjs = 0;
int AutonData::numFilteredObjs = 0;
int AutonData::nextAvailableSlot = 0;
void (*AutonData::algs[8])() = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

// Constructors

AutonData::AutonData()
{
  strncpy(name, "No Auton   \0", 12);
  side = NEUTRALSIDE;
  pos = NEUTRALPOS;
  velDataLen = 0;
  points = 0;
  originalSlot = -1;
  visible = true;
  numObjs++;
}

AutonData::AutonData(int slot)
{
  if(!load(slot))
  {
    strncpy(name, "No Auton   \0", 12);
    side = NEUTRALSIDE;
    pos = NEUTRALPOS;
    velDataLen = 0;
    points = 0;
    originalSlot = -1;
    visible = true;
  }
  numObjs++;
}

// Destructor

AutonData::~AutonData()
{
  numObjs--;
}

// Accessors / Mutators

void AutonData::setVelData(short *data, int len)
{
  copyData(data, (short *)&velData, len);
  clearData((short *)(&velData + len), 8*600 - len);
  velDataLen = len;
}

short *AutonData::getVelData()
{
  static short *data = new short[velDataLen];
  copyData((short *)&velData, data, velDataLen);
  return data;
}

// Member Functions

void AutonData::record(pros::Motor **motors, int len, int (*callback)(), int timeLimit, int frameSize)
{
  len = (len <= 8) ? len : 8; // can't record/replay with more than 8 motors
  double time = timeLimit / 1000.0;
  int startTime;
  velDataLen = (timeLimit / static_cast<double>(frameSize))*8;
  for(int frame = 0; frame < velDataLen/8; frame++)
  {
    startTime = pros::millis();
    time -= frameSize / 1000.0;
    for(int mtr = 0; mtr < len; mtr++)
      if(motors[mtr] != nullptr)
        velData[mtr][frame] = motors[mtr]->get_actual_velocity();

    algData[frame] = (*callback)(); // callback returns an int that describes the function index called
    if(frameSize - (pros::millis() - startTime) > 0 && frameSize - (pros::millis() - startTime) <= frameSize)
      pros::delay(frameSize - (pros::millis() - startTime)); // one frame is [frameSize] ms; sleep the remaining time
  }
  for(int i = 0; i < len; i++)
    if(motors[i] != nullptr)
      motors[i]->move(0);
}

bool AutonData::save(int slot)
{
  cout << "Saving Autonomous Data into slot #" << slot << endl;
  if(slot < 0 || slot > 99)
    return false;

  /*FILE *outfile = fopen(getFilename(slot).c_str(), "wb");
  fwrite((const char *)this, sizeof(AutonData), 1, outfile);
  if(!ferror(outfile))
  {
    dataSaved = true;
    fclose(outfile);
    return true;
  }
  fclose(outfile);
  return false;*/

  ofstream outfile(getFilename(slot).c_str(), ofstream::trunc | ofstream::binary);
  if(outfile.write((char *)this, sizeof(AutonData)))
  {
    outfile.close();
    return true;
  }
  outfile.close();
  return false;
}

bool AutonData::save()
{
  return save(nextAvailableSlot);
}

bool AutonData::load(int slot)
{
  /*FILE *infile = fopen(getFilename(slot).c_str(), "rb");
  if(infile == nullptr)
  {
    fclose(infile);
    return false;
  }
  fread((char *)this, sizeof(AutonData), 1, infile);
  if(!ferror(infile))
  {
    dataSaved = false;
    originalSlot = slot;
    fclose(infile);
    return true;
  }
  fclose(infile);
  return false;*/

  ifstream infile(getFilename(slot).c_str(), ifstream::binary);
  if(infile.good())
  {
    infile.read((char *)this, sizeof(AutonData));
    infile.close();
    originalSlot = slot;
    return true;
  }
  infile.close();
  return false;
}

bool AutonData::del() // there is no deleting, there is only hiding
{
  setInvisible();
  return true;
}

void AutonData::replay(pros::Motor **motors, int len, int frameSize)
{
  len = (len <= 8) ? len : 8; // can't record/replay with more than 8 motors
  int startTime;
  for(int frame = 0; frame < velDataLen/8; frame++)
  {
    startTime = pros::millis();
    for(int mtr = 0; mtr < len; mtr++)
      if(motors[mtr] != nullptr)
        motors[mtr]->move_velocity(velData[mtr][frame]);
    if(algData[frame] != 0) // algorithm data exists for this frame
      if(algs[algData[frame]-1] != nullptr)
        (*algs[algData[frame]-1])(); // call the corresponding function registered
    if(frameSize - (pros::millis() - startTime) > 0 && frameSize - (pros::millis() - startTime) <= frameSize)
      pros::delay(frameSize - (pros::millis() - startTime)); // one frame is [frameSize] ms; sleep the remaining time
  }
  for(int i = 0; i < len; i++)
    if(motors[i] != nullptr)
      motors[i]->move(0);
}

void AutonData::invert(int *lDriveCh, int *rDriveCh, int len)
{
  len = len <= 4 ? len : 4; // can't have any more than an 8 motor drive (4 per side)
  short temp;
  for(int i = 0; i < len; i++)
  {
    for(int j = 0; j < 600; j++)
    {
      temp = velData[lDriveCh[i]][j]; // do the swippity swappity thing
      velData[lDriveCh[i]][j] = velData[rDriveCh[i]][j];
      velData[rDriveCh[i]][j] = temp; // dO tHe SwIpPiTy SwApPiTy ThInG
    }
  }
}

// Static Member Functions

AutonData **AutonData::getAllObjs()
{
  static AutonData *objs[100];
  int writeIdx = 0;
  for(int i = 0; i < 100; i++)
  {
    objs[writeIdx] = new AutonData;
    if(objs[writeIdx]->load(i))
    {
      writeIdx++;
      if(nextAvailableSlot == i)
        nextAvailableSlot++;
    }
    else
      delete objs[writeIdx];
  }
  objs[writeIdx++] = new AutonData; // give the option of a new auton
  return objs;
}

AutonData **AutonData::filter(AutonData **data, int len, Side s)
{
  static AutonData **match = new AutonData *[len];
  int writeIdx = 0;
  numFilteredObjs = 0;
  for(int i = 0; i < len; i++)
  {
    if(data[i]->getSide() == s || data[i]->getSide() == NEUTRALSIDE)
    {
      match[writeIdx++] = data[i];
      numFilteredObjs++;
    }
  }
  return match;
}

AutonData **AutonData::filter(AutonData **data, int len, Pos p)
{
    static AutonData **match = new AutonData *[len];
    int writeIdx = 0;
    numFilteredObjs = 0;
    for(int i = 0; i < len; i++)
    {
        if(data[i]->getPos() == p || data[i]->getPos() == NEUTRALPOS)
        {
            match[writeIdx++] = data[i];
            numFilteredObjs++;
        }
    }
    return match;
}

AutonData **AutonData::filter(AutonData **data, int len, Side s, Pos p)
{
    static AutonData **match = new AutonData *[len];
    int writeIdx = 0;
    numFilteredObjs = 0;
    for(int i = 0; i < len; i++)
    {
        if((data[i]->getSide() == s || data[i]->getSide() == NEUTRALSIDE) && (data[i]->getPos() == p || data[i]->getPos() == NEUTRALPOS))
        {
            match[writeIdx++] = data[i];
            numFilteredObjs++;
        }
    }
    return match;
}

void AutonData::algRegister(void (*callback)(), int regSlt)
{
  if(regSlt > 0 && regSlt <= 8) // 0 is registered for ALG_NONE
    algs[regSlt - 1] = callback;
}

// Private Member Functions

string AutonData::getFilename(int slot)
{
    char num[3];
    itoa(slot, num, 10);
    string filename = "/usd/data-";
    filename.append(num);
    filename.append(".dat");
    return filename;
}

void AutonData::copyData(char *src, char *dest, int len)
{
    for(int i = 0; i < len; i++)
        dest[i] = src[i];
}

void AutonData::copyData(short *src, short *dest, int len)
{
  for(int i = 0; i < len; i++)
      dest[i] = src[i];
}

void AutonData::clearData(char *arr, int len)
{
    for(int i = 0; i < len; i++)
        arr[i] = 0;
}

void AutonData::clearData(short *arr, int len)
{
    for(int i = 0; i < len; i++)
        arr[i] = 0;
}
