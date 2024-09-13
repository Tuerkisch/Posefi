#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

// Default window layout --------------------------------------------------------------------
#define MAINWND_WIDTH   1300
#define MAINWND_HEIGHT  700

#define COL_NAME_WIDTH      180
#define COL_ANGLE_WIDTH     80
#define COL_POS_X_WIDTH     80
#define COL_POS_Z_WIDTH     COL_POS_X_WIDTH
#define COL_AMO_MIN_WIDTH   80
#define COL_AMO_MAX_WIDTH   80
#define COL_COST_WIDTH      90
#define COL_TOLERANCE_WIDTH 45
#define COL_COMPARE_PORTION_WIDTH   COL_POS_X_WIDTH
#define COL_MAX_COST_WIDTH  80

#define SCROLLBAR_SIZE              20
#define SPLITTER_HANDLE_SIZE        17
#define TABLE_HEADER_HEIGHT         23

#define LEFT_ELEMENT_ACTION_WIDTH   60
#define SPLIT_AREA_TOP_HEIGHT       500
#define ACTION_TABLE_HEIGHT         SPLIT_AREA_TOP_HEIGHT - TABLE_HEADER_HEIGHT - 2

#define LEFT_ELEMENT_WANTED_POS_WIDTH   LEFT_ELEMENT_ACTION_WIDTH
#define SPLIT_AREA_BOTTOM_HEIGHT        MAINWND_HEIGHT - SPLIT_AREA_TOP_HEIGHT - SPLITTER_HANDLE_HEIGHT
#define WANTED_POS_TABLE_HEIGHT         SPLIT_AREA_BOTTOM_HEIGHT - 2

#define ACTION_TABLE_WIDTH              this->ui->actionTable->width()
#define SCROLL_AREA_ACTIONS_WIDTH       this->ui->scrollAreaActions->width()
#define ACTION_TABLE_VISIBLE_WIDTH      (this->ui->splitterLeftTables->width() - LEFT_ELEMENT_ACTION_WIDTH - SCROLLBAR_SIZE)
#define WANTED_POS_TABLE_WIDTH          this->ui->wantedPosTable->width()
#define SCROLL_AREA_WANTED_POS_WIDTH    this->ui->scrollAreaWantedPos->width()
#define WANTED_TABLE_VISIBLE_WIDTH      (this->ui->splitterLeftTables->width() - LEFT_ELEMENT_WANTED_POS_WIDTH - SCROLLBAR_SIZE)

#define GAP_BOTTOM  10
#define GAP_RIGHT   GAP_BOTTOM

// -----------------------------------------------------------------
// Table content stuff -----------------------------------------------------

#define TEXT_ALIGNMENT  Qt::AlignCenter

#define FREE_LENGTH     -1
enum StringCases{NAME, HEX, DEC, UDEC};
enum XYZ_Cases{IS_NOT_XYZ,IS_XYZ_FIRST,IS_XYZ_SECOND};  // used to change whole columns to grey when x/y/z is deactivated

#define FIND_FIRST_LABELS           {"find x","find x","find y"}
#define FIND_SECOND_LABELS          {"find y","find z","find z"}
#define RESETTABLE_FIRST_LABELS     {"x is resettable","x is resettable","y is resettable"}
#define RESETTABLE_SECOND_LABELS    {"y is resettable","z is resettable","z is resettable"}
#define DEFAULT_POS_FIRST_LABELS    {"x:","x:","y:"}
#define DEFAULT_POS_SECOND_LABELS   {"y:","z:","z:"}

#define LOAD_COLUMN_DATA \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Name of Action","Name of Action","Name of Action"},NAME,IS_NOT_XYZ,FREE_LENGTH,"action_",COL_NAME_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"X","X","Y"},HEX,IS_XYZ_FIRST,8,"0",COL_POS_X_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Y","Z","Z"},HEX,IS_XYZ_SECOND,8,"0",COL_POS_Z_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Angle","Angle","Angle"},HEX,IS_NOT_XYZ,4,"0",COL_ANGLE_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Min","Min","Min"},DEC,IS_NOT_XYZ,FREE_LENGTH,"",COL_AMO_MIN_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Max","Max","Max"},DEC,IS_NOT_XYZ,FREE_LENGTH,"",COL_AMO_MAX_WIDTH)); \
this->ActionColumnData.push_back(ColumnData(new QString[3]{"Cost","Cost","Cost"},DEC,IS_NOT_XYZ,FREE_LENGTH,"",COL_COST_WIDTH)); \
    \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"Name of Searched","Name of Searched","Name of Searched"},NAME,IS_NOT_XYZ,FREE_LENGTH,"wanted_pos_",COL_NAME_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"X","X","Y"},HEX,IS_XYZ_FIRST,8,"0",COL_POS_X_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"Y","Z","Z"},HEX,IS_XYZ_SECOND,8,"0",COL_POS_Z_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"± X", "± X", "± Y"},DEC,IS_XYZ_FIRST,FREE_LENGTH,"",COL_TOLERANCE_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"± Y", "± Z", "± Z"},DEC,IS_XYZ_SECOND,FREE_LENGTH,"",COL_TOLERANCE_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"& X","& X","& Y"},HEX,IS_XYZ_FIRST,8,"F",COL_COMPARE_PORTION_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"& Y","& Z","& Z"},HEX,IS_XYZ_SECOND,8,"F",COL_COMPARE_PORTION_WIDTH)); \
this->WantedColumnData.push_back(ColumnData(new QString[3]{"Max Cost","Max Cost","Max Cost"},DEC,IS_NOT_XYZ,FREE_LENGTH,"",COL_MAX_COST_WIDTH));

enum ACTION_COLUMN_ORDER {ACTION_NAME, ACTION_POS1, ACTION_POS2, ACTION_ANGLE, ACTION_MIN, ACTION_MAX, ACTION_COST};
enum WANTED_COLUMN_ORDER {WANTED_NAME, WANTED_POS1, WANTED_POS2, WANTED_TOL1, WANTED_TOL2, WANTED_AND1, WANTED_AND2, WANTED_MAXCOST};

// -----------------------------------------------------------------
// Left side element of the two left tables -----------------------------------------------------
#define ADD_ACTION_BUTTON_WIDTH             40
#define ADD_ACTION_BUTTON_HEIGHT            40
#define ADD_ACTION_BUTTON_X_POS             10
#define ADD_ACTION_BUTTON_Y_POS             10
#define SPACE_FOR_ADD_ACTION_BUTTON         60
#define ADD_ACTION_BUTTON_ANIMATION_LENGTH  300
#define REMOVE_BUTTON_ANIMATION_LENGTH      300
#define CHECK_BOX_ANIMATION_LENGTH          300

#define ADD_WANTED_POS_BUTTON_WIDTH             ADD_ACTION_BUTTON_WIDTH
#define ADD_WANTED_POS_BUTTON_HEIGHT            ADD_ACTION_BUTTON_HEIGHT
#define ADD_WANTED_POS_BUTTON_X_POS             ADD_ACTION_BUTTON_X_POS
#define ADD_WANTED_POS_BUTTON_Y_POS             ADD_ACTION_BUTTON_Y_POS
#define SPACE_FOR_ADD_WANTED_POS_BUTTON         SPACE_FOR_ADD_ACTION_BUTTON
#define ADD_WANTED_POS_BUTTON_ANIMATION_LENGTH  ADD_ACTION_BUTTON_ANIMATION_LENGTH
#define WANTED_POS_REMOVE_BUTTON_ANIMATION_LENGTH   REMOVE_BUTTON_ANIMATION_LENGTH
#define WANTED_POS_CHECK_BOX_ANIMATION_LENGTH   CHECK_BOX_ANIMATION_LENGTH

// Left side element layout
#define REMOVE_BUTTON_WIDTH     20
#define REMOVE_BUTTON_HEIGHT    20
#define REMOVE_BUTTON_X_POS     5
#define USED_CHECKBOX_WIDTH     20
#define USED_CHECKBOX_HEIGHT    20
#define USED_CHECKBOX_X_POS     40

// -----------------------------------------------------------------
// Right side elements layout -------------------------------------------------
#define RS_MAIN_FRAME_WIDTH                     (MAINWND_WIDTH - this->ui->splitterLeftTables->width() - SPLITTER_HANDLE_SIZE)
#define RS_SCROLL_AREA_MOVE                     1   // move the four sub-frames on the right a little bit away from splitterRight
#define RS_HEADER_LABEL_HEIGHT                  12
#define RS_HEADER_LABEL_ADDED_WIDTH             15  // width of the label is set with adjustSize(), then this width is added
#define RS_HEADER_LABEL_XY                      10,2
#define PAGE_NAVIGATION_HEIGHT                  25
#define TOP_RIGHT_FRAME_GEOMETRY                0,0,520,160
#define SUB_SEARCH_FRAME_GEOMETRY               0,0,480,160
#define FULL_SOLUTIONS_FRAME_GEOMETRY           0,0,480,160
#define RADIOBUTTONS_CHECKBOXES_FRAME_GEOMETRY  0, 0, 290, 80
#define DEFAULT_POS_FRAME_GEOMETRY              290,0,250,80
#define START_PAUSE_CANCEL_FRAME_GEOMETRY       0,80,270,40
#define MAIN_SEARCH_PROGRESSBAR_GEOMETRY        270, 90, 200, 20
#define MAIN_SEARCH_PROGRESS_LABEL_GEOMETRY     270, 120, 200, 20

// Progress label layout (eg. variables set to (1000,1,"K") would do this: 1234567 -> 1234.5K)
#define PROGRESS_LABEL_DIVISOR          1000000    //
#define PROGRESS_LABEL_DECIMALS         0
#define PROGRESS_LABEL_SUFFIX           " M"
// to prevent the window from freezing:
#define ITERATIONS_PER_SEARCH_CYCLE     0x10000  // do this amount of iterations at once, then return control to the window

//#define PAUSE_BUTTON_ENABLED_CSS    "border-image:url(style/pause_button_dark.png)"
//#define PAUSE_BUTTON_DISABLED_CSS   "border-image:url(style/pause_button_dark_disabled.png)"
//#define PAUSE_BUTTON_CONTINUE_CSS   "border-image:url(style/play_button_dark.png)"

#define DEFAULT_POS_1ST_LINE_EDIT_GEOMETRY    30,30,65,23
#define DEFAULT_POS_2ND_LINE_EDIT_GEOMETRY    130,30,65,23

// SE = solution element ------------------------------------------
#define SOLUTIONS_PER_PAGE          10 // goes for both, partial and full solutions

#define SE_HEADER_HEIGHT            30
#define SE_FOLDED_HEIGHT            0 + SE_HEADER_HEIGHT
#define SE_TOP_FRAME_GAP            4
//#define PSE_FOLD_BUTTON_GEOMETRY    4,4,22,22
#define PSE_DELETE_BUTTON_SIZE      22,22
#define PSE_DELETE_BUTTON_GAP_RIGHT 4
#define PSE_DELETE_BUTTON_Y         4
#define PSE_HEADER_LABEL_GEOMETRY   65,2,300,26
#define SE_FOLD_BUTTON_GAP_RIGHT    30
#define SE_FOLD_BUTTON_CSS         "QPushButton { background-color: #00000000; border-radius: 0px; border: none}\
                                    QPushButton:hover { background-color: #40E0E0FF;}"

#define PSE_BOTTOM_FRAME_GAP            4
#define SE_SOLUTION_LABEL_MOVE         15,15
#define SE_BOTTOM_ADDITIONAL_HEIGHT     20
#define PSE_SUB_SEARCH_BUTTON_GEOMETRY  330,10,140,28
#define PSE_SUB_SEARCH_BUTTON_TEXT      "Start Sub-Search"
#define PSE_BOTTOM_FRAME_CSS            "QFrame { background-color: #FF858585 }"

// the labels that show x/y/z always have the same position but differ in size. The found position will have the position
// added to the label. The line-edit can have two different positions, depending on which position is still searched.
#define PSE_FIRST_LABELS                {"x","x","y"}
#define PSE_SECOND_LABELS               {"y","z","z"}
#define PSE_1ST_POS_LABEL_POSITION      10,10
#define PSE_2ND_POS_LABEL_POSITION      100,10
#define PSE_LINE_EDIT_POSITION_IF_1ST   28,10
#define PSE_LINE_EDIT_POSITION_IF_2ND   118,10
#define PSE_POS_LABEL_SIZE_OF_FOUND     80,23
#define PSE_POS_LABEL_SIZE_OF_SEARCHED  15,23
#define PSE_LINE_EDIT_SIZE              65,23

// animation variables
#define PSE_FROM_RIGHT_ANIMATION_X1         100
#define PSE_TO_RIGHT_ANIMATION_X2           100
#define PSE_FROM_RIGHT_ANIMATION_DURATION   300 // element moving from right to left when it appears
#define PSE_REMOVE_ANIMATION_DURATION       300 // element moving from left to right when it disappears
#define PSE_UP_DOWN_ANIMATION_DURATION      300 // elements moving up or down when an element was removed or folded/unfolded
#define PSE_SCROLL_AREA_RESIZE_TIMER        10  // the scroll area updates its height whenever the timer runs out
#define PSE_PAGE_CHANGE_FADEOUT_DURATION    200
#define PSE_PAGE_CHANGE_FADEIN_DURATION     200

// strings
#define SE_HEADER_SEPARATOR_STRING      "  |  "
#define SE_HEADER_FOUND_STRING          "found "
#define SE_HEADER_COST_STRING           "cost: "
#define SE_ACTION_PREFIX_STRING         ""
//#define SE_ACTION_NAME_RIGHT_ALIGNED  // if not defined, the action name will be left aligned
#define SE_DEFAULT_POS_STRING           "default:    "
#define SE_SEARCHED_POS_STRING          "searched:   "
#define SE_RESULT_POS_STRING            "result:     "
#define SE_COLON_STRING                 ": "
#define SE_SPACE_STRING                 "    "  // space between first and second coordinate
#define SE_ACTION_ANGLE_STRING          "    angle "
#define SE_ACTION_AMOUNT_STRING         "    amount "
#define SE_ZERO_POS_STRING              "0       "  // if a coordinate is 0, replace it with this string in the solution
#define SE_RESET_POS_STRING_1           "\n --- reset " // between the two reset strings comes x/y/z
#define SE_RESET_POS_STRING_2           " ---\n"
#define SE_ACTION_NAME_LENGTH           18
#define SE_ACTION_ANGLE_LENGTH          4
#define PSE_FROM_TO_STRING              "-"
#define PSE_OF_STRING                   " of "

// page navigation
#define PSE_PAGE_SUB_FRAME_SIZE     150,25
#define PSE_PAGE_EDIT_SIZE          40,20
#define PSE_NAVI_BUTTON_SIZE        20,20
#define PSE_SHOW_LABEL_SIZE         100,25
#define PSE_SHOW_LABEL_X            10
#define PSE_NAVI_BUTTON_GAP         3       // gap between buttons
#define PSE_NAVI_EDIT_GAP           10      // gap between the buttons and the line edit


// FSE = full solution element ------------------------------------------
//#define FSE_HEADER_HEIGHT           PSE_HEADER_HEIGHT
//#define FSE_FOLDED_HEIGHT           PSE_FOLDED_HEIGHT
//#define FSE_TOP_FRAME_GAP           PSE_TOP_FRAME_GAP
//#define FSE_FOLD_BUTTON_GEOMETRY    PSE_FOLD_BUTTON_GEOMETRY
#define FSE_DELETE_BUTTON_SIZE      PSE_DELETE_BUTTON_SIZE
#define FSE_DELETE_BUTTON_GAP_RIGHT PSE_DELETE_BUTTON_GAP_RIGHT
#define FSE_DELETE_BUTTON_Y         PSE_DELETE_BUTTON_Y
#define FSE_HEADER_LABEL_GEOMETRY   PSE_HEADER_LABEL_GEOMETRY

#define FSE_BOTTOM_FRAME_GAP            PSE_BOTTOM_FRAME_GAP
//#define FSE_SOLUTION_LABEL_MOVE         10,10
//#define FSE_BOTTOM_ADDITIONAL_HEIGHT    10
#define FSE_BOTTOM_FRAME_CSS            PSE_BOTTOM_FRAME_CSS

// animation variables
#define FSE_FROM_RIGHT_ANIMATION_X1         PSE_FROM_RIGHT_ANIMATION_X1
#define FSE_TO_RIGHT_ANIMATION_X2           PSE_TO_RIGHT_ANIMATION_X2
#define FSE_FROM_RIGHT_ANIMATION_DURATION   PSE_FROM_RIGHT_ANIMATION_DURATION // element moving from right to left when it appears
#define FSE_REMOVE_ANIMATION_DURATION       PSE_REMOVE_ANIMATION_DURATION // element moving from left to right when it disappears
#define FSE_UP_DOWN_ANIMATION_DURATION      PSE_UP_DOWN_ANIMATION_DURATION // elements moving up or down when an element was removed or folded/unfolded
#define FSE_SCROLL_AREA_RESIZE_TIMER        PSE_SCROLL_AREA_RESIZE_TIMER  // the scroll area updates its height whenever the timer runs out
#define FSE_PAGE_CHANGE_FADEOUT_DURATION    PSE_PAGE_CHANGE_FADEOUT_DURATION
#define FSE_PAGE_CHANGE_FADEIN_DURATION     PSE_PAGE_CHANGE_FADEIN_DURATION

// strings
//#define FSE_HEADER_SEPARATOR_STRING     PSE_HEADER_SEPARATOR_STRING
//#define FSE_HEADER_COST_STRING          PSE_HEADER_COST_STRING
//#define FSE_ACTION_PREFIX_STRING        PSE_ACTION_PREFIX_STRING
//#define FSE_FOUND_STRING                "found "
//#define FSE_COLON_STRING                ": "
//#define FSE_SPACE_STRING                "    "  // space between first and second found position
//#define FSE_ACTION_ANGLE_STRING         PSE_ACTION_ANGLE_STRING
//#define FSE_ACTION_AMOUNT_STRING        PSE_ACTION_AMOUNT_STRING
//#define FSE_ACTION_NAME_LENGTH          PSE_ACTION_NAME_LENGTH
//#define FSE_ACTION_ANGLE_LENGTH         PSE_ACTION_ANGLE_LENGTH
#define FSE_FROM_TO_STRING              PSE_FROM_TO_STRING
#define FSE_OF_STRING                   PSE_OF_STRING

// page navigation
#define FSE_PAGE_SUB_FRAME_SIZE     PSE_PAGE_SUB_FRAME_SIZE
#define FSE_PAGE_EDIT_SIZE          PSE_PAGE_EDIT_SIZE
#define FSE_NAVI_BUTTON_SIZE        PSE_NAVI_BUTTON_SIZE
#define FSE_SHOW_LABEL_SIZE         PSE_SHOW_LABEL_SIZE
#define FSE_SHOW_LABEL_X            PSE_SHOW_LABEL_X
#define FSE_NAVI_BUTTON_GAP         PSE_NAVI_BUTTON_GAP     // gap between buttons
#define FSE_NAVI_EDIT_GAP           PSE_NAVI_EDIT_GAP       // gap between thr buttons and the line edit



// -------------------------------------------------------------
// colors ------------------------------------------------
#define DARK_THEME_CSS          "style/dark_theme.css"
#define LIGHT_THEME_CSS         "style/light_theme.css"
#define TABLE_DARK              0xFF808080
#define TABLE_DARK_ERROR        0xFFD08080
#define TABLE_DARK_GREY         0xFF606060
#define TABLE_DARK_ERROR_GREY   0xFF906060
#define TABLE_LIGHT             0xFFF0F0F0
#define TABLE_LIGHT_ERROR       0xFFF0A0A0
#define TABLE_LIGHT_GREY        0xFFAAAAAA
#define TABLE_LIGHT_ERROR_GREY  0xFFDAAAAA
//#define TABLE_CORRECT_PROP      "correct"
//#define TABLE_USED_PROP         "used"



enum ColorTheme{
    DARK_THEME,
    LIGHT_THEME
};

// return values ------------------------------------------
enum ReturnStatements{
    SUCCESS = 0,
    ARRAY_OUT_OF_RANGE = 0x0001,
    FILE_NOT_OPEN = 0x0002,
    TOGGLE_UNCHANGED = 0x0004,
    FUNCTION_DEACTIVATED = 0x0008,
    INT_OVERFLOW = 0x0010,
    ARRAY_SIZE_ZERO = 0x0020,
    NO_POSITION_SEARCHED = 0x0040,
    NULL_POINTER = 0x0080,
    COSTS_TOO_HIGH = 0x0100,
    SEARCH_DONE = 0x0200,
    FOUND_FULL_SOLUTION = 0x0400,
    FOUND_PARTIAL_SOLUTION = 0x0800,
    ARRAY_AT_MAX = 0x1000,
    IMPOSSIBLE_PAGE_CHANGE = 0x2000
};

// ----------------------------------------------------
// stuff related to position data
#define NULL_VALUE  0xFFFFFFFF          // store this as position data if the position is unused

// -------------------------------------
// stuff related to running the search:
enum SolutionTypes{FULL_SOLUTION, PARTIAL_SOLUTION};






#endif // CONSTANTS_H
