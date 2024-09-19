#ifndef POSEFI_H
#define POSEFI_H

// BUGS:
// toggled off actions (and maybe searched) are not actually disabled

#include <QWidget>
#include <QMainWindow>
#include <QSignalMapper>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include "Constants.h"
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsOpacityEffect>
#include <QEvent>
#include "finder.h"

//#define LEFT_SIDE_ACTION_TABLE_ANIMATION
//#define LEFT_SIDE_WANTED_TABLE_ANIMATION

struct LeftSideElement;
struct ColumnData;
struct PartialSolutionElement;
struct SolutionElement;
struct SolutionCopy;
class AutoSelectLineEdit;
//class SeachEvent;
//class SolutionsEdit;


static QEvent::Type MainSearchType = static_cast<QEvent::Type>(QEvent::User + 1);
static QEvent::Type SubSearchType = static_cast<QEvent::Type>(QEvent::User + 2);


namespace Ui {
class Posefi;
}

class Posefi : public QMainWindow
{
    Q_OBJECT

public:
    explicit Posefi(QMainWindow *parent = nullptr);
    ~Posefi();

private slots:

    // move splitters
    int AdjustLeftRightSplitter();
    int AdjustLeftTableSplitter();
    int AdjustRightSideSplitter();

    // menubar
    int OpenFile();
    int SaveFile();
    int SaveInputs(); // Save all data inserted by the user, but not the progression of a search
    void StreamInputsToFile(QTextStream& stream);
    int LoadInputs();
    void StreamInputsFromFile(QTextStream& stream);

    // Top left scroll area
    int AddAction();
    int AddActionButtonAnimation();
    int RemoveActionButton_press(int index);
    int ActionCheckBox_press(int index);
    int ActionTableCell_edited(int row, int col);
    int ResizeActionTableHeader();
    int TopScrollArea_horizontalScroll(int val);
    int UpdateActionTableCellColor(int row, int col);

    // Bottom left scroll area
    int AddWantedPos();
    int AddWantedButtonAnimation();
    int RemoveWantedButton_press(int index);
    int WantedCheckBox_press(int index);
    int WantedTableCell_edited(int row, int col);
    int ResizeWantedTableHeader();
    int BottomScrollArea_horizontalScroll(int val);
    int UpdateWantedTableCellColor(int row, int col);

    // Top right radio buttons and checkboxes
    int FindXZ_RadioButton_toggle(int id);
    int LabelXYZ_RadioButton_toggle(int id);
    int DefaultFirstLineEdit_change(const QString& s);
    int DefaultSecondLineEdit_change(const QString& s);

    // Partial solutions
    int PartialSolutionFoldButton_press(int element_index);
    int PartialSolutionDeleteButton_press(int element_index);
    int PartialSolutionSubSearchButton_press(int element_index);
    int PSE_GeometryAnimationEnd(int element_index);
    int PSE_OpacityAnimationEnd(int element_index);
    int AdjustPartialSolutionScrollAreaHeight();
    int PartialSolutionFirstPageButton_press();
    int PartialSolutionPrevPageButton_press();
    int PartialSolutionNextPageButton_press();
    int PartialSolutionLastPageButton_press();
    int PartialSolutionPageEdit_change();
    int PartialSolutionEnableSubSearchButtons();
    int PartialSolutionDisableSubSearchButtons();

    // Full solutions
    int FullSolutionFoldButton_press(int element_index);
    int FullSolutionDeleteButton_press(int element_index);
    int FSE_GeometryAnimationEnd(int element_index);
    int FSE_OpacityAnimationEnd(int element_index);
    int AdjustFullSolutionScrollAreaHeight();
    int FullSolutionFirstPageButton_press();
    int FullSolutionPrevPageButton_press();
    int FullSolutionNextPageButton_press();
    int FullSolutionLastPageButton_press();
    int FullSolutionPageEdit_change();

    // Sub search stuff
    int InitiateSubSearch(SolutionCopy* ps);

    // Running the search
    int StartButton_press();
    int PauseButton_press();
    int CancelButton_press();
    int RunningMainSearch();

    // Edit solutions
    void deleteAllPartialSolutions();
    void deleteAllFullSolutions();
    void sortPartialSolutionsByCost();
    void sortFullSolutionsByCost();
    void sortPartialSolutionsByAngleCount();
    void sortFullSolutionsByAngleCount();
    void sortPartialSolutionsByActionCount();
    void sortFullSolutionsByActionCount();

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

private:
    Ui::Posefi *ui;

    int LoadWindowDefaultLayout();
    int LinkSignals();
    int SetColorTheme();
    int LoadColorThemes();

    // load and save settings
    void LoadSettings();
    void SaveSettings();

    // Top left scroll area
    int AddLeftSideElementActionTable();
    int ArrangeLeftSideActionTable();
    int ResizeActionTableHeight();
    int ArrangeActionColumnWidths();

    // Bottom left scroll area
    int AddLeftSideElementWantedTable();
    int ArrangeLeftSideWantedTable();
    int ResizeWantedTableHeight();
    int ArrangeWantedColumnWidths();

    // Right side
    int ResizeSplitterRightWidth();
    int ResizeSplitterRightHeight();
    void UpdateEditButtonGeometry();

    // Partial Solutions
    int ShowNewPartialSolutions();
    int CreatePartialSolutionElement(SolutionCopy* sc);
    int AddPartialSolution(SolutionCopy* sc);
    int CalculatePartialSolutionElement_YPos(uint index);
    int ResizePartialSolutionsWidth();
    int ManagePartialSolutionPages();
    int PartialSolutionChangePage(int page);
    int PSE_FadeOutAnimation(PartialSolutionElement* pse, uint duration);
    int PSE_FadeInAnimation(PartialSolutionElement* pse, uint duration);
    int PSE_AppearFromRightAnimation(PartialSolutionElement* pse);
    int PSE_DisappearToRightAnimation(PartialSolutionElement* pse);
    int PSE_PageChangeFadeInAnimation(PartialSolutionElement* pse);
    int Disable_PSE(PartialSolutionElement* pse);

    // Full Solutions
    int ShowNewFullSolutions();
    int CreateFullSolutionElement(SolutionCopy* sc);
    int AddFullSolution(SolutionCopy* sc);
    int CalculateFullSolutionElement_YPos(uint index);
    int ResizeFullSolutionsWidth();
    int ManageFullSolutionPages();
    int FullSolutionChangePage(int page);
    int FSE_FadeOutAnimation(SolutionElement* fse, uint duration);
    int FSE_FadeInAnimation(SolutionElement* fse, uint duration);
    int FSE_AppearFromRightAnimation(SolutionElement* fse);
    int FSE_DisappearToRightAnimation(SolutionElement* fse);
    int FSE_PageChangeFadeInAnimation(SolutionElement* fse);
    int Disable_FSE(SolutionElement* fse);

    void AddOrReplaceSolution(Search& search, std::vector<SolutionCopy>& copies, int solutionType, std::vector<uint>& freeSlots, std::vector<int>& solutionCopyIndices, int solutionIndex);

    // Preparing and running the search
    int SetStartButtonState();
    int TableAutoCompletion();
    int ValidDataCheck();
    int CreateMainSearch();
    int FinishMainSearch();

    // Edit solutions
    void deleteAllSolutions(SolutionTypes type);

    // Debug functions
    int PrintSearch(Search& s);
    int PrintMainSearch();
    int PrintSubSearch();
    int PrintSolution(int solution_type, Search& search, uint index);

// -------------------------------------
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent* e) override;
// -------------------------------------

    AutoSelectLineEdit* partialSolutionPageEdit;
    AutoSelectLineEdit* fullSolutionPageEdit;
    AutoSelectLineEdit* defaultFirstLineEdit;
    AutoSelectLineEdit* defaultSecondLineEdit;


// -------------------------------------

    bool find_xyz_first = true;
    bool find_xyz_second = true;

    bool is_first_default_pos_valid = true;
    bool is_second_default_pos_valid = true;

    bool main_search_is_running = false;
    bool main_search_is_paused = false;
    bool sub_search_is_running = false;
    bool sub_search_is_paused = false;
    QTimer main_search_timer;
    QTimer sub_search_timer;


    uint main_search_partial_solutions_printed;
    uint main_search_full_solutions_printed;

    QPropertyAnimation add_action_button_animation;
    QPropertyAnimation add_wanted_button_animation;

    QMenu *fileMenu;
    QMenu *editMenu;
    QAction* saveInputsAction;
    QAction* loadInputsAction;

    QButtonGroup xyzLable_radioButtons;
    QButtonGroup findXYZ_radioButtons;

    // Label string groups (for switching between xy, xz and yz)
    std::vector<QStringList> action_table_lables;
    std::vector<QStringList> wanted_table_lables;
    QString* find_first_labels;
    QString* find_second_labels;
    QString* resettable_first_labels;
    QString* resettable_second_labels;
    QString* default_pos_first_labels;
    QString* default_pos_second_labels;

    QString* solution_first_pos_labels;
    QString* solution_second_pos_labels;

    // Color Theme
    int color_theme = DARK_THEME;
    QString dark_theme_css;
    QString light_theme_css;
    QBrush table_error_color;
    QBrush table_normal_color;
    QBrush table_grey_color;
    QBrush table_error_grey_color;
    QBrush text_error_color;
    QBrush text_normal_color;
    QBrush text_grey_color;
    QBrush text_error_grey_color;

    // left side Table Data
    std::vector<ColumnData> ActionColumnData;
    std::vector<ColumnData> WantedColumnData;
    std::vector<LeftSideElement*> LeftSideActionTable;
    std::vector<LeftSideElement*> LeftSideWantedTable;

    // Partial Solution pages and elements
    std::vector<SolutionCopy> partial_solution_copies;
    std::vector<uint> partial_solution_copies_free_slots;
    QTimer pse_scroll_resize_timer;
    std::vector<PartialSolutionElement*> PartialSolutionElements;
    std::vector<int> PSE_Order;
    std::vector<int> ps_indices;
    int current_ps_page = -1; // -1 = no pages
    int current_ps_disable_animations = 0; // when changing a page, this keeps track of how many pse are still in fade out
    bool upcoming_ps_page_change = false;
    //SolutionsEdit* partialSolutionsEdit = nullptr;

    // Full Solution pages and elements
    std::vector<SolutionCopy> full_solution_copies;
    std::vector<uint> full_solution_copies_free_slots;
    QTimer fse_scroll_resize_timer;
    std::vector<SolutionElement*> FullSolutionElements;
    std::vector<int> FSE_Order;
    std::vector<int> fs_indices;
    int current_fs_page = -1; // -1 = no pages
    int current_fs_disable_animations = 0; // when changing a page, this keeps track of how many fse are still in fade out
    bool upcoming_fs_page_change = false;
    //SolutionsEdit* fullSolutionsEdit = nullptr;

    // Search
    Search main_search;
    Search sub_search;
    uint solutions_per_page = SOLUTIONS_PER_PAGE;

    // Save Data
    QString lastDirectory;
};

class AutoSelectLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit AutoSelectLineEdit(QWidget* parent = nullptr);
    ~AutoSelectLineEdit();
    void focusInEvent(QFocusEvent *e) override;
};

// the elements that appear when adding an action or searched
struct LeftSideElement {
    QPushButton Button;
    QCheckBox CheckBox;
    QSignalMapper ButtonSignalMapper;
    QSignalMapper CheckBoxSignalMapper;
    QPropertyAnimation ButtonAnimation;
    QPropertyAnimation CheckBoxAnimation;
    bool Used = true;
    std::vector<bool> is_entry_valid;
};

struct ColumnData{
    ColumnData(QString header_strings[3], int string_case, int xz_index, int length, QString filler_character, int column_width):
        header_strings(header_strings), filler_character(filler_character), string_case(string_case), xyz_case(xz_index), desired_length(length), column_width(column_width){}
    QString* header_strings;
    QString filler_character;
    int string_case;
    int xyz_case;
    int desired_length;
    int column_width;
};


struct SolutionElement{
    int SetText();
    virtual void SetEnabled(bool enable);
    QFrame top_frame;
    QFrame bottom_frame;
    QLabel header_label;
    QPushButton fold_button;
    QPushButton delete_button;
    QLabel solution_description_label;
    QSignalMapper fold_button_signal_mapper;
    QSignalMapper delete_button_signal_mapper;
    QSignalMapper geometry_ani_end_signal_mapper;
    QSignalMapper opacity_ani_end_signal_mapper;
    QPropertyAnimation geometry_animation;
    QPropertyAnimation opacity_animation;
    QGraphicsOpacityEffect opacity_effect;
    bool folded = true;
    int order_index; // negative if element is unused
    SolutionCopy* full_sol_copy = nullptr;
    SolutionCopy* part_sol_copy = nullptr;// TODO partial solution
};
struct PartialSolutionElement : public SolutionElement{
    void SetEnabled(bool enable) override;
    QPushButton sub_search_button;
    QSignalMapper sub_search_button_signal_mapper;
};

// copy with all data is necessary because the Search-objects clear all data when a new search is started
// This includes all necessary solution data, including "Searched" and "Action" data. Unlike the "Solution" objects,
// a "SolutionCopy" object is indipendent of a "Search" object as parent.
struct SolutionCopy{
    SolutionCopy(Search& search, int solution_type, uint solution_index);
    void Replace(Search& search, int solution_type, uint solution_index);
    QString searched_name;
    WORD searched_pos1, searched_pos2;
    WORD default_pos1, default_pos2;
    WORD found_pos1, found_pos2;
    bool found_first, found_second;
    int costs;
    std::vector<QString> action_names;
    std::vector<HWORD> action_angles;
    std::vector<uint> action_amounts;
    QString xyz_first_string;
    QString xyz_second_string;

private:
    void CopyData(Search& search, int solution_type, uint solution_index);
};
/*
class SearchEvent : public QEvent
{
public:
    QEvent::Type SearchEventType = QEvent::User;

    SearchEvent(QEvent::Type type) :
        QEvent(type)
    {}
};*/

/*
class SolutionsEdit : public QObject
{
    Q_OBJECT

public:
    SolutionsEdit(QWidget* parent, std::vector<SolutionCopy>* solutionCopies, QToolButton* deleteBtn, QToolButton* sortBtn);


public slots:


private:
    std::vector<SolutionCopy>* m_solutions;

    QToolButton* m_deleteBtn;
    QToolButton* m_sortBtn;
};*/

#endif // POSEFI_H
