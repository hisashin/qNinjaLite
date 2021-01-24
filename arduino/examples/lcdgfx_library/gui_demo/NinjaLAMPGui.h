class NinjaLAMPGui {
  public:
    NinjaLAMPGui();
    void setup();         // Called from NinjaLAMP_Arduino setup 
    int loop(int state);  // Call from NinjaLAMP_Arduino loop when command = COMMAND_MENU
  private:
    int old_state = 0;
    // Side switches, pin number depends on orientation written in EEPROM, reset in setup
    int pinMoveUp = 10;   // move up
    int pinOk = 9;        // enter
    int pinMoveDown = 8;  // move down
    void initSetupMenu();
    void initRunMenu();
    int selectedMenuItem;
    void initCyclePage(int selectedMenuItem);
    void initRunPage(int selectedMenuItem);
};
