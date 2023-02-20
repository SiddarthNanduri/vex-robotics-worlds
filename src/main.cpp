#include "main.h"

// Globals

pros::Motor LDrive(1, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor RDrive(6, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor Stack(2, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor Lift0(18, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor Lift1(17, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor Intake0(7, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor Intake1(8, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor *motors[] = { &LDrive,
											   	&RDrive,
										     	&Stack,
									       	&Lift0,
												 	&Lift1,
													&Intake0,
													&Intake1,
												 	nullptr };

pros::ADIButton StackBtn('A');
pros::ADIButton LiftBtn('B');

pros::Controller Cont(pros::E_CONTROLLER_MASTER);

AutonData *autonData;
DriverProfile *currentDriver;

DriverProfile **profiles;
int numProfiles;
AutonData **objs;
int numObjs;
AutonData **filteredObjs;
int numFilteredObjs;

Side side = RED;
Pos pos = LEFT;

lv_obj_t *driverList;
lv_obj_t *saveBtn;
lv_obj_t *delBtn;
lv_obj_t *driveModeList;
lv_obj_t *autonList;

bool breakDriveLoop = false;
bool suspendOpcontrol = false;
bool useAuton = true;

int (*baseFrame)();

string getDriverOptions(DriverProfile **, int);
string getAutonOptions(AutonData **, int);

static lv_res_t ddlist_action(lv_obj_t *);
static lv_res_t saveDriver(lv_obj_t *);
static lv_res_t side_switch_action(lv_obj_t *);
static lv_res_t pos_switch_action(lv_obj_t *);
static lv_res_t copyAuton(lv_obj_t *);
static lv_res_t recAutonMenu(lv_obj_t *);
static lv_res_t delAutonMenu(lv_obj_t *);
static lv_res_t recAuton(lv_obj_t *, const char *);
static lv_res_t delAuton(lv_obj_t *, const char *);
static lv_res_t postAuton(lv_obj_t *, const char *);
static lv_res_t saveAutonData(lv_obj_t *);
static lv_res_t pointsRollerAct(lv_obj_t *);
static lv_res_t autonEnabledAct(lv_obj_t *);
static lv_res_t invertAuton(lv_obj_t *);

void saveAuton();

void alg_assert_release();
void alg_release();
void alg_tarestack();

void pp(Side, Pos);

int drexpo(int, double, double);
int driverCtlBaseFrameRArc();
int driverCtlBaseFrameLArc();
int driverCtlBaseFrameSplit();
int driverCtlBaseFrameCommon();

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize()
{
	cout << "Robot Online" << endl << "64W Competition Program Rev. " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << VERSION_INFO << "-" << VERSION_YEAR << " with ";
	if (__cplusplus == 201703L) std::cout << "C++17\n";
  else if (__cplusplus == 201402L) std::cout << "C++14\n";
  else if (__cplusplus == 201103L) std::cout << "C++11\n";
  else if (__cplusplus == 199711L) std::cout << "C++98\n";
  else std::cout << "pre-standard C++\n";
	cout << "Begin Initialization" << endl;

	cout << "Resetting Lift Position... ";
	tareLift(&Lift0, &Lift1, &LiftBtn, LIFTT);
	cout << "done" << endl;

	cout << "Resetting Ramp Position... ";
	tareStack(&Stack, &StackBtn, STACKT);
	cout << "done" << endl;

	cout << "Setting motor brakemodes... ";
	Intake0.set_brake_mode(MOTOR_BRAKE_HOLD);
	Intake1.set_brake_mode(MOTOR_BRAKE_HOLD);
	Stack.set_brake_mode(MOTOR_BRAKE_BRAKE);
	cout << "done" << endl;

	cout << "Registering AutonData algorithms... ";
  AutonData::algRegister(alg_assert_release, ALG_ASSERT_RELEASE);
  AutonData::algRegister(alg_release, ALG_RELEASE);
  AutonData::algRegister(alg_tarestack, ALG_TARESTACK);
	cout << "done" << endl;

	// write autonomous selecting code here
	cout << "Reading driver profiles and autonomous data from sd card... ";
	profiles = DriverProfile::getAllProfiles();
	numProfiles = DriverProfile::getNumProfiles();
	objs = AutonData::getAllObjs();
  numObjs = AutonData::getNumObjs(); // must call getNumObjs AFTER all objects are loaded
	filteredObjs = AutonData::filter(objs, numObjs, side, pos);
	numFilteredObjs = AutonData::getNumFilteredObjs();
	currentDriver = profiles[0];
	autonData = filteredObjs[0];
	cout << "Found " << numProfiles << " driver profiles and " << numObjs << " data objects" << endl;

	cout << "Initializing GUI... ";
	lv_obj_t *tabview = lv_tabview_create(lv_scr_act(), NULL);


	lv_obj_t *autonSetup = lv_tabview_add_tab(tabview, "Auton. Setup");

	lv_obj_t *title4 = lv_label_create(autonSetup, NULL);
	lv_label_set_text(title4, "Select Field Position");
	lv_obj_align(title4, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 0);

	static lv_style_t side_bg_style;
	static lv_style_t side_indic_style;
	static lv_style_t side_knob_on_style;
	static lv_style_t side_knob_off_style;
	lv_style_copy(&side_bg_style, &lv_style_pretty);
	side_bg_style.body.radius = LV_RADIUS_CIRCLE;
	side_bg_style.body.main_color = LV_COLOR_HEX(0xef9f9f);
	side_bg_style.body.grad_color = LV_COLOR_HEX(0xef9f9f);
	lv_style_copy(&side_indic_style, &lv_style_pretty_color);
	side_indic_style.body.radius = LV_RADIUS_CIRCLE;
	side_indic_style.body.main_color = LV_COLOR_HEX(0x9fc8ef);
	side_indic_style.body.grad_color = LV_COLOR_HEX(0x9fc8ef);
	side_indic_style.body.padding.hor = 0;
	side_indic_style.body.padding.ver = 0;
	lv_style_copy(&side_knob_off_style, &lv_style_pretty_color);
	side_knob_off_style.body.radius = LV_RADIUS_CIRCLE;
	side_knob_off_style.body.shadow.width = 4;
	side_knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;
	side_knob_off_style.body.main_color = LV_COLOR_RED;
	side_knob_off_style.body.grad_color = LV_COLOR_RED;
	lv_style_copy(&side_knob_on_style, &lv_style_pretty_color);
	side_knob_on_style.body.radius = LV_RADIUS_CIRCLE;
	side_knob_on_style.body.shadow.width = 4;
	side_knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;
	side_knob_on_style.body.main_color = LV_COLOR_BLUE;
	side_knob_on_style.body.grad_color = LV_COLOR_BLUE;

	lv_obj_t *sw1 = lv_sw_create(autonSetup, NULL);
	lv_sw_set_style(sw1, LV_SW_STYLE_BG, &side_bg_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &side_indic_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_ON, &side_knob_on_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_OFF, &side_knob_off_style);
	lv_obj_align(sw1, title4, LV_ALIGN_CENTER, -50, 45);
	lv_sw_set_action(sw1, side_switch_action);

	static lv_style_t pos_bg_style;
	static lv_style_t pos_indic_style;
	static lv_style_t pos_knob_on_style;
	static lv_style_t pos_knob_off_style;
	lv_style_copy(&pos_bg_style, &lv_style_pretty);
	pos_bg_style.body.radius = LV_RADIUS_CIRCLE;
	lv_style_copy(&pos_indic_style, &lv_style_pretty);
	pos_indic_style.body.radius = LV_RADIUS_CIRCLE;
	pos_indic_style.body.padding.hor = 0;
	pos_indic_style.body.padding.ver = 0;
	lv_style_copy(&pos_knob_off_style, &lv_style_pretty);
	pos_knob_off_style.body.radius = LV_RADIUS_CIRCLE;
	pos_knob_off_style.body.shadow.width = 4;
	pos_knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;
	lv_style_copy(&pos_knob_on_style, &lv_style_pretty);
	pos_knob_on_style.body.radius = LV_RADIUS_CIRCLE;
	pos_knob_on_style.body.shadow.width = 4;
	pos_knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;

	lv_obj_t *sw2 = lv_sw_create(autonSetup, NULL);
	lv_sw_set_style(sw2, LV_SW_STYLE_BG, &pos_bg_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_INDIC, &pos_indic_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_KNOB_ON, &pos_knob_on_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_KNOB_OFF, &pos_knob_off_style);
	lv_obj_align(sw2, sw1, LV_ALIGN_CENTER, 0, 50);
	lv_sw_set_action(sw2, pos_switch_action);

	/*lv_obj_t *copyBtn = lv_btn_create(autonSetup, NULL);
	lv_obj_align(copyBtn, title4, LV_ALIGN_CENTER, 50, 45);
	lv_cont_set_fit(copyBtn, true, true);
	lv_btn_set_action(copyBtn, LV_BTN_ACTION_CLICK, copyAuton);
	lv_obj_t *copyBtnLabel = lv_label_create(copyBtn, NULL);
	lv_label_set_text(copyBtnLabel, "Copy");*/

	lv_obj_t *recBtn = lv_btn_create(autonSetup, NULL);
	lv_obj_align(recBtn, title4, LV_ALIGN_CENTER, 50, 45);
	lv_cont_set_fit(recBtn, true, true);
	lv_btn_set_action(recBtn, LV_BTN_ACTION_CLICK, recAutonMenu);
	lv_obj_t *recBtnLabel = lv_label_create(recBtn, NULL);
	lv_label_set_text(recBtnLabel, "Record");

	lv_obj_t *delBtn = lv_btn_create(autonSetup, NULL);
	lv_obj_align(delBtn, recBtn, LV_ALIGN_CENTER, 0, 70);
	lv_cont_set_fit(delBtn, true, true);
	lv_btn_set_action(delBtn, LV_BTN_ACTION_CLICK, delAutonMenu);
	lv_obj_t *delBtnLabel = lv_label_create(delBtn, NULL);
	lv_label_set_text(delBtnLabel, "Delete");

	lv_obj_t *title3 = lv_label_create(autonSetup, NULL);
	lv_label_set_text(title3, "Select an Auton.");
	lv_obj_align(title3, NULL, LV_ALIGN_IN_TOP_MID, 100, 20);

	static lv_style_t style_bg;
	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.shadow.width = 4;
	style_bg.text.color = LV_COLOR_MAKE(0x10, 0x20, 0x50);

	autonList = lv_ddlist_create(autonSetup, NULL);
	lv_obj_set_free_num(autonList, 2);
	lv_obj_set_style(autonList, &style_bg);
	lv_ddlist_set_options(autonList, getAutonOptions(filteredObjs, numFilteredObjs).c_str());
	lv_obj_align(autonList, title3, LV_ALIGN_CENTER, 0, 30);
	lv_ddlist_set_action(autonList, ddlist_action);

	lv_obj_t *autonEnabled = lv_cb_create(autonSetup, NULL);
	lv_obj_align(autonEnabled, autonList, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_cb_set_action(autonEnabled, autonEnabledAct);
	lv_cb_set_checked(autonEnabled, true);
	lv_cb_set_text(autonEnabled, "");


	lv_obj_t *driverSetup = lv_tabview_add_tab(tabview, "Driver Setup");

	lv_obj_t *title1 = lv_label_create(driverSetup, NULL);
	lv_label_set_text(title1, "Who's Driving?");
	lv_obj_align(title1, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 0);

	driverList = lv_ddlist_create(driverSetup, NULL);
	lv_obj_set_free_num(driverList, 1);
	lv_obj_set_style(driverList, &style_bg);
	lv_ddlist_set_options(driverList, getDriverOptions(profiles, numProfiles).c_str());
	lv_obj_align(driverList, title1, LV_ALIGN_CENTER, 0, 30);
	lv_ddlist_set_action(driverList, ddlist_action);

	saveBtn = lv_btn_create(driverSetup, NULL);
	lv_cont_set_fit(saveBtn, true, true);
	lv_obj_align(saveBtn, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
	lv_btn_set_action(saveBtn, LV_BTN_ACTION_CLICK, saveDriver);
	if(currentDriver->getDataSaved())
		lv_btn_set_state(saveBtn, LV_BTN_STATE_INA);
	lv_obj_t *saveBtnLabel = lv_label_create(saveBtn, NULL);
	lv_label_set_text(saveBtnLabel, "Save");

	lv_obj_t *title2 = lv_label_create(driverSetup, NULL);
	lv_label_set_text(title2, "Drive Mode");
	lv_obj_align(title2, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);

	driveModeList = lv_ddlist_create(driverSetup, NULL);
	lv_obj_set_free_num(driveModeList, 3);
	lv_obj_set_style(driveModeList, &style_bg);
	lv_ddlist_set_options(driveModeList, "Left Arcade\nTank\nSplit\nRight Arcade");
	lv_ddlist_set_selected(driveModeList, static_cast<int>(currentDriver->getDriveMode()));
	lv_obj_align(driveModeList, title2, LV_ALIGN_CENTER, 0, 30);
	lv_ddlist_set_action(driveModeList, ddlist_action);


	lv_obj_t *advanced = lv_tabview_add_tab(tabview, "Advanced");

	lv_obj_t *joystickTuningLabel = lv_label_create(advanced, NULL);
	lv_label_set_text(joystickTuningLabel, "Joystick Tuning");
	lv_obj_align(joystickTuningLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 0);

	lv_obj_t *r2Label = lv_label_create(advanced, NULL);
	lv_obj_t *e2Label = lv_label_create(advanced, NULL);
	lv_obj_t *r1Label = lv_label_create(advanced, NULL);
	lv_obj_t *e1Label = lv_label_create(advanced, NULL);
	lv_label_set_text(r2Label, "r2:");
	lv_label_set_text(e2Label, "e2:");
	lv_label_set_text(r1Label, "r1:");
	lv_label_set_text(e1Label, "e1:");
	lv_obj_align(r2Label, joystickTuningLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_align(e2Label, r2Label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_align(r1Label, e2Label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_align(e1Label, r1Label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

	lv_obj_t *r2 = lv_slider_create(advanced, NULL);
	lv_obj_t *e2 = lv_slider_create(advanced, NULL);
	lv_obj_t *r1 = lv_slider_create(advanced, NULL);
	lv_obj_t *e1 = lv_slider_create(advanced, NULL);
	lv_obj_align(r2, r2Label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_obj_align(e2, e2Label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_obj_align(r1, r1Label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_obj_align(e1, e1Label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);

	/*lv_obj_t *sensorValuesLabel = lv_label_create(advanced, NULL);
	lv_label_set_text(sensorValuesLabel, "Sensor Values");
	lv_obj_align(sensorValuesLabel, e1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

	lv_obj_t *catapultPotLabel = lv_label_create(advanced, NULL);
	lv_obj_t *sonarLabel = lv_label_create(advanced, NULL);
	stringstream buf;
	buf << "Catapult Pot: " << CatapultPot.get_value();
	lv_label_set_text(catapultPotLabel, buf.str().c_str());
	buf.clear();
	buf << "Sonar: " << Sonar.get_value() << " cm (" << Sonar.get_value() / 2.54 << " in)";
	lv_label_set_text(sonarLabel, buf.str().c_str());*/
	cout << "done" << endl;
	cout << "Initialization Complete" << endl;
}

string getDriverOptions(DriverProfile **profs, int len)
{
	string options;
	if(profs != nullptr)
	{
		for(int i = 0; i < len; i++)
		{
			if(profs[i] != nullptr)
			{
				options.append(profs[i]->getName());
				options.append("\n");
			}
		}
	}
	options.pop_back(); // pop the last \n off
	return options;
}

string getAutonOptions(AutonData **a, int len)
{
	string autonOptions;
	if(a != nullptr)
	{
		char points[3];
		for(int i = 0; i < len; i++)
		{
			if(a[i] != nullptr /*&& a[i]->getVisible()*/)
			{
				autonOptions.append(a[i]->getName());
				autonOptions.append(": ");
				itoa(a[i]->getPoints(), points, 10);
				autonOptions.append(points);
				autonOptions.append("\n");
			}
		}
	}
	autonOptions.append("Preprog");
	return autonOptions;
}

static lv_res_t ddlist_action(lv_obj_t *ddlist)
{
	if(lv_obj_get_free_num(ddlist) == 1)
	{
		currentDriver = profiles[lv_ddlist_get_selected(ddlist)];
		if(currentDriver->getDataSaved())
			lv_btn_set_state(saveBtn, LV_BTN_STATE_INA);
		else
			lv_btn_set_state(saveBtn, LV_BTN_STATE_REL);
		lv_ddlist_set_selected(driveModeList, static_cast<int>(currentDriver->getDriveMode()));
		breakDriveLoop = true;
	}
	else if(lv_obj_get_free_num(ddlist) == 2)
	{
		if(lv_ddlist_get_selected(ddlist) < numFilteredObjs)
			autonData = filteredObjs[lv_ddlist_get_selected(ddlist)];
		else
			autonData = nullptr;
	}
	else if(lv_obj_get_free_num(ddlist) == 3)
	{
		currentDriver->setDriveMode(static_cast<DriveMode>(lv_ddlist_get_selected(ddlist)));
		breakDriveLoop = true;
	}

	return LV_RES_OK;
}

static lv_res_t saveDriver(lv_obj_t *btn)
{
	if(currentDriver->save())
		lv_btn_set_state(saveBtn, LV_BTN_STATE_INA);

	return LV_RES_OK;
}

static lv_res_t side_switch_action(lv_obj_t *sw)
{
	if(side == RED)
	{
		side = BLUE;
		lv_sw_on(sw);
	}
	else if(side == BLUE)
	{
		side = RED;
		lv_sw_off(sw);
	}

	filteredObjs = AutonData::filter(objs, numObjs, side, pos);
	numFilteredObjs = AutonData::getNumFilteredObjs();
	lv_ddlist_set_options(autonList, getAutonOptions(filteredObjs, numFilteredObjs).c_str());
	if(filteredObjs != nullptr)
		autonData = filteredObjs[0];

	return LV_RES_OK;
}

static lv_res_t pos_switch_action(lv_obj_t *sw)
{
	if(pos == LEFT)
	{
		pos = RIGHT;
		lv_sw_on(sw);
	}
	else if(pos == RIGHT)
	{
		pos = LEFT;
		lv_sw_off(sw);
	}

	filteredObjs = AutonData::filter(objs, numObjs, side, pos);
	numFilteredObjs = AutonData::getNumFilteredObjs();
	lv_ddlist_set_options(autonList, getAutonOptions(filteredObjs, numFilteredObjs).c_str());
	if(filteredObjs != nullptr)
		autonData = filteredObjs[0];

	return LV_RES_OK;
}

static lv_res_t copyAuton(lv_obj_t *btn)
{
	saveAuton(); // copying is essentially re-saving

	return LV_RES_OK;
}

static lv_res_t recAutonMenu(lv_obj_t *btn)
{
	suspendOpcontrol = true;
	breakDriveLoop = true;
	lv_obj_t *mBox = lv_mbox_create(lv_scr_act(), NULL);
	lv_mbox_set_text(mBox, "Record an Autonomous?\n(A when ready)");
	static const char *btns[] = {"Match", "Skills", "Cancel", ""};
	lv_mbox_add_btns(mBox, btns, NULL);
	lv_mbox_set_action(mBox, recAuton);
	lv_obj_set_width(mBox, 250);
	lv_obj_align(mBox, NULL, LV_ALIGN_CENTER, 0, 0);

	return LV_RES_OK;
}

static lv_res_t delAutonMenu(lv_obj_t *btn)
{
	lv_obj_t *mBox = lv_mbox_create(lv_scr_act(), NULL);
	lv_mbox_set_text(mBox, "Delete Recording\nAre you sure?\n(you can't undo this unless you ask Jacob)");
	static const char *btns[] = {"Confirm", "Cancel", ""};
	lv_mbox_add_btns(mBox, btns, NULL);
	lv_mbox_set_action(mBox, delAuton);
	lv_obj_set_width(mBox, 250);
	lv_obj_align(mBox, NULL, LV_ALIGN_CENTER, 0, 0);

	return LV_RES_OK;
}

static lv_res_t recAuton(lv_obj_t *btnmtx, const char *txt)
{
	lv_obj_t * mBox = lv_mbox_get_from_btn(btnmtx);
	if(strncmp(txt, "Cancel", 6) == 0)
	{
		lv_obj_del(mBox);
		return LV_RES_INV;
	}

	int ms = 0;
	if(strncmp(txt, "Match", 5) == 0)
		ms = 15000;
	else if(strncmp(txt, "Skills", 6) == 0)
		ms = 60000;

	// Record an autonomous
	if(autonData != nullptr)
		delete autonData;
	autonData = new AutonData;
	waitUntil(Cont.get_digital(DIGITAL_A));
	waitUntil(!Cont.get_digital(DIGITAL_A));
	Cont.rumble("-");
	pros::delay(1000);
	int startTime = pros::millis();
	deploy(&Lift0, &Lift1, &Stack);
  tareLift(&Lift0, &Lift1, &LiftBtn, 500);
  tareStack(&Stack, &StackBtn, 500);
	autonData->record(motors, 8, baseFrame, ms - (pros::millis() - startTime)); // The above deploy algorithm takes time.  Compensate with less time in auton.
	lv_mbox_set_text(mBox, "Record an Autonomous\nRecording Complete");
	static const char *opt[] = {"Save", "Discard", ""};
	lv_mbox_add_btns(mBox, opt, NULL);
	lv_mbox_set_action(mBox, postAuton);

	return LV_RES_OK;
}

static lv_res_t delAuton(lv_obj_t *btnmtx, const char *txt)
{
	if(strncmp(txt, "Confirm", 7) == 0)
	{
		autonData->del();
		delete autonData;
	}

	lv_obj_t * mBox = lv_mbox_get_from_btn(btnmtx);
	lv_obj_del(mBox);

	return LV_RES_OK;
}

static lv_res_t postAuton(lv_obj_t *btnmtx, const char *txt)
{
	if(strncmp(txt, "Save", 4) == 0)
		saveAuton();

	lv_obj_t * mBox = lv_mbox_get_from_btn(btnmtx);
	lv_obj_del(mBox);
	suspendOpcontrol = false;

	return LV_RES_OK;
}

lv_obj_t *tempPage;

bool autonInv = false;

static lv_res_t saveAutonData(lv_obj_t *btn)
{
	lv_obj_del(tempPage);
	autonData->setPos(pos);
	autonData->setSide(side);
	if(autonInv)
	{
		int lDrives[] = {0};
		int rDrives[] = {1};
		autonData->invert(lDrives, rDrives, 1);
	}
	autonData->save(); // It's literally that simple

	return LV_RES_INV;
}

static lv_res_t pointsRollerAct(lv_obj_t *roll)
{
	autonData->setPoints(lv_roller_get_selected(roll));

	return LV_RES_OK;
}

static lv_res_t autonEnabledAct(lv_obj_t *cb)
{
	useAuton = lv_cb_is_checked(cb);

	return LV_RES_OK;
}

static lv_res_t invertAuton(lv_obj_t *cb)
{
	autonInv = lv_cb_is_checked(cb);

	return LV_RES_OK;
}

void saveAuton()
{
	char name[] = "Recordable\0";
	autonData->setName(name, 11);

	tempPage = lv_page_create(lv_scr_act(), NULL);
	lv_obj_set_pos(tempPage, 0, 0);
	lv_obj_set_size(tempPage, 480, 240);

	lv_obj_t *label = lv_label_create(tempPage, NULL);
	lv_label_set_text(label, "Points");
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -50);

	lv_obj_t *roller = lv_roller_create(tempPage, NULL);
	lv_roller_set_options(roller, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20");
	lv_roller_set_visible_row_count(roller, 3);
	lv_roller_set_action(roller, pointsRollerAct);
	lv_obj_align(roller, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

	lv_obj_t *title4 = lv_label_create(tempPage, NULL);
	lv_label_set_text(title4, "Field Position");
	lv_obj_align(title4, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 0);

	static lv_style_t style_bg;
	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.shadow.width = 4;
	style_bg.text.color = LV_COLOR_MAKE(0x10, 0x20, 0x50);

	static lv_style_t side_bg_style;
	static lv_style_t side_indic_style;
	static lv_style_t side_knob_on_style;
	static lv_style_t side_knob_off_style;
	lv_style_copy(&side_bg_style, &lv_style_pretty);
	side_bg_style.body.radius = LV_RADIUS_CIRCLE;
	side_bg_style.body.main_color = LV_COLOR_HEX(0xef9f9f);
	side_bg_style.body.grad_color = LV_COLOR_HEX(0xef9f9f);
	lv_style_copy(&side_indic_style, &lv_style_pretty_color);
	side_indic_style.body.radius = LV_RADIUS_CIRCLE;
	side_indic_style.body.main_color = LV_COLOR_HEX(0x9fc8ef);
	side_indic_style.body.grad_color = LV_COLOR_HEX(0x9fc8ef);
	side_indic_style.body.padding.hor = 0;
	side_indic_style.body.padding.ver = 0;
	lv_style_copy(&side_knob_off_style, &lv_style_pretty_color);
	side_knob_off_style.body.radius = LV_RADIUS_CIRCLE;
	side_knob_off_style.body.shadow.width = 4;
	side_knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;
	side_knob_off_style.body.main_color = LV_COLOR_RED;
	side_knob_off_style.body.grad_color = LV_COLOR_RED;
	lv_style_copy(&side_knob_on_style, &lv_style_pretty_color);
	side_knob_on_style.body.radius = LV_RADIUS_CIRCLE;
	side_knob_on_style.body.shadow.width = 4;
	side_knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;
	side_knob_on_style.body.main_color = LV_COLOR_BLUE;
	side_knob_on_style.body.grad_color = LV_COLOR_BLUE;

	lv_obj_t *sw1 = lv_sw_create(tempPage, NULL);
	lv_sw_set_style(sw1, LV_SW_STYLE_BG, &side_bg_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &side_indic_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_ON, &side_knob_on_style);
	lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_OFF, &side_knob_off_style);
	lv_obj_align(sw1, title4, LV_ALIGN_CENTER, 0, 45);
	lv_sw_set_action(sw1, side_switch_action);
	if(side == BLUE)
		lv_sw_on(sw1);

	static lv_style_t pos_bg_style;
	static lv_style_t pos_indic_style;
	static lv_style_t pos_knob_on_style;
	static lv_style_t pos_knob_off_style;
	lv_style_copy(&pos_bg_style, &lv_style_pretty);
	pos_bg_style.body.radius = LV_RADIUS_CIRCLE;
	lv_style_copy(&pos_indic_style, &lv_style_pretty);
	pos_indic_style.body.radius = LV_RADIUS_CIRCLE;
	pos_indic_style.body.padding.hor = 0;
	pos_indic_style.body.padding.ver = 0;
	lv_style_copy(&pos_knob_off_style, &lv_style_pretty);
	pos_knob_off_style.body.radius = LV_RADIUS_CIRCLE;
	pos_knob_off_style.body.shadow.width = 4;
	pos_knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;
	lv_style_copy(&pos_knob_on_style, &lv_style_pretty);
	pos_knob_on_style.body.radius = LV_RADIUS_CIRCLE;
	pos_knob_on_style.body.shadow.width = 4;
	pos_knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;

	lv_obj_t *sw2 = lv_sw_create(tempPage, NULL);
	lv_sw_set_style(sw2, LV_SW_STYLE_BG, &pos_bg_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_INDIC, &pos_indic_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_KNOB_ON, &pos_knob_on_style);
	lv_sw_set_style(sw2, LV_SW_STYLE_KNOB_OFF, &pos_knob_off_style);
	lv_obj_align(sw2, sw1, LV_ALIGN_CENTER, 0, 50);
	lv_sw_set_action(sw2, pos_switch_action);
	if(pos == RIGHT)
		lv_sw_on(sw2);

	lv_obj_t *saveBtn = lv_btn_create(tempPage, NULL);
	lv_obj_align(saveBtn, sw2, LV_ALIGN_CENTER, 0, 70);
	lv_cont_set_fit(saveBtn, true, true);
	lv_btn_set_action(saveBtn, LV_BTN_ACTION_CLICK, saveAutonData);
	lv_obj_t *saveBtnLabel = lv_label_create(saveBtn, NULL);
	lv_label_set_text(saveBtnLabel, "Save");

	lv_obj_t *nameLabel = lv_label_create(tempPage, NULL);
	lv_label_set_text(nameLabel, "Name");
	lv_obj_align(nameLabel, NULL, LV_ALIGN_IN_TOP_RIGHT, -70, 0);

	//lv_obj_t *kb = lv_kb_create(lv_scr_act(), NULL); // Keyboard support (hopefully) is coming soon!
	//lv_kb_set_cursor_manage(kb, true);
	lv_obj_t *nameTextArea = lv_ta_create(tempPage, NULL);
	lv_ta_set_one_line(nameTextArea, true);
	lv_obj_set_width(nameTextArea, 150);
	lv_ta_set_text(nameTextArea, "Recordable");
	lv_obj_align(nameTextArea, nameLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
	//lv_kb_set_ta(kb, nameTextArea);

	lv_obj_t *invertCb = lv_cb_create(tempPage, NULL);
	lv_obj_align(invertCb, nameTextArea, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
	lv_cb_set_text(invertCb, "Invert");
	lv_cb_set_action(invertCb, invertAuton);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

// The slightly more important stuff starts here. vvv

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous()
{
  cout << endl << "Entering autonomous" << endl;
  deploy(&Lift0, &Lift1, &Stack);
  tareLift(&Lift0, &Lift1, &LiftBtn, 500);
  tareStack(&Stack, &StackBtn, 500);
  if(autonData != nullptr && useAuton)
    autonData->replay(motors, 8); // for recordable auton
  else if(useAuton)
    pp(side, pos);
}

void pp(Side s, Pos p)
{
  if(s == RED && p == LEFT)
  {

  }
  else if(s == RED && p == RIGHT)
  {

  }
  else if(s == BLUE && p == LEFT)
  {

  }
  else if(s == BLUE && p == RIGHT)
  {

  }
}

// The truly important stuff starts here. vvv

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol()
{
  cout << endl << "Entering opcontrol" << endl;
  while(true)
  {
    waitUntil(!suspendOpcontrol);
    switch(currentDriver->getDriveMode())
    {
    case RIGHTARCADE:
      baseFrame = &driverCtlBaseFrameRArc;
      break;
    case LEFTARCADE:
      baseFrame = &driverCtlBaseFrameLArc;
      break;
    case SPLIT:
      baseFrame = &driverCtlBaseFrameSplit;
      break;
    default:
      baseFrame = &driverCtlBaseFrameRArc;
    }
    while(!breakDriveLoop)
    {
      (*baseFrame)(); // call baseframe driverctl function
      pros::delay(5); // don't hog cpu
    }
    cout << "Driveloop broken" << endl;
    breakDriveLoop = false;
    pros::delay(5);
  }
}

void alg_assert_release()
{
  assertStack(&Stack, &Intake0, &Intake1);
  pros::delay(50); // TODO: tune this value
  releaseStack(&LDrive, &RDrive, &Intake0, &Intake1);
  tareLift(&Lift0, &Lift1, &LiftBtn, LIFTT);
  tareStack(&Stack, &StackBtn, STACKT);
}

void alg_release()
{
  releaseStack(&LDrive, &RDrive, &Intake0, &Intake1); // this routine can also double for releasing cubes placed in towers
}

void alg_tarestack()
{
  tareStack(&Stack, &StackBtn, STACKT);
}

int drexpo(int input, double rgain, double egain) // just a quick expo equation for the joystick controls
{
    return sgn(input)*rgain*pow(100, 1-egain)*pow(abs(input), egain); // see Desmos for a pretty graph of this
}

int driverCtlBaseFrameRArc()
{
  int joy_x = Cont.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
  int joy_y = Cont.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

  LDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) + drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));
  RDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) - drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));

  return driverCtlBaseFrameCommon();
}

int driverCtlBaseFrameLArc()
{
  int joy_x = Cont.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X);
  int joy_y = Cont.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);

  LDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) + drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));
  RDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) - drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));

  return driverCtlBaseFrameCommon();
}

int driverCtlBaseFrameSplit()
{
  int joy_x = Cont.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X);
  int joy_y = Cont.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

  LDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) + drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));
  RDrive.move(drexpo(joy_y, currentDriver->getRgain2(), currentDriver->getEgain2()) - drexpo(joy_x, currentDriver->getRgain1(), currentDriver->getEgain1()));

  return driverCtlBaseFrameCommon();
}

int driverCtlBaseFrameCommon()
{
  int intakeSpeed = 130;
  int liftSpeed = 80;
  int stackSpeed = 127; // this is only when automatically moving in conjunction with the lift motors
  // Buttons here (remember to return ALG_* if calling a registered algorithm)
  // Lift
  bool aneurysm = false; // this determines whether or not the lift is having an aneurysm. it doesn't work.
  if(Cont.get_digital(DIGITAL_R1))
  {
    if(Stack.get_position() < 160*5)
    {
      Stack.move(stackSpeed);
      Lift0.move(liftSpeed/2);
      Lift1.move(liftSpeed/2);
    }
    else
    {
      Stack.move(0);
      Lift0.move(liftSpeed);
      Lift1.move(liftSpeed);
    }
    // This is a workaround so you can outtake cubes while forcing the lift up
    if(Cont.get_digital(DIGITAL_L2))
    {
      Intake0.move(-127);
      Intake1.move(-127);
    }
    else
    {
      Intake0.move(0);
      Intake1.move(0);
    }
  }
  else if(Cont.get_digital(DIGITAL_R2) && !LiftBtn.get_value() && !aneurysm)
  {
    if(Lift0.get_position() < 25*5)
    {
      Lift0.move(-liftSpeed/2);
      Lift1.move(-liftSpeed/2);
    }
    else
    {
      Lift0.move(-liftSpeed);
      Lift1.move(-liftSpeed);
    }
    if(Stack.get_position() < 160*5 && Lift0.get_position() >= 270)
      Stack.move(stackSpeed);
    else if(!StackBtn.get_value() && Lift0.get_position() < 270)
      Stack.move(-stackSpeed);
    else
      Stack.move(0);
  }
  else
  {
    Lift0.move(0);
    Lift1.move(0);
    aneurysm = Cont.get_digital(DIGITAL_R2); // if the user is still holding the down button, that means the above else if statement was avoided by LiftBtn.  Which means aneurysm.

    // Default Stack motor movement
    int joy_s = Cont.get_analog(ANALOG_LEFT_Y);
    int threshold = 5;
    if(joy_s > threshold && Stack.get_position() <= 290*5)
      Stack.move(joy_s);
    else if(joy_s < -threshold && !StackBtn.get_value())
      Stack.move(joy_s);
    else
    {
      Stack.move(0);

      // Default intake movement (this inception is giving me a headache)
      if(Cont.get_digital(DIGITAL_L1))
      {
        Intake0.move_velocity(intakeSpeed);
        Intake1.move_velocity(intakeSpeed);
      }
      else if(Cont.get_digital(DIGITAL_L2))
      {
        Intake0.move_velocity(-intakeSpeed);
        Intake1.move_velocity(-intakeSpeed);
      }
      else
      {
        Intake0.move(0);
        Intake1.move(0);
      }
    }
  }

  if(Cont.get_digital(DIGITAL_B))
  {
    waitUntil(!Cont.get_digital(DIGITAL_B));
    alg_tarestack();
    return ALG_TARESTACK;
  }

  if(Cont.get_digital(DIGITAL_A))
  {
    waitUntil(!Cont.get_digital(DIGITAL_A));
    alg_release();
    return ALG_RELEASE;
  }

  if(Cont.get_digital(DIGITAL_Y))
  {
    waitUntil(!Cont.get_digital(DIGITAL_Y));
    alg_assert_release();
    return ALG_ASSERT_RELEASE;
  }
  return ALG_NONE;
}
