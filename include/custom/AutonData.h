#ifndef AUTONDATA_H_INCLUDED
#define AUTONDATA_H_INCLUDED

#include <string>
#include <string.h>
#include <fstream>
using namespace std;

#include <cstdlib>

#include "api.h"

enum Side {RED, BLUE, NEUTRALSIDE};
enum Pos {LEFT, RIGHT, NEUTRALPOS};

class AutonData
{
public:
    // Constructors
    AutonData();
    AutonData(int); // load from specified slot

    // Destructor
    ~AutonData();

    // Accessors / Mutators
    void setName(char *n, int len) { strncpy(name, n, len); }
    const char *getName() const { return &name[0]; }
    void setSide(Side s) { side = s; }
    Side getSide() const { return side; }
    void setPos(Pos p) { pos = p; }
    Pos getPos() const { return pos; }
    void setPoints(int p) { if(p < 200) points = p; }
    int getPoints() const { return points; }
    void setVelData(short *, int);
    short *getVelData();
    int getVelDataLen() const { return velDataLen; }
    int getOriginalSlot() const { return originalSlot; }
    void setVisible() { visible = true; }
    void setInvisible() { visible = false; }
    bool getVisible() const { return visible; }

    // Static Accessors / Mutators
    static int getNumObjs() { return numObjs; }
    static int getNumFilteredObjs() { return numFilteredObjs; }
    static int getNextAvailableSlot() { return nextAvailableSlot; }

    // Member Functions
    void record(pros::Motor **, int, int (*)(), int, int = 100); // Note: This function WILL overwrite any existing PWM data!  Use carefully!
    bool save(int);
    bool save();
    bool load(int);
    bool del();
    void replay(pros::Motor **, int, int = 100);
    void invert(int *, int *, int);

    // Static Member Functions
    static AutonData **getAllObjs();
    static AutonData **filter(AutonData **, int, Side);
    static AutonData **filter(AutonData **, int, Pos);
    static AutonData **filter(AutonData **, int, Side, Pos);
    static AutonData **filter(AutonData **data, int len, Pos p, Side s) { return filter(data, len, s, p); }
    static void algRegister(void (*)(), int);
private:
    static string getFilename(int);
    static void copyData(char *, char *, int);
    static void copyData(short *, short *, int);
    static void clearData(char *, int);
    static void clearData(short *, int);
    static int numObjs;
    static int numFilteredObjs;
    static int nextAvailableSlot;
    static void (*algs[8])(); // array of pointers to 8 callback functions for use within autonomous recordings
    char name[12];
    Side side;
    Pos pos;
    int points;
    short velData[8][600]; // 8 channels by 600 frames (for match auton, 150 are used; for skills, up to 600).
    short algData[600]; // algorithm channel (call functions within an autonomous recording)
    int velDataLen; // records length of velData that is actually used; getPwmData will return 'trimmed' version
    bool visible;
    int originalSlot;
};

#endif // AUTONDATA_H_INCLUDED
