#include "posefi.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <QCursor>
#include <string>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

typedef uint16_t UINT;
bool HexCheck(QString str);
bool DecCheck(QString str, bool signed_ = true);
QString AdjustHexString(QString in, UINT length, QString filler = "0");
QString ProgressLabelString(uint64_t current, uint64_t total);

Posefi::Posefi(QMainWindow *parent) : QMainWindow(parent), ui(new Ui::Posefi){
    ui->setupUi(this);
    // TODO load settings and maybe a different layout
    LoadWindowDefaultLayout();
    LinkSignals();
    LoadSettings();
}

void Posefi::closeEvent(QCloseEvent *event)
{
    SaveSettings();
    QWidget::closeEvent(event);
}

bool Posefi::event(QEvent* e)
{
    if(e->type() == MainSearchType)
    {
        RunningMainSearch();
        return true;
    }
    if(e->type() == SubSearchType)
    {
        //RunningSubSearch();
        return true;
    }
    return QMainWindow::event(e);
}

Posefi::~Posefi(){
    for (UINT i = 0; i < this->LeftSideActionTable.size(); i++){
        delete this->LeftSideActionTable[i];
    }
    for (UINT i = 0; i < this->LeftSideWantedTable.size(); i++){
        delete this->LeftSideWantedTable[i];
    }
    for (UINT i = 0; i < this->PartialSolutionElements.size(); i++){
        delete this->PartialSolutionElements[i];
    }
    for (UINT i = 0; i < this->FullSolutionElements.size(); i++){
        delete this->FullSolutionElements[i];
    }
    delete ui;
}

int Posefi::LoadWindowDefaultLayout(){

    this->partialSolutionPageEdit = new AutoSelectLineEdit(this->ui->partialSolutionsPageSubFrame);
    this->partialSolutionPageEdit->setAlignment(Qt::AlignCenter);
    this->fullSolutionPageEdit = new AutoSelectLineEdit(this->ui->fullSolutionsPageSubFrame);
    this->fullSolutionPageEdit->setAlignment(Qt::AlignCenter);
    this->defaultFirstLineEdit = new AutoSelectLineEdit(this->ui->defaultPosFrame);
    this->defaultFirstLineEdit->setAlignment(Qt::AlignCenter);
    this->defaultSecondLineEdit = new AutoSelectLineEdit(this->ui->defaultPosFrame);
    this->defaultSecondLineEdit->setAlignment(Qt::AlignCenter);

    // color themes ------------------
    LoadColorThemes();
    SetColorTheme();

// -------------------- SET GEOMETRIES ------------------------
    // main window, central widget
    this->setBaseSize(MAINWND_WIDTH, MAINWND_HEIGHT);
    UINT h = MAINWND_HEIGHT - this->ui->menubar->height() - GAP_BOTTOM;// - QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    this->ui->centralwidget->setGeometry(0, 0, MAINWND_WIDTH, h);
    this->ui->splitterLeftRight->setGeometry(0, 0, MAINWND_WIDTH, h);

    this->ui->splitterLeftRight->setHandleWidth(SPLITTER_HANDLE_SIZE);
    this->ui->splitterLeftTables->setHandleWidth(SPLITTER_HANDLE_SIZE);
    this->ui->splitterRight->setHandleWidth(SPLITTER_HANDLE_SIZE);

    // load general table data
    LOAD_COLUMN_DATA

    this->ui->actionTable->setColumnCount(ActionColumnData.size());
    this->ui->wantedPosTable->setColumnCount(WantedColumnData.size());
    this->ui->actionTableHeader->setColumnCount(ActionColumnData.size());
    this->ui->wantedPosTableHeader->setColumnCount(WantedColumnData.size());
    this->action_table_lables.resize(3);
    int action_table_default_width = 0;
    for(UINT i = 0; i < this->ActionColumnData.size(); i++){
        this->action_table_lables[0].push_back(this->ActionColumnData[i].header_strings[0]);
        this->action_table_lables[1].push_back(this->ActionColumnData[i].header_strings[1]);
        this->action_table_lables[2].push_back(this->ActionColumnData[i].header_strings[2]);
        this->ui->actionTable->setColumnWidth(i,this->ActionColumnData[i].column_width);
        this->ui->actionTableHeader->setColumnWidth(i,this->ActionColumnData[i].column_width);
        action_table_default_width += this->ActionColumnData[i].column_width;
    }
    this->wanted_table_lables.resize(3);
    int wanted_table_default_width = 0;
    for(UINT i = 0; i < this->WantedColumnData.size(); i++){
        this->wanted_table_lables[0].push_back(this->WantedColumnData[i].header_strings[0]);
        this->wanted_table_lables[1].push_back(this->WantedColumnData[i].header_strings[1]);
        this->wanted_table_lables[2].push_back(this->WantedColumnData[i].header_strings[2]);
        this->ui->wantedPosTable->setColumnWidth(i,this->WantedColumnData[i].column_width);
        this->ui->wantedPosTableHeader->setColumnWidth(i,this->WantedColumnData[i].column_width);
        wanted_table_default_width += this->WantedColumnData[i].column_width;
    }

//  menu bar
    fileMenu = this->ui->menubar->addMenu(tr("&File"));
    saveInputsAction = new QAction("Save Input Data", this);
    connect(Posefi::saveInputsAction, &QAction::triggered, this, &Posefi::SaveInputs);
    fileMenu->addAction(saveInputsAction);
    loadInputsAction = new QAction("Load Input Data", this);
    connect(Posefi::loadInputsAction, &QAction::triggered, this, &Posefi::LoadInputs);
    fileMenu->addAction(loadInputsAction);

    // splitter parent
    int action_scroll_area_width = LEFT_ELEMENT_ACTION_WIDTH + action_table_default_width + SCROLLBAR_SIZE;
    int wanted_scroll_area_width = LEFT_ELEMENT_WANTED_POS_WIDTH + wanted_table_default_width + SCROLLBAR_SIZE;
    int left_table_split_area_width = ((action_scroll_area_width > wanted_scroll_area_width) ? action_scroll_area_width : wanted_scroll_area_width);
    this->ui->splitterLeftTables->setGeometry(0,0,left_table_split_area_width, h);

    // splitter children
    UINT h2 = h - SPLIT_AREA_TOP_HEIGHT - SPLITTER_HANDLE_SIZE;

    // top child contents
    this->ui->frameTop->setGeometry(0,0,left_table_split_area_width, SPLIT_AREA_TOP_HEIGHT);
    this->ui->actionTableHeader->setGeometry(LEFT_ELEMENT_ACTION_WIDTH,0, action_table_default_width, TABLE_HEADER_HEIGHT);
    this->ui->actionTableHeader->horizontalHeader()->setFixedHeight(TABLE_HEADER_HEIGHT);
    this->ui->scrollAreaActions->setGeometry(0,TABLE_HEADER_HEIGHT, action_scroll_area_width, SPLIT_AREA_TOP_HEIGHT - TABLE_HEADER_HEIGHT);
    this->ui->scrollAreaActionsContents->setFixedSize(action_scroll_area_width - SCROLLBAR_SIZE, SPLIT_AREA_TOP_HEIGHT - TABLE_HEADER_HEIGHT - 2);
    this->ui->actionTable->setFixedSize(action_table_default_width, ACTION_TABLE_HEIGHT);
    this->ui->actionTable->move(LEFT_ELEMENT_ACTION_WIDTH, 0);
    this->ui->addActionButton->setGeometry(ADD_ACTION_BUTTON_X_POS, ADD_ACTION_BUTTON_Y_POS, ADD_ACTION_BUTTON_WIDTH, ADD_ACTION_BUTTON_HEIGHT);

    // bottom child contents
    this->ui->frameBottom->setGeometry(0,0,left_table_split_area_width, h2);
    this->ui->wantedPosTableHeader->setGeometry(LEFT_ELEMENT_ACTION_WIDTH,0, wanted_table_default_width, TABLE_HEADER_HEIGHT);
    this->ui->wantedPosTableHeader->horizontalHeader()->setFixedHeight(TABLE_HEADER_HEIGHT);
    this->ui->scrollAreaWantedPos->setGeometry(0,TABLE_HEADER_HEIGHT,wanted_scroll_area_width, h2 - TABLE_HEADER_HEIGHT);
    this->ui->scrollAreaWantedPosContents->setFixedSize(wanted_scroll_area_width - SCROLLBAR_SIZE, h2-2 - TABLE_HEADER_HEIGHT);
    this->ui->wantedPosTable->setFixedSize(wanted_table_default_width, h2-2 - TABLE_HEADER_HEIGHT);
    this->ui->wantedPosTable->move(LEFT_ELEMENT_ACTION_WIDTH, 0);
    this->ui->addWantedPosButton->setGeometry(ADD_WANTED_POS_BUTTON_X_POS, ADD_WANTED_POS_BUTTON_Y_POS, ADD_WANTED_POS_BUTTON_WIDTH, ADD_WANTED_POS_BUTTON_HEIGHT);

    // splitter stretch factor
    this->ui->splitterLeftTables->setStretchFactor(0,1);

// ------------ Right side elements ---------------------


    this->ui->rightSideMainFrame->setGeometry(0, 0, RS_MAIN_FRAME_WIDTH, this->ui->splitterLeftRight->height());
    this->ui->splitterRight->setFixedWidth(ui->rightSideMainFrame->width() - GAP_RIGHT);
    this->ui->RadioButtonsAndCheckboxes->setGeometry(RADIOBUTTONS_CHECKBOXES_FRAME_GEOMETRY);
    this->ui->topRightFrame->setGeometry(TOP_RIGHT_FRAME_GEOMETRY);
    this->ui->scrollAreaTopRight->setGeometry(RS_SCROLL_AREA_MOVE,RS_SCROLL_AREA_MOVE+RS_HEADER_LABEL_HEIGHT,this->ui->topRightFrame->width()-(2*RS_SCROLL_AREA_MOVE),this->ui->topRightFrame->height()-(2*RS_SCROLL_AREA_MOVE)-RS_HEADER_LABEL_HEIGHT);
    this->ui->defaultPosFrame->setGeometry(DEFAULT_POS_FRAME_GEOMETRY);
    this->ui->startPauseCancelFrame->setGeometry(START_PAUSE_CANCEL_FRAME_GEOMETRY);
    this->ui->progressBarMainSearch->setGeometry(MAIN_SEARCH_PROGRESSBAR_GEOMETRY);
    this->ui->progressBarMainSearch->setRange(0,100);
    this->ui->progressBarMainSearch->setVisible(false);
    this->ui->mainSearchProgressLabel->setGeometry(MAIN_SEARCH_PROGRESS_LABEL_GEOMETRY);
    this->ui->startButton->setEnabled(false);
    this->ui->pauseButton->setEnabled(false);
    this->ui->cancelButton->setEnabled(false);
    this->ui->redFrameDefaultPos1->setVisible(false);
    this->ui->redFrameDefaultPos2->setVisible(false);
    this->defaultFirstLineEdit->setGeometry(DEFAULT_POS_1ST_LINE_EDIT_GEOMETRY);
    this->defaultSecondLineEdit->setGeometry(DEFAULT_POS_2ND_LINE_EDIT_GEOMETRY);
    this->ui->redFrameDefaultPos1->setGeometry(this->defaultFirstLineEdit->geometry());
    this->ui->redFrameDefaultPos2->setGeometry(this->defaultSecondLineEdit->geometry());

    // geometry of partial solution is set on first resize() call
    this->ui->scrollAreaPartialSolutions->move(RS_SCROLL_AREA_MOVE,RS_HEADER_LABEL_HEIGHT+RS_SCROLL_AREA_MOVE);
    this->ui->scrollAreaPartialSolutions->setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);

    this->ui->subSearchFrame->setGeometry(SUB_SEARCH_FRAME_GEOMETRY);
    this->ui->scrollAreaSubSearch->setGeometry(RS_SCROLL_AREA_MOVE,
                                               RS_SCROLL_AREA_MOVE+RS_HEADER_LABEL_HEIGHT,
                                               this->ui->subSearchFrame->width()-(2*RS_SCROLL_AREA_MOVE),
                                               this->ui->subSearchFrame->height()-(2*RS_SCROLL_AREA_MOVE));

    this->ui->scrollAreaFullSolutions->setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    this->ui->fullSolutionsFrame->setGeometry(FULL_SOLUTIONS_FRAME_GEOMETRY);
    this->ui->scrollAreaFullSolutions->setGeometry(RS_SCROLL_AREA_MOVE,
                                                   RS_SCROLL_AREA_MOVE+RS_HEADER_LABEL_HEIGHT,
                                                   this->ui->fullSolutionsFrame->width()-(2*RS_SCROLL_AREA_MOVE),
                                                   this->ui->fullSolutionsFrame->height()-(2*RS_SCROLL_AREA_MOVE)-PAGE_NAVIGATION_HEIGHT);

    this->ui->splitterRight->setStretchFactor(1,1);
    this->ui->splitterLeftRight->setStretchFactor(1,1);

    this->ui->partialSolutionsFirstButton->setEnabled(false);
    this->ui->partialSolutionsPrevButton->setEnabled(false);
    this->ui->partialSolutionsNextButton->setEnabled(false);
    this->ui->partialSolutionsLastButton->setEnabled(false);
    this->partialSolutionPageEdit->setEnabled(false);

    this->ui->fullSolutionsFirstButton->setEnabled(false);
    this->ui->fullSolutionsPrevButton->setEnabled(false);
    this->ui->fullSolutionsNextButton->setEnabled(false);
    this->ui->fullSolutionsLastButton->setEnabled(false);
    this->fullSolutionPageEdit->setEnabled(false);

    this->ui->labelMainSearch->adjustSize();
    this->ui->labelPartialSolutions->adjustSize();
    this->ui->labelSubSearch->adjustSize();
    this->ui->labelFullSolutions->adjustSize();
    this->ui->labelMainSearch->setFixedSize(this->ui->labelMainSearch->width()+RS_HEADER_LABEL_ADDED_WIDTH, RS_HEADER_LABEL_HEIGHT);
    this->ui->labelPartialSolutions->setFixedSize(this->ui->labelPartialSolutions->width()+RS_HEADER_LABEL_ADDED_WIDTH, RS_HEADER_LABEL_HEIGHT);
    this->ui->labelSubSearch->setFixedSize(this->ui->labelSubSearch->width()+RS_HEADER_LABEL_ADDED_WIDTH, RS_HEADER_LABEL_HEIGHT);
    this->ui->labelFullSolutions->setFixedSize(this->ui->labelFullSolutions->width()+RS_HEADER_LABEL_ADDED_WIDTH, RS_HEADER_LABEL_HEIGHT);
    this->ui->labelMainSearch->move(RS_HEADER_LABEL_XY);
    this->ui->labelPartialSolutions->move(RS_HEADER_LABEL_XY);
    this->ui->labelSubSearch->move(RS_HEADER_LABEL_XY);
    this->ui->labelFullSolutions->move(RS_HEADER_LABEL_XY);

    //partialSolutionsEdit = new SolutionsEdit(this, &partial_solution_copies, ui->partialSolutionsDeleteButton, ui->partialSolutionsSortButton);
    //fullSolutionsEdit = new SolutionsEdit(this, &full_solution_copies, ui->fullSolutionsDeleteButton, ui->fullSolutionsSortButton);


// -------------------------------------------------------------------------



    // Radio Button Group ---------------------
    this->findXYZ_radioButtons.addButton(this->ui->radioButton_xyz_first, 0);
    this->findXYZ_radioButtons.addButton(this->ui->radioButton_xyz_second, 1);
    this->findXYZ_radioButtons.addButton(this->ui->radioButton_xyz_both, 2);
    this->ui->radioButton_xyz_both->setChecked(true);
    this->xyzLable_radioButtons.addButton(this->ui->radioButton_xy, 0);
    this->xyzLable_radioButtons.addButton(this->ui->radioButton_xz, 1);
    this->xyzLable_radioButtons.addButton(this->ui->radioButton_yz, 2);

    // load all label groups and set to xz
    this->find_first_labels = new QString[] FIND_FIRST_LABELS;
    this->find_second_labels = new QString[] FIND_SECOND_LABELS;
    this->resettable_first_labels = new QString[] RESETTABLE_FIRST_LABELS;
    this->resettable_second_labels = new QString[] RESETTABLE_SECOND_LABELS;
    this->default_pos_first_labels = new QString[] DEFAULT_POS_FIRST_LABELS;
    this->default_pos_second_labels = new QString[] DEFAULT_POS_SECOND_LABELS;
    this->ui->radioButton_xz->setChecked(true);
    this->LabelXYZ_RadioButton_toggle(1);
    this->solution_first_pos_labels = new QString[] PSE_FIRST_LABELS;
    this->solution_second_pos_labels = new QString[] PSE_SECOND_LABELS;

    // -------------------------------------------


    return SUCCESS;
}

int Posefi::LinkSignals(){
    // Splitter
    connect(ui->splitterLeftRight, SIGNAL(splitterMoved(int,int)), this, SLOT(AdjustLeftRightSplitter()));
    connect(ui->splitterLeftTables, SIGNAL(splitterMoved(int,int)), this, SLOT(AdjustLeftTableSplitter()));
    connect(ui->splitterRight, SIGNAL(splitterMoved(int,int)), this, SLOT(AdjustRightSideSplitter()));

    // Top left scroll area
    connect(ui->addActionButton, SIGNAL(clicked()), this, SLOT(AddAction()));
    connect(this->ui->actionTable, SIGNAL(cellChanged(int,int)), this, SLOT(ActionTableCell_edited(int,int)));
    connect(this->ui->actionTableHeader->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(ResizeActionTableHeader()));
    connect(this->ui->scrollAreaActions->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(TopScrollArea_horizontalScroll(int)));
    add_action_button_animation.setTargetObject(this->ui->addActionButton);
    add_action_button_animation.setPropertyName("geometry");

    // Bottom left scroll area
    connect(ui->addWantedPosButton, SIGNAL(clicked()), this, SLOT(AddWantedPos()));
    connect(this->ui->wantedPosTable, SIGNAL(cellChanged(int,int)), this, SLOT(WantedTableCell_edited(int,int)));
    connect(this->ui->wantedPosTableHeader->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(ResizeWantedTableHeader()));
    connect(this->ui->scrollAreaWantedPos->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(BottomScrollArea_horizontalScroll(int)));
    add_wanted_button_animation.setTargetObject(this->ui->addWantedPosButton);
    add_wanted_button_animation.setPropertyName("geometry");

    // Right Side
    connect(&this->findXYZ_radioButtons, SIGNAL(idClicked(int)), this, SLOT(FindXZ_RadioButton_toggle(int)));
    connect(&this->xyzLable_radioButtons, SIGNAL(idClicked(int)), this, SLOT(LabelXYZ_RadioButton_toggle(int)));
    connect(this->ui->startButton, SIGNAL(clicked()), this, SLOT(StartButton_press()));
    connect(this->ui->pauseButton, SIGNAL(clicked()), this, SLOT(PauseButton_press()));
    connect(this->ui->cancelButton, SIGNAL(clicked()), this, SLOT(CancelButton_press()));
    connect(this->defaultFirstLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(DefaultFirstLineEdit_change(const QString&)));
    connect(this->defaultSecondLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(DefaultSecondLineEdit_change(const QString&)));

    // Partial Solutions
    connect(&this->pse_scroll_resize_timer, SIGNAL(timeout()), this, SLOT(AdjustPartialSolutionScrollAreaHeight()));
    connect(this->ui->partialSolutionsFirstButton, SIGNAL(clicked()), this, SLOT(PartialSolutionFirstPageButton_press()));
    connect(this->ui->partialSolutionsPrevButton, SIGNAL(clicked()), this, SLOT(PartialSolutionPrevPageButton_press()));
    connect(this->ui->partialSolutionsNextButton, SIGNAL(clicked()), this, SLOT(PartialSolutionNextPageButton_press()));
    connect(this->ui->partialSolutionsLastButton, SIGNAL(clicked()), this, SLOT(PartialSolutionLastPageButton_press()));
    connect(this->partialSolutionPageEdit, SIGNAL(editingFinished()),this,SLOT(PartialSolutionPageEdit_change()));
    QAction* deleteAllPartialSolutionsAction = new QAction("Delete All");
    connect(deleteAllPartialSolutionsAction, &QAction::triggered, this, &Posefi::deleteAllPartialSolutions);
    ui->partialSolutionsDeleteButton->addAction(deleteAllPartialSolutionsAction);
    QAction* sortPartialSolutionsByCostAction = new QAction("Sort by cost");
    connect(sortPartialSolutionsByCostAction, &QAction::triggered, this, &Posefi::sortPartialSolutionsByCost);
    ui->partialSolutionsSortButton->addAction(sortPartialSolutionsByCostAction);
    QAction* sortPartialSolutionsByAngleCountAction = new QAction("Sort by count of unique angles");
    connect(sortPartialSolutionsByAngleCountAction, &QAction::triggered, this, &Posefi::sortPartialSolutionsByAngleCount);
    ui->partialSolutionsSortButton->addAction(sortPartialSolutionsByAngleCountAction);
    QAction* sortPartialSolutionsByActionCountAction = new QAction("Sort by count of unique actions");
    connect(sortPartialSolutionsByActionCountAction, &QAction::triggered, this, &Posefi::sortPartialSolutionsByActionCount);
    ui->partialSolutionsSortButton->addAction(sortPartialSolutionsByActionCountAction);

    // Full Solutions
    connect(&this->fse_scroll_resize_timer, SIGNAL(timeout()), this, SLOT(AdjustFullSolutionScrollAreaHeight()));
    connect(this->ui->fullSolutionsFirstButton, SIGNAL(clicked()), this, SLOT(FullSolutionFirstPageButton_press()));
    connect(this->ui->fullSolutionsPrevButton, SIGNAL(clicked()), this, SLOT(FullSolutionPrevPageButton_press()));
    connect(this->ui->fullSolutionsNextButton, SIGNAL(clicked()), this, SLOT(FullSolutionNextPageButton_press()));
    connect(this->ui->fullSolutionsLastButton, SIGNAL(clicked()), this, SLOT(FullSolutionLastPageButton_press()));
    connect(this->fullSolutionPageEdit, SIGNAL(editingFinished()),this,SLOT(FullSolutionPageEdit_change()));
    QAction* deleteAllFullSolutionsAction = new QAction("Delete All");
    connect(deleteAllFullSolutionsAction, &QAction::triggered, this, &Posefi::deleteAllFullSolutions);
    ui->fullSolutionsDeleteButton->addAction(deleteAllFullSolutionsAction);
    QAction* sortFullSolutionsByCostAction = new QAction("Sort by cost");
    connect(sortFullSolutionsByCostAction, &QAction::triggered, this, &Posefi::sortFullSolutionsByCost);
    ui->fullSolutionsSortButton->addAction(sortFullSolutionsByCostAction);
    QAction* sortFullSolutionsByAngleCountAction = new QAction("Sort by count of unique angles");
    connect(sortFullSolutionsByAngleCountAction, &QAction::triggered, this, &Posefi::sortFullSolutionsByAngleCount);
    ui->fullSolutionsSortButton->addAction(sortFullSolutionsByAngleCountAction);
    QAction* sortFullSolutionsByActionCountAction = new QAction("Sort by count of unique actions");
    connect(sortFullSolutionsByActionCountAction, &QAction::triggered, this, &Posefi::sortFullSolutionsByActionCount);
    ui->fullSolutionsSortButton->addAction(sortFullSolutionsByActionCountAction);

    // Search timer
    connect(&this->main_search_timer, SIGNAL(timeout()), this, SLOT(RunningMainSearch()));


    // -------- Sub Search ------------------------
    //subSearchWidget = new SubSearchWidget(ui->scrollAreaSubSearchContents);

    return SUCCESS;
}

int Posefi::AdjustLeftRightSplitter(){
    ResizeSplitterRightWidth();
    ui->scrollAreaActions->setFixedWidth(this->ui->splitterLeftTables->width());
    ui->scrollAreaWantedPos->setFixedWidth(this->ui->splitterLeftTables->width());
    int visible_action_table_width = ACTION_TABLE_VISIBLE_WIDTH;
    int visible_wanted_table_width = WANTED_TABLE_VISIBLE_WIDTH;
    ui->actionTableHeader->setFixedWidth(visible_action_table_width + ui->scrollAreaActions->horizontalScrollBar()->value());
    ui->wantedPosTableHeader->setFixedWidth(visible_wanted_table_width + ui->scrollAreaWantedPos->horizontalScrollBar()->value());
    this->ArrangeActionColumnWidths();
    this->ArrangeWantedColumnWidths();
    return SUCCESS;
}

int Posefi::AdjustLeftTableSplitter(){
    ui->scrollAreaActions->setFixedHeight(this->ui->frameTop->height() - this->ui->actionTableHeader->horizontalHeader()->height());
    ui->scrollAreaWantedPos->setFixedHeight(this->ui->frameBottom->height() - this->ui->wantedPosTableHeader->horizontalHeader()->height());
    ResizeActionTableHeight();
    ResizeWantedTableHeight();

    return SUCCESS;
}

int Posefi::AdjustRightSideSplitter(){
    this->ResizeSplitterRightHeight();
    this->AdjustPartialSolutionScrollAreaHeight();
    this->AdjustFullSolutionScrollAreaHeight();
    return SUCCESS;
}

void Posefi::resizeEvent(QResizeEvent *e){

    UINT h = e->size().height() - this->ui->menubar->height() - GAP_BOTTOM;
    this->ui->centralwidget->setBaseSize(this->width(), h);
    this->ui->splitterLeftRight->setFixedSize(this->width(), h);

    // skip left side on first call
    if (!e->oldSize().isValid()){
        goto rightSide;
    }

    // Left side
    this->ui->splitterLeftTables->setFixedHeight(h);
    this->ui->scrollAreaActions->setFixedSize(this->ui->splitterLeftTables->width(), this->ui->frameTop->height() - TABLE_HEADER_HEIGHT);
    this->ui->scrollAreaWantedPos->setFixedSize(this->ui->splitterLeftTables->width(), this->ui->frameBottom->height() - TABLE_HEADER_HEIGHT);
    this->ResizeActionTableHeight();
    this->ResizeWantedTableHeight();
    ui->actionTableHeader->setFixedWidth(ACTION_TABLE_VISIBLE_WIDTH + ui->scrollAreaActions->horizontalScrollBar()->value());
    ui->wantedPosTableHeader->setFixedWidth(WANTED_TABLE_VISIBLE_WIDTH + ui->scrollAreaWantedPos->horizontalScrollBar()->value());

    // TODO bugged
    //this->ArrangeActionColumnWidths();
    //this->ArrangeWantedColumnWidths();

    rightSide:

    // Right side
    this->ui->rightSideMainFrame->setFixedHeight(h);
    this->ui->splitterRight->setFixedHeight(h);
    this->ResizeSplitterRightWidth();
    this->ResizeSplitterRightHeight();
    if (!e->oldSize().isValid()){   // Manually set this scroll area on first call because the resize function is called when the frame is not the right size
        int h = this->ui->splitterRight->height() - (3 * SPLITTER_HANDLE_SIZE) - this->ui->topRightFrame->height()
                - this->ui->subSearchFrame->height() - this->ui->fullSolutionsFrame->height() - RS_HEADER_LABEL_HEIGHT
                - PAGE_NAVIGATION_HEIGHT - (2*RS_SCROLL_AREA_MOVE);

        // partial solution stuff
        this->ui->scrollAreaPartialSolutions->setFixedHeight(h);
        this->ui->partialSolutionsPageFrame->setGeometry(
                    RS_SCROLL_AREA_MOVE,
                    this->ui->scrollAreaPartialSolutions->height() + this->ui->scrollAreaPartialSolutions->y(),
                    this->ui->scrollAreaPartialSolutions->width(),
                    PAGE_NAVIGATION_HEIGHT);
        this->ui->partialSolutionsPageSubFrame->setFixedSize(PSE_PAGE_SUB_FRAME_SIZE);
        int y = (this->ui->partialSolutionsPageFrame->height() - this->ui->partialSolutionsPageSubFrame->height()) / 2;
        int x = (this->ui->partialSolutionsPageFrame->width() - this->ui->partialSolutionsPageSubFrame->width()) / 2;
        this->ui->partialSolutionsPageSubFrame->move(x,y);
        this->ui->partialSolutionsFirstButton->setFixedSize(PSE_NAVI_BUTTON_SIZE);
        this->ui->partialSolutionsPrevButton->setFixedSize(PSE_NAVI_BUTTON_SIZE);
        this->ui->partialSolutionsNextButton->setFixedSize(PSE_NAVI_BUTTON_SIZE);
        this->ui->partialSolutionsLastButton->setFixedSize(PSE_NAVI_BUTTON_SIZE);
        this->partialSolutionPageEdit->setFixedSize(PSE_PAGE_EDIT_SIZE);
        this->ui->partialSolutionsShowLabel->setFixedSize(PSE_SHOW_LABEL_SIZE);
        y = (this->ui->partialSolutionsPageFrame->height() - this->ui->partialSolutionsShowLabel->height()) / 2;
        this->ui->partialSolutionsShowLabel->move(PSE_SHOW_LABEL_X,y);
        x = (this->ui->partialSolutionsPageSubFrame->width() - this->partialSolutionPageEdit->width()) / 2;
        y = (this->ui->partialSolutionsPageSubFrame->height() - this->partialSolutionPageEdit->height()) / 2;
        this->partialSolutionPageEdit->move(x,y);
        y = (this->ui->partialSolutionsPageSubFrame->height() - this->ui->partialSolutionsFirstButton->height()) / 2;
        x = this->partialSolutionPageEdit->width() + this->partialSolutionPageEdit->x() + PSE_NAVI_EDIT_GAP;
        this->ui->partialSolutionsNextButton->move(x,y);
        x += PSE_NAVI_BUTTON_GAP + this->ui->partialSolutionsNextButton->width();
        this->ui->partialSolutionsLastButton->move(x,y);
        x = this->partialSolutionPageEdit->x() - PSE_NAVI_EDIT_GAP - this->ui->partialSolutionsPrevButton->width();
        this->ui->partialSolutionsPrevButton->move(x,y);
        x -= PSE_NAVI_BUTTON_GAP + this->ui->partialSolutionsFirstButton->width();
        this->ui->partialSolutionsFirstButton->move(x,y);
        this->AdjustPartialSolutionScrollAreaHeight();

        // full solution stuff
        this->ui->scrollAreaFullSolutions->setFixedHeight(h);
        this->ui->fullSolutionsPageFrame->setGeometry(
                    RS_SCROLL_AREA_MOVE,
                    this->ui->scrollAreaFullSolutions->height() + this->ui->scrollAreaFullSolutions->y(),
                    this->ui->scrollAreaFullSolutions->width(),
                    PAGE_NAVIGATION_HEIGHT);
        QSize fullSolSubFrameSize(FSE_PAGE_SUB_FRAME_SIZE);
        this->ui->fullSolutionsPageSubFrame->setFixedSize(fullSolSubFrameSize);
        y = (this->ui->fullSolutionsPageFrame->height() - fullSolSubFrameSize.height()) / 2;
        x = (this->ui->fullSolutionsPageFrame->width() - fullSolSubFrameSize.width()) / 2;
        this->ui->fullSolutionsPageSubFrame->move(x,y);
        this->ui->fullSolutionsFirstButton->setFixedSize(FSE_NAVI_BUTTON_SIZE);
        this->ui->fullSolutionsPrevButton->setFixedSize(FSE_NAVI_BUTTON_SIZE);
        this->ui->fullSolutionsNextButton->setFixedSize(FSE_NAVI_BUTTON_SIZE);
        this->ui->fullSolutionsLastButton->setFixedSize(FSE_NAVI_BUTTON_SIZE);
        this->fullSolutionPageEdit->setFixedSize(FSE_PAGE_EDIT_SIZE);
        this->ui->fullSolutionsShowLabel->setFixedSize(FSE_SHOW_LABEL_SIZE);
        y = (this->ui->fullSolutionsPageFrame->height() - this->ui->fullSolutionsShowLabel->height()) / 2;
        this->ui->fullSolutionsShowLabel->move(FSE_SHOW_LABEL_X,y);
        x = (fullSolSubFrameSize.width() - this->fullSolutionPageEdit->width()) / 2;
        y = (fullSolSubFrameSize.height() - this->fullSolutionPageEdit->height()) / 2;
        this->fullSolutionPageEdit->move(x,y);
        y = (fullSolSubFrameSize.height() - this->ui->fullSolutionsFirstButton->height()) / 2;
        x = this->fullSolutionPageEdit->width() + this->fullSolutionPageEdit->x() + FSE_NAVI_EDIT_GAP;
        this->ui->fullSolutionsNextButton->move(x,y);
        x += FSE_NAVI_BUTTON_GAP + this->ui->fullSolutionsNextButton->width();
        this->ui->fullSolutionsLastButton->move(x,y);
        x = this->fullSolutionPageEdit->x() - FSE_NAVI_EDIT_GAP - this->ui->fullSolutionsPrevButton->width();
        this->ui->fullSolutionsPrevButton->move(x,y);
        x -= FSE_NAVI_BUTTON_GAP + this->ui->fullSolutionsFirstButton->width();
        this->ui->fullSolutionsFirstButton->move(x,y);

        //Solution edit buttons
        UpdateEditButtonGeometry();
    }
}

int Posefi::AddAction(){

    UINT index = ui->actionTable->rowCount();
    this->ui->actionTable->insertRow(index);
    ResizeActionTableHeight();
    AddActionButtonAnimation();

    if(this->ui->actionTable->rowCount() > (int)this->LeftSideActionTable.size()){
        AddLeftSideElementActionTable();
    }
    else{
        for (UINT i = 0; i < this->LeftSideActionTable[index]->is_entry_valid.size(); i++){
            this->LeftSideActionTable[index]->is_entry_valid[i] = true;
        }

    }
    for (UINT i = 0; i < this->ui->actionTable->columnCount(); i++){
        this->ui->actionTable->setItem(index, i, new QTableWidgetItem(QString("")));
        this->ui->actionTable->item(index, i)->setTextAlignment(TEXT_ALIGNMENT);
    }

    ArrangeLeftSideActionTable();

    // Here could be animations for remove button and checkbox
#ifdef LEFT_SIDE_ACTION_TABLE_ANIMATION
    this->LeftSideActionTable.back()->ButtonAnimation.setDuration(REMOVE_BUTTON_ANIMATION_LENGTH);
    //this->LeftSideActionTable.back()->ButtonAnimation.setStartValue(QRect(REMOVE_BUTTON_X_POS + (REMOVE_BUTTON_WIDTH/2), this->LeftSideActionTable.back()->Button.y() + (REMOVE_BUTTON_HEIGHT/2), 0, 0));
    this->LeftSideActionTable.back()->ButtonAnimation.setStartValue(QRect(REMOVE_BUTTON_X_POS, this->LeftSideActionTable.back()->Button.y(), REMOVE_BUTTON_WIDTH, 0));
    this->LeftSideActionTable.back()->ButtonAnimation.setEndValue(QRect(REMOVE_BUTTON_X_POS, this->LeftSideActionTable.back()->Button.y(), REMOVE_BUTTON_WIDTH, REMOVE_BUTTON_HEIGHT));
    //this->LeftSideActionTable.back()->ButtonAnimation.setEasingCurve(QEasingCurve::OutQuad);
    this->LeftSideActionTable.back()->ButtonAnimation.start();

    this->LeftSideActionTable.back()->CheckBoxAnimation.setDuration(CHECK_BOX_ANIMATION_LENGTH);
    //this->LeftSideActionTable.back()->CheckBoxAnimation.setStartValue(QRect(USED_CHECKBOX_X_POS + (USED_CHECKBOX_WIDTH/2), this->LeftSideActionTable.back()->CheckBox.y() + (USED_CHECKBOX_HEIGHT/2), 0, 0));
    this->LeftSideActionTable.back()->CheckBoxAnimation.setStartValue(QRect(USED_CHECKBOX_X_POS, this->LeftSideActionTable.back()->CheckBox.y(), USED_CHECKBOX_WIDTH, 0));
    this->LeftSideActionTable.back()->CheckBoxAnimation.setEndValue(QRect(USED_CHECKBOX_X_POS, this->LeftSideActionTable.back()->CheckBox.y(), USED_CHECKBOX_WIDTH, USED_CHECKBOX_HEIGHT));
    //this->LeftSideActionTable.back()->CheckBoxAnimation.setEasingCurve(QEasingCurve::OutQuad);
    this->LeftSideActionTable.back()->CheckBoxAnimation.start();
#endif

    SetStartButtonState();

    return SUCCESS;
}

int Posefi::AddWantedPos(){
    UINT index = ui->wantedPosTable->rowCount();
    this->ui->wantedPosTable->insertRow(index);
    ResizeWantedTableHeight();
    AddWantedButtonAnimation();

    if(this->ui->wantedPosTable->rowCount() > (int)this->LeftSideWantedTable.size()){
        AddLeftSideElementWantedTable();
    }
    else{
        for (UINT i = 0; i < this->LeftSideWantedTable[index]->is_entry_valid.size(); i++){
            this->LeftSideWantedTable[index]->is_entry_valid[i] = true;
        }
    }
    for (UINT i = 0; i < this->ui->wantedPosTable->columnCount(); i++){
        this->ui->wantedPosTable->setItem(index, i, new QTableWidgetItem(QString("")));
        this->ui->wantedPosTable->item(index, i)->setTextAlignment(TEXT_ALIGNMENT);
    }

    ArrangeLeftSideWantedTable();

#ifdef LEFT_SIDE_WANTED_TABLE_ANIMATION
    // Here could be animations for remove button and checkbox
#endif

    SetStartButtonState();

    return SUCCESS;
}

int Posefi::AddActionButtonAnimation(){

    UINT row_count = this->ui->actionTable->rowCount();
    int add_button_y = ADD_ACTION_BUTTON_Y_POS;
    if (row_count > 0)
        add_button_y += this->ui->actionTable->rowViewportPosition(row_count - 1) + this->ui->actionTable->rowHeight(row_count - 1);

    add_action_button_animation.setDuration(ADD_ACTION_BUTTON_ANIMATION_LENGTH);
    add_action_button_animation.setStartValue(QRect(this->ui->addActionButton->x(), this->ui->addActionButton->y(), ADD_ACTION_BUTTON_WIDTH, ADD_ACTION_BUTTON_HEIGHT));
    add_action_button_animation.setEndValue(QRect(this->ui->addActionButton->x(), add_button_y, ADD_ACTION_BUTTON_WIDTH, ADD_ACTION_BUTTON_HEIGHT));
    add_action_button_animation.setEasingCurve(QEasingCurve::OutQuad);
    add_action_button_animation.start();
    return SUCCESS;
}

int Posefi::AddWantedButtonAnimation(){

    UINT row_count = this->ui->wantedPosTable->rowCount();
    int add_button_y = ADD_WANTED_POS_BUTTON_Y_POS;
    if (row_count > 0)
        add_button_y += this->ui->wantedPosTable->rowViewportPosition(row_count - 1) + this->ui->wantedPosTable->rowHeight(row_count - 1);

    add_wanted_button_animation.setDuration(ADD_WANTED_POS_BUTTON_ANIMATION_LENGTH);
    add_wanted_button_animation.setStartValue(QRect(this->ui->addWantedPosButton->x(), this->ui->addWantedPosButton->y(), ADD_WANTED_POS_BUTTON_WIDTH, ADD_WANTED_POS_BUTTON_HEIGHT));
    add_wanted_button_animation.setEndValue(QRect(this->ui->addWantedPosButton->x(), add_button_y, ADD_WANTED_POS_BUTTON_WIDTH, ADD_WANTED_POS_BUTTON_HEIGHT));
    add_wanted_button_animation.setEasingCurve(QEasingCurve::OutQuad);
    add_wanted_button_animation.start();
    return SUCCESS;
}

int Posefi::AddLeftSideElementActionTable(){
    UINT index = this->LeftSideActionTable.size();

    this->LeftSideActionTable.push_back(new LeftSideElement);
    this->LeftSideActionTable.back()->Button.setParent(this->ui->scrollAreaActionsContents);
    this->LeftSideActionTable.back()->Button.setObjectName("removeBtn");
    this->LeftSideActionTable.back()->ButtonSignalMapper.setParent(this);
    this->LeftSideActionTable.back()->Button.show();
    this->LeftSideActionTable.back()->CheckBox.setParent(this->ui->scrollAreaActionsContents);
    this->LeftSideActionTable.back()->CheckBox.setChecked(true);
    this->LeftSideActionTable.back()->CheckBox.show();
#ifndef LEFT_SIDE_ACTION_TABLE_ANIMATION
    this->LeftSideActionTable.back()->Button.setFixedSize(REMOVE_BUTTON_WIDTH,REMOVE_BUTTON_HEIGHT);
    this->LeftSideActionTable.back()->CheckBox.setFixedSize(USED_CHECKBOX_WIDTH,USED_CHECKBOX_HEIGHT);
#endif

    this->LeftSideActionTable.back()->Used = true;
    for(UINT i = 0; i < this->ui->actionTable->columnCount(); i++){
        this->LeftSideActionTable.back()->is_entry_valid.push_back(true);
    }

    connect(&this->LeftSideActionTable.back()->Button, SIGNAL(clicked()), &this->LeftSideActionTable.back()->ButtonSignalMapper, SLOT(map()));
    this->LeftSideActionTable.back()->ButtonSignalMapper.setMapping(&this->LeftSideActionTable.back()->Button, index);
    connect(&this->LeftSideActionTable.back()->ButtonSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(RemoveActionButton_press(int)));

    connect(&this->LeftSideActionTable.back()->CheckBox, SIGNAL(stateChanged(int)), &this->LeftSideActionTable.back()->CheckBoxSignalMapper, SLOT(map()));
    this->LeftSideActionTable.back()->CheckBoxSignalMapper.setMapping(&this->LeftSideActionTable.back()->CheckBox, index);
    connect(&this->LeftSideActionTable.back()->CheckBoxSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(ActionCheckBox_press(int)));

    this->LeftSideActionTable[index]->ButtonAnimation.setTargetObject(&this->LeftSideActionTable[index]->Button);
    this->LeftSideActionTable[index]->CheckBoxAnimation.setTargetObject(&this->LeftSideActionTable[index]->CheckBox);
    this->LeftSideActionTable[index]->ButtonAnimation.setPropertyName("geometry");
    this->LeftSideActionTable[index]->CheckBoxAnimation.setPropertyName("geometry");

    return SUCCESS;
}

int Posefi::AddLeftSideElementWantedTable(){
    UINT index = this->LeftSideWantedTable.size();

    this->LeftSideWantedTable.push_back(new LeftSideElement);
    this->LeftSideWantedTable.back()->Button.setParent(this->ui->scrollAreaWantedPosContents);
    this->LeftSideWantedTable.back()->Button.setObjectName("removeBtn");
    this->LeftSideWantedTable.back()->ButtonSignalMapper.setParent(this);
    this->LeftSideWantedTable.back()->Button.show();
    this->LeftSideWantedTable.back()->CheckBox.setParent(this->ui->scrollAreaWantedPosContents);
    this->LeftSideWantedTable.back()->CheckBox.setChecked(true);
    this->LeftSideWantedTable.back()->CheckBox.show();
#ifndef LEFT_SIDE_WANTED_TABLE_ANIMATION
    this->LeftSideWantedTable.back()->Button.setFixedSize(REMOVE_BUTTON_WIDTH,REMOVE_BUTTON_HEIGHT);
    this->LeftSideWantedTable.back()->CheckBox.setFixedSize(USED_CHECKBOX_WIDTH,USED_CHECKBOX_HEIGHT);
#endif

    this->LeftSideWantedTable.back()->Used = true;
    for(UINT i = 0; i < this->ui->wantedPosTable->columnCount(); i++){
        this->LeftSideWantedTable.back()->is_entry_valid.push_back(true);
    }

    connect(&this->LeftSideWantedTable.back()->Button, SIGNAL(clicked()), &this->LeftSideWantedTable.back()->ButtonSignalMapper, SLOT(map()));
    this->LeftSideWantedTable.back()->ButtonSignalMapper.setMapping(&this->LeftSideWantedTable.back()->Button, index);
    connect(&this->LeftSideWantedTable.back()->ButtonSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(RemoveWantedButton_press(int)));

    connect(&this->LeftSideWantedTable.back()->CheckBox, SIGNAL(stateChanged(int)), &this->LeftSideWantedTable.back()->CheckBoxSignalMapper, SLOT(map()));
    this->LeftSideWantedTable.back()->CheckBoxSignalMapper.setMapping(&this->LeftSideWantedTable.back()->CheckBox, index);
    connect(&this->LeftSideWantedTable.back()->CheckBoxSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(WantedCheckBox_press(int)));

    this->LeftSideWantedTable[index]->ButtonAnimation.setTargetObject(&this->LeftSideWantedTable[index]->Button);
    this->LeftSideWantedTable[index]->CheckBoxAnimation.setTargetObject(&this->LeftSideWantedTable[index]->CheckBox);
    this->LeftSideWantedTable[index]->ButtonAnimation.setPropertyName("geometry");
    this->LeftSideWantedTable[index]->CheckBoxAnimation.setPropertyName("geometry");

    return SUCCESS;
}

int Posefi::ArrangeLeftSideActionTable(){
    int header_height = ui->actionTable->horizontalHeader()->height();
    int y_pos;
    int row_count = this->ui->actionTable->rowCount();
    for(UINT index = 0; index < this->LeftSideActionTable.size(); index++){
        if(index < row_count){
            y_pos = this->ui->actionTable->rowViewportPosition(index);
            y_pos += ((this->ui->actionTable->rowHeight(index) - REMOVE_BUTTON_HEIGHT)/2);
            y_pos += header_height;
            this->LeftSideActionTable[index]->Button.move(REMOVE_BUTTON_X_POS, y_pos);
            this->LeftSideActionTable[index]->CheckBox.move(USED_CHECKBOX_X_POS, y_pos);
            this->LeftSideActionTable[index]->Button.setEnabled(true);
            this->LeftSideActionTable[index]->Button.setVisible(true);
            this->LeftSideActionTable[index]->CheckBox.setEnabled(true);
            this->LeftSideActionTable[index]->CheckBox.setVisible(true);
            this->LeftSideActionTable[index]->CheckBox.setChecked(this->LeftSideActionTable[index]->Used);
#ifndef LEFT_SIDE_ACTION_TABLE_ANIMATION
            this->LeftSideActionTable[index]->Button.setFixedSize(REMOVE_BUTTON_WIDTH,REMOVE_BUTTON_HEIGHT);
            this->LeftSideActionTable[index]->CheckBox.setFixedSize(USED_CHECKBOX_WIDTH,USED_CHECKBOX_HEIGHT);
#endif
        }
        else{
            this->LeftSideActionTable[index]->Button.setEnabled(false);
            this->LeftSideActionTable[index]->Button.setVisible(false);
            this->LeftSideActionTable[index]->CheckBox.setEnabled(false);
            this->LeftSideActionTable[index]->CheckBox.setVisible(false);
            this->LeftSideActionTable[index]->CheckBox.setCheckState(Qt::CheckState::Checked);
#ifndef LEFT_SIDE_ACTION_TABLE_ANIMATION
            this->LeftSideActionTable[index]->Button.setFixedSize(0,0);
            this->LeftSideActionTable[index]->CheckBox.setFixedSize(0,0);
#endif
        }
    }
    return SUCCESS;
}

int Posefi::ArrangeLeftSideWantedTable(){
    int header_height = ui->wantedPosTable->horizontalHeader()->height();
    int y_pos;
    int row_count = this->ui->wantedPosTable->rowCount();
    for(UINT index = 0; index < this->LeftSideWantedTable.size(); index++){
        if(index < row_count){
            y_pos = this->ui->wantedPosTable->rowViewportPosition(index);
            y_pos += ((this->ui->wantedPosTable->rowHeight(index) - REMOVE_BUTTON_HEIGHT)/2);
            y_pos += header_height;
            this->LeftSideWantedTable[index]->Button.move(REMOVE_BUTTON_X_POS, y_pos);
            this->LeftSideWantedTable[index]->CheckBox.move(USED_CHECKBOX_X_POS, y_pos);
            this->LeftSideWantedTable[index]->Button.setEnabled(true);
            this->LeftSideWantedTable[index]->Button.setVisible(true);
            this->LeftSideWantedTable[index]->CheckBox.setEnabled(true);
            this->LeftSideWantedTable[index]->CheckBox.setVisible(true);
            this->LeftSideWantedTable[index]->CheckBox.setChecked(this->LeftSideWantedTable[index]->Used);

#ifndef LEFT_SIDE_WANTED_TABLE_ANIMATION
            this->LeftSideWantedTable[index]->Button.setFixedSize(REMOVE_BUTTON_WIDTH,REMOVE_BUTTON_HEIGHT);
            this->LeftSideWantedTable[index]->CheckBox.setFixedSize(USED_CHECKBOX_WIDTH,USED_CHECKBOX_HEIGHT);
#endif
        }
        else{
            this->LeftSideWantedTable[index]->Button.setEnabled(false);
            this->LeftSideWantedTable[index]->Button.setVisible(false);
            this->LeftSideWantedTable[index]->CheckBox.setEnabled(false);
            this->LeftSideWantedTable[index]->CheckBox.setVisible(false);
            this->LeftSideWantedTable[index]->CheckBox.setCheckState(Qt::CheckState::Checked);
#ifndef LEFT_SIDE_WANTED_TABLE_ANIMATION
            this->LeftSideWantedTable[index]->Button.setFixedSize(0,0);
            this->LeftSideWantedTable[index]->CheckBox.setFixedSize(0,0);
#endif
        }
    }
    return SUCCESS;
}

int Posefi::ResizeActionTableHeight(){
    int used_table_height = SPACE_FOR_ADD_ACTION_BUTTON;
    for (UINT i = 0; i < ui->actionTable->rowCount(); i++){
        used_table_height += ui->actionTable->rowHeight(i);
    }
    //used_table_height += ui->actionTable->horizontalHeader()->height();

    int visible_table_height = ui->scrollAreaActions->height();
    if (this->ui->scrollAreaActions->horizontalScrollBar()->maximum())
        visible_table_height -= SCROLLBAR_SIZE;

    int h = (visible_table_height > used_table_height) ? visible_table_height : used_table_height;
    ui->scrollAreaActionsContents->setFixedHeight(h-2);
    ui->actionTable->setFixedHeight(h-2);
    return SUCCESS;
}

int Posefi::ResizeActionTableHeader(){
    int w = 0;
    for(UINT i = 0; i < ui->actionTable->columnCount(); i++){
        ui->actionTable->setColumnWidth(i, ui->actionTableHeader->columnWidth(i));
        w += ui->actionTable->columnWidth(i);
    }

    ui->scrollAreaActionsContents->setFixedWidth(w + LEFT_ELEMENT_ACTION_WIDTH);
    ui->actionTable->setFixedWidth(w+2);

    this->ArrangeActionColumnWidths();
    return SUCCESS;
}

int Posefi::ResizeWantedTableHeight(){
    int used_table_height = SPACE_FOR_ADD_WANTED_POS_BUTTON;
    for (UINT i = 0; i < ui->wantedPosTable->rowCount(); i++){
        used_table_height += ui->wantedPosTable->rowHeight(i);
    }
    //used_table_height += ui->wantedPosTable->horizontalHeader()->height();

    int visible_table_height = ui->scrollAreaWantedPos->height();
    if (this->ui->scrollAreaWantedPos->horizontalScrollBar()->maximum())
        visible_table_height -= SCROLLBAR_SIZE;

    int h = (visible_table_height > used_table_height) ? visible_table_height : used_table_height;
    ui->scrollAreaWantedPosContents->setFixedHeight(h-2);
    ui->wantedPosTable->setFixedHeight(h-2);

    return SUCCESS;
}

int Posefi::ResizeWantedTableHeader(){
    int w = 0;
    for(UINT i = 0; i < ui->wantedPosTable->columnCount(); i++){
        ui->wantedPosTable->setColumnWidth(i, ui->wantedPosTableHeader->columnWidth(i));
        w += ui->wantedPosTable->columnWidth(i);
    }
    ui->scrollAreaWantedPosContents->setFixedWidth(w + LEFT_ELEMENT_WANTED_POS_WIDTH);
    ui->wantedPosTable->setFixedWidth(w+2);
    this->ArrangeWantedColumnWidths();
    return SUCCESS;
}

int Posefi::ArrangeActionColumnWidths(){
    int visible_action_table_width = ACTION_TABLE_VISIBLE_WIDTH;
    int used_table_width = -1;
    for(UINT i = 0; i < ui->actionTable->columnCount(); i++){
        used_table_width += ui->actionTable->columnWidth(i);
    }
    int visible_used_difference = visible_action_table_width - used_table_width;
    if (visible_used_difference > 0){
        int index = this->ui->actionTable->columnCount()-1;
        int w = this->ui->actionTable->columnWidth(index) + visible_used_difference;
        this->ui->actionTable->setColumnWidth(index, w);
        this->ui->actionTableHeader->setColumnWidth(index, w);
    }
    else if (visible_used_difference < 0){
        int index = this->ui->actionTable->columnCount()-1;
        int decrease = this->ui->actionTable->columnWidth(index) - this->ActionColumnData[index].column_width;
        if (decrease > 0){
            if ((visible_used_difference + decrease) > 0){
                decrease = -1 * visible_used_difference;
            }
            int w = this->ui->actionTable->columnWidth(index) - decrease;
            this->ui->actionTable->setColumnWidth(index, w);
            this->ui->actionTableHeader->setColumnWidth(index, w);
        }
    }
    this->ResizeActionTableHeight();
    return SUCCESS;
}

int Posefi::ArrangeWantedColumnWidths(){
    int visible_wanted_table_width = WANTED_TABLE_VISIBLE_WIDTH;
    int used_table_width = -1;
    for(UINT i = 0; i < ui->wantedPosTable->columnCount(); i++){
        used_table_width += ui->wantedPosTable->columnWidth(i);
    }
    int visible_used_difference = visible_wanted_table_width - used_table_width;
    if (visible_used_difference > 0){
        int index = this->ui->wantedPosTable->columnCount()-1;
        int w = this->ui->wantedPosTable->columnWidth(index) + visible_used_difference;
        this->ui->wantedPosTable->setColumnWidth(index, w);
        this->ui->wantedPosTableHeader->setColumnWidth(index, w);
    }
    else if (visible_used_difference < 0){
        int index = this->ui->wantedPosTable->columnCount()-1;
        int decrease = this->ui->wantedPosTable->columnWidth(index) - this->WantedColumnData[index].column_width;
        if (decrease > 0){
            if ((visible_used_difference + decrease) > 0){
                decrease = -1 * visible_used_difference;
            }
            int w = this->ui->wantedPosTable->columnWidth(index) - decrease;
            this->ui->wantedPosTable->setColumnWidth(index, w);
            this->ui->wantedPosTableHeader->setColumnWidth(index, w);
        }
    }
    this->ResizeWantedTableHeight();
    return SUCCESS;
}

int Posefi::TopScrollArea_horizontalScroll(int val){
    this->ui->actionTableHeader->setGeometry(LEFT_ELEMENT_ACTION_WIDTH - val, 0, ACTION_TABLE_VISIBLE_WIDTH + val, TABLE_HEADER_HEIGHT);
    ui->actionTableHeader->setFixedWidth(ACTION_TABLE_VISIBLE_WIDTH + val);
    return SUCCESS;
}

int Posefi::BottomScrollArea_horizontalScroll(int val){
    this->ui->wantedPosTableHeader->setGeometry(LEFT_ELEMENT_WANTED_POS_WIDTH - val, 0, WANTED_TABLE_VISIBLE_WIDTH + val, TABLE_HEADER_HEIGHT);
    ui->wantedPosTableHeader->setFixedWidth(WANTED_TABLE_VISIBLE_WIDTH + val);
    return SUCCESS;
}

int Posefi::RemoveActionButton_press(int index){

    UINT x = this->ui->actionTable->rowCount();
    if (index >= x){
        qInfo() << "oor error in Posefi::RemoveActionButton_press(int index)";
        return ARRAY_OUT_OF_RANGE;
    }
    this->ui->actionTable->removeRow(index);

    for(UINT u = index; u < this->LeftSideActionTable.size()-1; u++){
        this->LeftSideActionTable[u]->Used = this->LeftSideActionTable[u+1]->Used;
    }
    ArrangeLeftSideActionTable();
    ResizeActionTableHeight();
    AddActionButtonAnimation();
    SetStartButtonState();

    return SUCCESS;
}

int Posefi::RemoveWantedButton_press(int index){
    UINT x = this->ui->wantedPosTable->rowCount();
    if (index >= x){
        qInfo() << "oor error in Posefi::RemoveWantedButton_press(int index)";
        return ARRAY_OUT_OF_RANGE;
    }
    this->ui->wantedPosTable->removeRow(index);

    for(UINT u = index; u < this->LeftSideWantedTable.size()-1; u++){
        this->LeftSideWantedTable[u]->Used = this->LeftSideWantedTable[u+1]->Used;
    }
    ArrangeLeftSideWantedTable();
    ResizeWantedTableHeight();
    AddWantedButtonAnimation();
    SetStartButtonState();

    return SUCCESS;
}

int Posefi::ActionCheckBox_press(int index){
    if (index >= (int)this->LeftSideActionTable.size()){
        qInfo() << "oor error in Posefi::ActionCheckBox_press(int index)";
        return ARRAY_OUT_OF_RANGE;
    }
    this->LeftSideActionTable[index]->Used = this->LeftSideActionTable[index]->CheckBox.isChecked();
    SetStartButtonState();
    return this->UpdateActionTableCellColor(index, -1);
}

int Posefi::WantedCheckBox_press(int index){
    if (index >= (int)this->LeftSideWantedTable.size()){
        qInfo() << "oor error in Posefi::WantedCheckBox_press(int index)";
        return ARRAY_OUT_OF_RANGE;
    }
    this->LeftSideWantedTable[index]->Used = this->LeftSideWantedTable[index]->CheckBox.isChecked();
    SetStartButtonState();
    return this->UpdateWantedTableCellColor(index, -1);
}

int Posefi::ActionTableCell_edited(int row, int col){
    this->UpdateActionTableCellColor(row, col);
    return SetStartButtonState();
}

int Posefi::WantedTableCell_edited(int row, int col){
    this->UpdateWantedTableCellColor(row, col);
    return SetStartButtonState();
}

// use negative values for row and col to update all rows and/or columns
int Posefi::UpdateActionTableCellColor(int row, int col){
    int row_start = (row < 0) ? 0 : row;
    int row_end = (row < 0) ? this->ui->actionTable->rowCount() - 1 : row;
    int col_start = (col < 0) ? 0 : col;
    int col_end = (col < 0) ? this->ui->actionTable->columnCount() - 1 : col;
    if((row_start) > this->ui->actionTable->rowCount() - 1 || (col_start) > this->ui->actionTable->columnCount() - 1)
        return ARRAY_OUT_OF_RANGE;

    disconnect(this->ui->actionTable, SIGNAL(cellChanged(int, int)), this, SLOT(ActionTableCell_edited(int,int)));

    for (UINT r = row_start; r <= row_end; r++){
        for (UINT c = col_start; c <= col_end; c++){
            QString s = this->ui->actionTable->item(r,c)->text();
            bool chk;
            switch(ActionColumnData[c].string_case){// IS THIS CORRECT
            case HEX:
                chk = HexCheck(s);
                break;
            case DEC:
                chk = DecCheck(s);
                break;
            case UDEC:
                chk = (DecCheck(s,false));
                break;
            default:
                chk = true;
                break;
            }
            LeftSideActionTable[r]->is_entry_valid[c] = chk;
            bool usd = true;
            if (!LeftSideActionTable[r]->Used
                    || (this->ActionColumnData[c].xyz_case == IS_XYZ_FIRST && !this->find_xyz_first)
                    || (this->ActionColumnData[c].xyz_case == IS_XYZ_SECOND && !this->find_xyz_second))
                usd = false;

            if(chk)
            {
                if(usd)
                {
                    ui->actionTable->item(r,c)->setBackground(table_normal_color);
                    ui->actionTable->item(r,c)->setForeground(text_normal_color);
                }
                else
                {
                    ui->actionTable->item(r,c)->setBackground(table_grey_color);
                    ui->actionTable->item(r,c)->setForeground(text_grey_color);
                }
            }
            else
            {
                if(usd)
                {
                    ui->actionTable->item(r,c)->setBackground(table_error_color);
                    ui->actionTable->item(r,c)->setForeground(text_error_color);
                }
                else
                {
                    ui->actionTable->item(r,c)->setBackground(table_error_grey_color);
                    ui->actionTable->item(r,c)->setForeground(text_error_grey_color);
                }
            }
        }
    }

    connect(this->ui->actionTable, SIGNAL(cellChanged(int,int)), this, SLOT(ActionTableCell_edited(int,int)));
    return SUCCESS;
}

int Posefi::UpdateWantedTableCellColor(int row, int col){
    int row_start = (row < 0) ? 0 : row;
    int row_end = (row < 0) ? this->ui->wantedPosTable->rowCount() - 1 : row;
    int col_start = (col < 0) ? 0 : col;
    int col_end = (col < 0) ? this->ui->wantedPosTable->columnCount() - 1 : col;
    if((row_start) > this->ui->wantedPosTable->rowCount() - 1 || (col_start) > this->ui->wantedPosTable->columnCount() - 1)
        return ARRAY_OUT_OF_RANGE;

    disconnect(this->ui->wantedPosTable, SIGNAL(cellChanged(int, int)), this, SLOT(WantedTableCell_edited(int,int)));

    for (UINT r = row_start; r <= row_end; r++){
        for (UINT c = col_start; c <= col_end; c++){
            QString s = this->ui->wantedPosTable->item(r,c)->text();
            bool chk;
            switch(WantedColumnData[c].string_case){// IS THIS CORRECT
            case HEX:
                chk = HexCheck(s);
                break;
            case DEC:
                chk = DecCheck(s);
                break;
            case UDEC:
                chk = (DecCheck(s,false));
                break;
            default:
                chk = true;
                break;
            }
            this->LeftSideWantedTable[r]->is_entry_valid[c] = chk;
            bool usd = true;
            if (!this->LeftSideWantedTable[r]->Used
                    || (this->WantedColumnData[c].xyz_case == IS_XYZ_FIRST && !this->find_xyz_first)
                    || (this->WantedColumnData[c].xyz_case == IS_XYZ_SECOND && !this->find_xyz_second))
                usd = false;
            if (chk && usd)
            {
                this->ui->wantedPosTable->item(r,c)->setBackground(this->table_normal_color);
                this->ui->wantedPosTable->item(r,c)->setForeground(this->text_normal_color);
            }
            else if (!chk && usd)
            {
                this->ui->wantedPosTable->item(r,c)->setBackground(this->table_error_color);
                this->ui->wantedPosTable->item(r,c)->setForeground(this->text_error_color);
            }
            else if (chk && !usd)
            {
                this->ui->wantedPosTable->item(r,c)->setBackground(this->table_grey_color);
                this->ui->wantedPosTable->item(r,c)->setForeground(this->text_grey_color);
            }
            else
            {
                this->ui->wantedPosTable->item(r,c)->setBackground(this->table_error_grey_color);
                this->ui->wantedPosTable->item(r,c)->setForeground(this->text_error_grey_color);
            }
        }
    }

    connect(this->ui->wantedPosTable, SIGNAL(cellChanged(int,int)), this, SLOT(WantedTableCell_edited(int,int)));
    return SUCCESS;
}

int Posefi::FindXZ_RadioButton_toggle(int id){

    bool xyz_first_changed = ((id == 1 && this->find_xyz_first) || (id != 1 && !this->find_xyz_first));
    bool xyz_second_changed = ((id == 0 && this->find_xyz_second) || (id != 0 && !this->find_xyz_second));

    if (!(xyz_first_changed || xyz_second_changed))
        return TOGGLE_UNCHANGED;

    if((int)this->LeftSideActionTable.size() < this->ui->actionTable->rowCount()){
        qInfo() << "array out of range error in FindXZ_RadioButton_toggle(int id), action table row count:" << this->ui->actionTable->rowCount() << ", left side element count: " << this->LeftSideActionTable.size();
        return ARRAY_OUT_OF_RANGE;
    }

    this->find_xyz_first = (id != 1);
    this->find_xyz_second = (id != 0);

    if (this->ui->actionTable->rowCount() > 0){
        for (int i = 0; i < this->ui->actionTable->columnCount(); i++){
            if ((this->ActionColumnData[i].xyz_case == IS_XYZ_FIRST && xyz_first_changed) || (this->ActionColumnData[i].xyz_case == IS_XYZ_SECOND && xyz_second_changed))
                this->UpdateActionTableCellColor(-1, i);
        }
    }
    if (this->ui->wantedPosTable->rowCount() > 0){

        for (int i = 0; i < this->ui->wantedPosTable->columnCount(); i++){
            if ((this->WantedColumnData[i].xyz_case == IS_XYZ_FIRST && xyz_first_changed) || (this->WantedColumnData[i].xyz_case == IS_XYZ_SECOND && xyz_second_changed))
                this->UpdateWantedTableCellColor(-1, i);
        }
    }
    SetStartButtonState();
    return SUCCESS;
}

int Posefi::LabelXYZ_RadioButton_toggle(int id){
    this->ui->actionTableHeader->setHorizontalHeaderLabels(action_table_lables[id]);
    this->ui->wantedPosTableHeader->setHorizontalHeaderLabels(wanted_table_lables[id]);
    this->ui->radioButton_xyz_first->setText(find_first_labels[id]);
    this->ui->radioButton_xyz_second->setText(find_second_labels[id]);
    this->ui->checkBoxX_Resettable->setText(resettable_first_labels[id]);
    this->ui->checkBoxZ_Resettable->setText(resettable_second_labels[id]);
    this->ui->labelDefaultFirst->setText(this->default_pos_first_labels[id]);
    this->ui->labelDefaultSecond->setText(this->default_pos_second_labels[id]);
    return SUCCESS;
}

int Posefi::ResizeSplitterRightWidth(){
    int w = this->ui->rightSideMainFrame->width() - GAP_RIGHT;
    this->ui->splitterRight->setFixedWidth(w);
    this->ui->topRightFrame->setFixedWidth(w);
    this->ui->partialSolutionsFrame->setFixedWidth(w);
    this->ui->subSearchFrame->setFixedWidth(w);
    this->ui->fullSolutionsFrame->setFixedWidth(w);
    w -= (2*RS_SCROLL_AREA_MOVE);

    this->ui->scrollAreaTopRight->setFixedWidth(w);
    this->ui->scrollAreaPartialSolutions->setFixedWidth(w);
    this->ui->scrollAreaSubSearch->setFixedWidth(w);
    this->ui->scrollAreaFullSolutions->setFixedWidth(w);
    this->ui->partialSolutionsPageFrame->setFixedWidth(w);
    this->ui->fullSolutionsPageFrame->setFixedWidth(w);

    int x = (w - this->ui->partialSolutionsPageSubFrame->width()) / 2;
    int x_min = this->ui->partialSolutionsShowLabel->width() + this->ui->partialSolutionsShowLabel->x();
    if(x < x_min)
        x = x_min;
    this->ui->partialSolutionsPageSubFrame->move(x,this->ui->partialSolutionsPageSubFrame->y());
    this->ResizePartialSolutionsWidth();

    x = (w - this->ui->fullSolutionsPageSubFrame->width()) / 2;
    x_min = this->ui->fullSolutionsShowLabel->width() + this->ui->fullSolutionsShowLabel->x();
    if(x < x_min)
        x = x_min;
    this->ui->fullSolutionsPageSubFrame->move(x,this->ui->fullSolutionsPageSubFrame->y());
    this->ResizeFullSolutionsWidth();

    //Solution edit buttons
    UpdateEditButtonGeometry();

    return SUCCESS;
}

int Posefi::ResizeSplitterRightHeight(){
    int h1 = this->ui->topRightFrame->height() - RS_HEADER_LABEL_HEIGHT - (2*RS_SCROLL_AREA_MOVE);
    int h2 = this->ui->partialSolutionsFrame->height() - RS_HEADER_LABEL_HEIGHT - PAGE_NAVIGATION_HEIGHT - (2*RS_SCROLL_AREA_MOVE);
    int h3 = this->ui->subSearchFrame->height() - RS_HEADER_LABEL_HEIGHT - (2*RS_SCROLL_AREA_MOVE);
    int h4 = this->ui->fullSolutionsFrame->height() - RS_HEADER_LABEL_HEIGHT - PAGE_NAVIGATION_HEIGHT - (2*RS_SCROLL_AREA_MOVE);

    this->ui->scrollAreaTopRight->setFixedHeight(h1);
    this->ui->scrollAreaPartialSolutions->setFixedHeight(h2);
    this->ui->scrollAreaSubSearch->setFixedHeight(h3);
    this->ui->scrollAreaFullSolutions->setFixedHeight(h4);

    this->ui->partialSolutionsPageFrame->move(
                RS_SCROLL_AREA_MOVE,this->ui->partialSolutionsFrame->height()
                - this->ui->partialSolutionsPageFrame->height() - RS_SCROLL_AREA_MOVE);
    this->ui->fullSolutionsPageFrame->move(
                RS_SCROLL_AREA_MOVE,this->ui->fullSolutionsFrame->height()
                - this->ui->fullSolutionsPageFrame->height() - RS_SCROLL_AREA_MOVE);
    return SUCCESS;
}

void Posefi::UpdateEditButtonGeometry()
{
    int spaceLeftRight = 30;
    int gap = 20;
    int right = ui->fullSolutionsPageFrame->width();
    int sortBtnLeft = right - ui->fullSolutionsDeleteButton->width() - spaceLeftRight;
    int sortBtnLeftMin = ui->fullSolutionsPageSubFrame->geometry().right() + spaceLeftRight + 70;
    if(sortBtnLeft < sortBtnLeftMin)
        sortBtnLeft = sortBtnLeftMin;
    int deleteBtnLeft = sortBtnLeft - gap - ui->fullSolutionsSortButton->width();
    int btnTop = (ui->fullSolutionsPageFrame->height() - ui->fullSolutionsSortButton->height()) / 2;
    ui->fullSolutionsDeleteButton->move(deleteBtnLeft, btnTop);
    ui->partialSolutionsDeleteButton->move(deleteBtnLeft, btnTop);
    ui->fullSolutionsSortButton->move(sortBtnLeft, btnTop);
    ui->partialSolutionsSortButton->move(sortBtnLeft, btnTop);
}

// ------ partial and full solution element stuff ---------------

// page change animation bug fix

// Call this function to display new solutions, either when a new solution got found or if an older solution got deleted.
int Posefi::ShowNewPartialSolutions(){

    // if a page change is coming up, but not all elements have disappeared yet,
    // don't update any new solutions yet.
    if(this->upcoming_ps_page_change && this->current_ps_disable_animations != 0){
        return SUCCESS;
    }

    this->ManagePartialSolutionPages();

    if(ps_indices.empty() || current_ps_page < 0)
        return SUCCESS;

    uint ps_indices_start = this->current_ps_page * this->solutions_per_page;
    if(ps_indices_start >= ps_indices.size()){
        qInfo() << "Error in Posefi::UpdatePartialSolutions(): ARRAY_OUT_OF_RANGE";
        return ARRAY_OUT_OF_RANGE;
    }

    // Determine the count of PartialSolutionElements that are free to display new solutions.
    // The count is the sum of old disabled elements and elements that have not been created yet.
    // An old element is free if it got disabled and its disappearing animation has ended.
    int free_slots = this->solutions_per_page - this->PSE_Order.size();
    for(int i = this->PSE_Order.size()-1; i >= 0; i--){
        int pse_index = this->PSE_Order[i];
        if(pse_index >= 0){// positive index means element is used, and so are all the previous elements.
            ps_indices_start += i+1;
            break;
        }
        pse_index = -pse_index -1;
        PartialSolutionElement* pse = this->PartialSolutionElements[pse_index];
        if(pse->opacity_animation.currentTime() == pse->opacity_animation.totalDuration()){
            free_slots++;
        }
    }

    int following_solutions = ps_indices.size() - ps_indices_start;

    if(free_slots < 0 || following_solutions < 0){
        qInfo() << "Error in Posefi::UpdatePartialSolutions(): (free_slots < 0 || following_solutions < 0)";
    }

    uint display_new_count = (free_slots < following_solutions) ? free_slots : following_solutions;
    for(uint i = 0; i < display_new_count; i++){
        int ps_copy_index = this->ps_indices[ps_indices_start + i];
        if(ps_copy_index < 0 || (uint)ps_copy_index >= this->partial_solution_copies.size()){
            qInfo() << "Error in Posefi::UpdatePartialSolutions(): (ps_copy_index < 0 || (uint)ps_copy_index >= this->partial_solution_copies.size())";
            continue;
        }
        this->AddPartialSolution(&this->partial_solution_copies[ps_copy_index]);
    }

    this->upcoming_ps_page_change = false; // If this has been true, the page change is done now.
    // (this->AddPartialSolution and this->CreatePartialSolutionElement check it to see which animation an element should appear in)

    this->ManagePartialSolutionPages();

    return SUCCESS;
}

int Posefi::ShowNewFullSolutions(){

    // if a page change is coming up, but not all elements have disappeared yet,
    // don't update any new solutions yet.
    if(this->upcoming_fs_page_change && this->current_fs_disable_animations != 0){
        return SUCCESS;
    }

    this->ManageFullSolutionPages();

    if(fs_indices.empty() || current_fs_page < 0)
        return SUCCESS;

    uint fs_indices_start = this->current_fs_page * this->solutions_per_page;
    if(fs_indices_start >= fs_indices.size()){
        qInfo() << "Error in Posefi::ShowNewFullSolutions(): ARRAY_OUT_OF_RANGE";
        return ARRAY_OUT_OF_RANGE;
    }

    // Determine the count of FullSolutionElements that are free to display new solutions.
    // The count is the sum of old disabled elements and elements that have not been created yet.
    // An old element is free if it got disabled and its disappearing animation has ended.
    int free_slots = this->solutions_per_page - this->FSE_Order.size();
    for(int i = this->FSE_Order.size()-1; i >= 0; i--){
        int fse_index = this->FSE_Order[i];
        if(fse_index >= 0){// positive index means element is used, and so are all the previous elements.
            fs_indices_start += i+1;
            break;
        }
        fse_index = -fse_index -1;
        SolutionElement* fse = this->FullSolutionElements[fse_index];
        if(fse->opacity_animation.currentTime() == fse->opacity_animation.totalDuration()){   // ERROR
            free_slots++;
        }
    }

    int following_solutions = fs_indices.size() - fs_indices_start;

    if(free_slots < 0 || following_solutions < 0){
        qInfo() << "Error in Posefi::ShowNewFullSolutions(): (free_slots < 0 || following_solutions < 0)";
    }

    uint display_new_count = (free_slots < following_solutions) ? free_slots : following_solutions;
    for(uint i = 0; i < display_new_count; i++){
        int fs_copy_index = this->fs_indices[fs_indices_start + i];
        if(fs_copy_index < 0 || (uint)fs_copy_index >= this->full_solution_copies.size()){
            qInfo() << "Error in Posefi::ShowNewFullSolutions(): (fs_copy_index < 0 || (uint)fs_copy_index >= this->full_solution_copies.size())";
            continue;
        }
        this->AddFullSolution(&this->full_solution_copies[fs_copy_index]);
    }

    this->upcoming_fs_page_change = false; // If this has been true, the page change is done now.
    // (this->AddFullSolution and this->CreateFullSolutionElement check it to see which animation an element should appear in)

    this->ManageFullSolutionPages();

    return SUCCESS;
}

// create a new partial solution element. Only do this if the max is not reached. Once max is reached, work with pages
int Posefi::CreatePartialSolutionElement(SolutionCopy* sc){
    uint element_index = this->PartialSolutionElements.size();
    if(element_index >= this->solutions_per_page){
        qInfo() << "ARRAY_AT_MAX error in Posefi::CreatePartialSolutionElement()";
        return ARRAY_AT_MAX;
    }

    this->PartialSolutionElements.push_back(new PartialSolutionElement);
    this->PartialSolutionElements.back()->order_index = element_index;
    this->PartialSolutionElements.back()->part_sol_copy = sc;
    this->PSE_Order.push_back(element_index);

    PartialSolutionElement* pse = this->PartialSolutionElements.back();

    // set parents
    pse->top_frame.setParent(this->ui->scrollAreaPartialSolutionsContents);
    pse->bottom_frame.setParent(&pse->top_frame);
    pse->delete_button.setParent(&pse->top_frame);
    pse->delete_button.setObjectName("removeBtn");
    pse->fold_button.setParent(&pse->top_frame);
    pse->sub_search_button.setParent(&pse->bottom_frame);
    pse->header_label.setParent(&pse->top_frame);
    pse->solution_description_label.setParent(&pse->bottom_frame);

    // connect animation to top_frame
    pse->geometry_animation.setTargetObject(&pse->top_frame);
    pse->geometry_animation.setPropertyName("geometry");
    pse->opacity_effect.setParent(&pse->top_frame);
    pse->opacity_animation.setTargetObject(&pse->opacity_effect);
    pse->opacity_animation.setPropertyName("opacity");
    pse->top_frame.setGraphicsEffect(&pse->opacity_effect);
    pse->opacity_animation.setStartValue(0);
    pse->opacity_animation.setEndValue(1);
    pse->opacity_animation.setDuration(PSE_FROM_RIGHT_ANIMATION_DURATION);

    // set geometries
    int top_frame_x_pos = SE_TOP_FRAME_GAP;
    int top_frame_y_pos = CalculatePartialSolutionElement_YPos(element_index);
    int top_frame_width = this->ui->scrollAreaPartialSolutionsContents->width() - 2*SE_TOP_FRAME_GAP;
    int top_frame_height = SE_FOLDED_HEIGHT;
    pse->top_frame.setGeometry(top_frame_x_pos,top_frame_y_pos,top_frame_width,top_frame_height);
    pse->header_label.setGeometry(PSE_HEADER_LABEL_GEOMETRY);
    pse->fold_button.setGeometry(0,0,pse->top_frame.width() - SE_FOLD_BUTTON_GAP_RIGHT,SE_FOLDED_HEIGHT);
    pse->fold_button.setStyleSheet(SE_FOLD_BUTTON_CSS);
    pse->delete_button.setFixedSize(PSE_DELETE_BUTTON_SIZE);
    int del_x = pse->top_frame.width() - pse->delete_button.width() - PSE_DELETE_BUTTON_GAP_RIGHT;
    pse->delete_button.move(del_x,PSE_DELETE_BUTTON_Y);
    pse->solution_description_label.move(SE_SOLUTION_LABEL_MOVE);
    pse->sub_search_button.setGeometry(PSE_SUB_SEARCH_BUTTON_GEOMETRY);

    // set z-order
    pse->fold_button.stackUnder(&pse->delete_button);
    pse->header_label.stackUnder(&pse->fold_button);

    int bottom_frame_x_pos = PSE_BOTTOM_FRAME_GAP;
    int bottom_frame_y_pos = SE_HEADER_HEIGHT;
    int bottom_frame_width = top_frame_width - 2*PSE_BOTTOM_FRAME_GAP;
    pse->bottom_frame.move(bottom_frame_x_pos,bottom_frame_y_pos);
    pse->bottom_frame.setFixedWidth(bottom_frame_width);

    pse->solution_description_label.setTextInteractionFlags(Qt::TextSelectableByMouse);
    pse->SetText();

    pse->sub_search_button.setText(PSE_SUB_SEARCH_BUTTON_TEXT);

    pse->top_frame.show();

    // connect signals
    connect(&pse->fold_button, SIGNAL(clicked()), &pse->fold_button_signal_mapper, SLOT(map()));
    pse->fold_button_signal_mapper.setMapping(&pse->fold_button, element_index);
    connect(&pse->fold_button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(PartialSolutionFoldButton_press(int)));
    connect(&pse->delete_button, SIGNAL(clicked()), &pse->delete_button_signal_mapper, SLOT(map()));
    pse->delete_button_signal_mapper.setMapping(&pse->delete_button, element_index);
    connect(&pse->delete_button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(PartialSolutionDeleteButton_press(int)));
    connect(&pse->sub_search_button, SIGNAL(clicked()), &pse->sub_search_button_signal_mapper, SLOT(map()));
    pse->sub_search_button_signal_mapper.setMapping(&pse->sub_search_button, element_index);
    connect(&pse->sub_search_button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(PartialSolutionSubSearchButton_press(int)));
    connect(&pse->opacity_animation, SIGNAL(finished()), &pse->opacity_ani_end_signal_mapper,SLOT(map()));
    pse->opacity_ani_end_signal_mapper.setMapping(&pse->opacity_animation, element_index);
    connect(&pse->opacity_ani_end_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(PSE_OpacityAnimationEnd(int)));
    connect(&pse->geometry_animation, SIGNAL(finished()), &pse->geometry_ani_end_signal_mapper,SLOT(map()));
    pse->geometry_ani_end_signal_mapper.setMapping(&pse->geometry_animation, element_index);
    connect(&pse->geometry_ani_end_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(PSE_GeometryAnimationEnd(int)));

    // animation to make the element appear from the right
    QPropertyAnimation* ani = &pse->geometry_animation;
    ani->setDuration(PSE_FROM_RIGHT_ANIMATION_DURATION);
    ani->setStartValue(QRect(PSE_FROM_RIGHT_ANIMATION_X1,top_frame_y_pos,top_frame_width, top_frame_height));
    ani->setEndValue(QRect(top_frame_x_pos,top_frame_y_pos, top_frame_width, top_frame_height));
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->start();
    pse->opacity_animation.start();

    this->AdjustPartialSolutionScrollAreaHeight();

    return SUCCESS;
}

// create a new full solution element. Only do this if the max is not reached. Once max is reached, work with pages
int Posefi::CreateFullSolutionElement(SolutionCopy* sc){
    uint element_index = this->FullSolutionElements.size();
    if(element_index >= this->solutions_per_page){
        qInfo() << "ARRAY_AT_MAX error in Posefi::CreateFullSolutionElement()";
        return ARRAY_AT_MAX;
    }

    this->FullSolutionElements.push_back(new SolutionElement);
    this->FullSolutionElements.back()->order_index = element_index;
    this->FullSolutionElements.back()->full_sol_copy = sc;
    this->FSE_Order.push_back(element_index);

    SolutionElement* fse = this->FullSolutionElements.back();

    // set parents
    fse->top_frame.setParent(this->ui->scrollAreaFullSolutionsContents);
    fse->bottom_frame.setParent(&fse->top_frame);
    fse->delete_button.setParent(&fse->top_frame);
    fse->delete_button.setObjectName("removeBtn");
    fse->fold_button.setParent(&fse->top_frame);
    fse->header_label.setParent(&fse->top_frame);
    fse->solution_description_label.setParent(&fse->bottom_frame);

    // connect animation to top_frame
    fse->geometry_animation.setTargetObject(&fse->top_frame);
    fse->geometry_animation.setPropertyName("geometry");
    fse->opacity_effect.setParent(&fse->top_frame);
    fse->opacity_animation.setTargetObject(&fse->opacity_effect);
    fse->opacity_animation.setPropertyName("opacity");
    fse->top_frame.setGraphicsEffect(&fse->opacity_effect);
    fse->opacity_animation.setStartValue(0);
    fse->opacity_animation.setEndValue(1);
    fse->opacity_animation.setDuration(FSE_FROM_RIGHT_ANIMATION_DURATION);

    // set geometries
    int top_frame_x_pos = SE_TOP_FRAME_GAP;
    int top_frame_y_pos = CalculateFullSolutionElement_YPos(element_index);
    int top_frame_width = this->ui->scrollAreaFullSolutionsContents->width() - 2*SE_TOP_FRAME_GAP;
    int top_frame_height = SE_FOLDED_HEIGHT;
    fse->top_frame.setGeometry(top_frame_x_pos,top_frame_y_pos,top_frame_width,top_frame_height);
    fse->header_label.setGeometry(FSE_HEADER_LABEL_GEOMETRY);
    fse->fold_button.setGeometry(0,0,fse->top_frame.width() - SE_FOLD_BUTTON_GAP_RIGHT,SE_FOLDED_HEIGHT);
    fse->fold_button.setStyleSheet(SE_FOLD_BUTTON_CSS);
    fse->delete_button.setFixedSize(FSE_DELETE_BUTTON_SIZE);
    int del_x = fse->top_frame.width() - fse->delete_button.width() - FSE_DELETE_BUTTON_GAP_RIGHT;
    fse->delete_button.move(del_x,FSE_DELETE_BUTTON_Y);
    fse->solution_description_label.move(SE_SOLUTION_LABEL_MOVE);

    // set z-order
    fse->fold_button.stackUnder(&fse->delete_button);
    fse->header_label.stackUnder(&fse->fold_button);

    int bottom_frame_x_pos = FSE_BOTTOM_FRAME_GAP;
    int bottom_frame_y_pos = SE_HEADER_HEIGHT;
    int bottom_frame_width = top_frame_width - 2*FSE_BOTTOM_FRAME_GAP;
    fse->bottom_frame.move(bottom_frame_x_pos,bottom_frame_y_pos);
    fse->bottom_frame.setFixedWidth(bottom_frame_width);

    fse->solution_description_label.setTextInteractionFlags(Qt::TextSelectableByMouse);
    fse->SetText();

    fse->top_frame.show();

    // connect signals
    connect(&fse->fold_button, SIGNAL(clicked()), &fse->fold_button_signal_mapper, SLOT(map()));
    fse->fold_button_signal_mapper.setMapping(&fse->fold_button, element_index);
    connect(&fse->fold_button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(FullSolutionFoldButton_press(int)));
    connect(&fse->delete_button, SIGNAL(clicked()), &fse->delete_button_signal_mapper, SLOT(map()));
    fse->delete_button_signal_mapper.setMapping(&fse->delete_button, element_index);
    connect(&fse->delete_button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(FullSolutionDeleteButton_press(int)));
    connect(&fse->opacity_animation, SIGNAL(finished()), &fse->opacity_ani_end_signal_mapper,SLOT(map()));
    fse->opacity_ani_end_signal_mapper.setMapping(&fse->opacity_animation, element_index);
    connect(&fse->opacity_ani_end_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(FSE_OpacityAnimationEnd(int)));
    connect(&fse->geometry_animation, SIGNAL(finished()), this, SLOT(TriggerMouseHoverCheck()));
    connect(&fse->geometry_animation, SIGNAL(finished()), &fse->geometry_ani_end_signal_mapper,SLOT(map()));
    fse->geometry_ani_end_signal_mapper.setMapping(&fse->geometry_animation, element_index);
    connect(&fse->geometry_ani_end_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(FSE_GeometryAnimationEnd(int)));

    // animation to make the element appear from the right
    QPropertyAnimation* ani = &fse->geometry_animation;
    ani->setDuration(FSE_FROM_RIGHT_ANIMATION_DURATION);
    ani->setStartValue(QRect(FSE_FROM_RIGHT_ANIMATION_X1,top_frame_y_pos,top_frame_width, top_frame_height));
    ani->setEndValue(QRect(top_frame_x_pos,top_frame_y_pos, top_frame_width, top_frame_height));
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->start();
    fse->opacity_animation.start();

    this->AdjustFullSolutionScrollAreaHeight();

    return SUCCESS;
}

int Posefi::AddPartialSolution(SolutionCopy* sc){

    if(this->PSE_Order.size() == 0 || this->PSE_Order.back() >= 0){
        // negative at the end of the order array means at least one present element is disabled
        // if there is not, create new element
        return this->CreatePartialSolutionElement(sc);
    }

    // reactivate a disabled element:
    int order_index = 0;
    int element_index;
    for(int i = 0; i < (int)this->PSE_Order.size(); i++){
        if(PSE_Order[i] < 0){
            order_index = i;
            break;
        }
    }

    element_index = -(PSE_Order[order_index] + 1);

    if(element_index < 0){
        qInfo() << "Error in Posefi::AddPartialSolution: Trying to reactivate element when there is none to reactivate.";
        return ARRAY_OUT_OF_RANGE;
    }

    PartialSolutionElement* pse = this->PartialSolutionElements[element_index];

    this->PSE_Order[order_index] = element_index;
    pse->order_index = order_index;
    pse->part_sol_copy = sc;

    pse->top_frame.setEnabled(true);
    pse->top_frame.show();

    // change the z-order: put new element under the previous one
    if(order_index > 0){
        int ei_prev = this->PSE_Order[order_index - 1];
        if(ei_prev < 0 || ei_prev >= (int)this->PartialSolutionElements.size()){
            qInfo() << "error in Posefi::AddPartialSolution: (ei_prev < 0 || ei_prev >= (int)this->PartialSolutionElements.size())";
        }
        else{
            pse->top_frame.stackUnder(&this->PartialSolutionElements[ei_prev]->top_frame);
        }
    }

    //if element was deleted when unfolded it is still in that state and needs to be folded again
    if(!pse->folded){
        pse->folded = true;
        pse->top_frame.setGeometry(pse->top_frame.x(),pse->top_frame.y(),pse->top_frame.width(),SE_FOLDED_HEIGHT);
    }

    pse->delete_button.setEnabled(true);
    pse->fold_button.setEnabled(true);
    //pse->sub_search_button.setEnabled(true);  //TODO set enabled if sub search possible

    // solution header text and action text
    pse->SetText();

    // animation
    if(this->upcoming_ps_page_change){
        this->PSE_PageChangeFadeInAnimation(pse);
    }
    else{
        this->PSE_AppearFromRightAnimation(pse);
    }

    this->AdjustPartialSolutionScrollAreaHeight();

    return SUCCESS;
}

int Posefi::AddFullSolution(SolutionCopy* sc){

    if(this->FSE_Order.size() == 0 || this->FSE_Order.back() >= 0){
        // negative at the end of the order array means at least one present element is disabled
        // if there is not, create new element
        return this->CreateFullSolutionElement(sc);
    }

    // reactivate a disabled element:
    int order_index = 0;
    int element_index;
    for(int i = 0; i < (int)this->FSE_Order.size(); i++){
        if(FSE_Order[i] < 0){
            order_index = i;
            break;
        }
    }

    element_index = -(FSE_Order[order_index] + 1);

    if(element_index < 0){
        qInfo() << "Error in Posefi::AddFullSolution: Trying to reactivate element when there is none to reactivate.";
        return ARRAY_OUT_OF_RANGE;
    }

    SolutionElement* fse = this->FullSolutionElements[element_index];

    this->FSE_Order[order_index] = element_index;
    fse->order_index = order_index;
    fse->full_sol_copy = sc;

    fse->top_frame.setEnabled(true);
    fse->top_frame.show();

    // change the z-order: put new element under the previous one
    if(order_index > 0){
        int ei_prev = this->FSE_Order[order_index - 1];
        if(ei_prev < 0 || ei_prev >= (int)this->FullSolutionElements.size()){
            qInfo() << "error in Posefi::AddFullSolution: (ei_prev < 0 || ei_prev >= (int)this->FullSolutionElements.size())";
        }
        else{
            fse->top_frame.stackUnder(&this->FullSolutionElements[ei_prev]->top_frame);
        }
    }

    //if element was deleted when unfolded it is still in that state and needs to be folded again
    if(!fse->folded){
        fse->folded = true;
        fse->top_frame.setGeometry(fse->top_frame.x(),fse->top_frame.y(),fse->top_frame.width(),SE_FOLDED_HEIGHT);
    }

    fse->delete_button.setEnabled(true);
    fse->fold_button.setEnabled(true);

    // solution header text and action text
    fse->SetText();

    // animation
    if(this->upcoming_fs_page_change){
        this->FSE_PageChangeFadeInAnimation(fse);
    }
    else{
        this->FSE_AppearFromRightAnimation(fse);
    }

    this->AdjustFullSolutionScrollAreaHeight();

    return SUCCESS;
}

int SolutionElement::SetText(){

    QString label_header;

    if(this->full_sol_copy != nullptr){
        // full solution header
        label_header = this->full_sol_copy->searched_name; // crash
        label_header += SE_HEADER_SEPARATOR_STRING;
        label_header += SE_HEADER_COST_STRING;
        int costs = this->full_sol_copy->costs;
        if(this->part_sol_copy != nullptr)
            costs += this->part_sol_copy->costs;
        label_header += QString::number(costs,10);
    }
    else if(this->part_sol_copy != nullptr){
        // partial solution header
        label_header = this->part_sol_copy->searched_name;
        label_header += SE_HEADER_SEPARATOR_STRING;
        label_header += SE_HEADER_FOUND_STRING;
        if(this->part_sol_copy->found_first)
            label_header += this->part_sol_copy->xyz_first_string;
        else
            label_header += this->part_sol_copy->xyz_second_string;
        label_header += SE_HEADER_SEPARATOR_STRING;
        label_header += SE_HEADER_COST_STRING;
        label_header += QString::number(this->part_sol_copy->costs,10);
    }
    else{
        label_header = "error: solution == nullptr\nInform Türkenheimer.";
        return NULL_POINTER;
    }
    if(header_label.text() != label_header)
        this->header_label.setText(label_header);

    WORD default1 = NULL_VALUE;
    WORD default2 = NULL_VALUE;
    WORD searched1 = NULL_VALUE;
    WORD searched2 = NULL_VALUE;
    WORD result1 = NULL_VALUE;
    WORD result2 = NULL_VALUE;
    QString xyz_first_str = "";
    QString xyz_second_str = "";

    QString label;
    SolutionCopy* sc;
    bool goto_partial_solution_actions = (this->part_sol_copy != nullptr);
    int bottom_frame_height;

    if(this->full_sol_copy != nullptr && this->part_sol_copy != nullptr){
        label = "test string:\n123456789\nabcdefghijklmnopqrstuvwxyz\nasdfghjklöä";
        goto done;
    }

    if(this->full_sol_copy != nullptr){ // if full solution

        if(this->part_sol_copy != nullptr){ // if full solution resulted from a sub-search
            // a sub search always searches for both coordinates
            default1 = this->part_sol_copy->default_pos1;
            default2 = this->part_sol_copy->default_pos2;
            searched1 = this->part_sol_copy->searched_pos1;
            searched2 = this->part_sol_copy->searched_pos2;
            result1 = this->part_sol_copy->found_pos1;
            result2 = this->part_sol_copy->found_pos2;
            xyz_first_str = this->part_sol_copy->xyz_first_string;
            xyz_second_str = this->part_sol_copy->xyz_second_string;
        }
        else{ // if full solution resulted from a main-search
            // the main search can search for both or only one coordinate
            if(this->full_sol_copy->found_first){
                default1 = this->full_sol_copy->default_pos1;
                searched1 = this->full_sol_copy->searched_pos1;
                result1 = this->full_sol_copy->found_pos1;
                xyz_first_str = this->full_sol_copy->xyz_first_string;
            }
            if(this->full_sol_copy->found_second){
                default2 = this->full_sol_copy->default_pos2;
                searched2 = this->full_sol_copy->searched_pos2;
                result2 = this->full_sol_copy->found_pos2;
                xyz_second_str = this->full_sol_copy->xyz_second_string;
            }
        }
    }
    else if(this->part_sol_copy != nullptr){ // if partial solution
        // partial solutions always result from a search that searches both
        default1 = this->part_sol_copy->default_pos1;
        default2 = this->part_sol_copy->default_pos2;
        searched1 = this->part_sol_copy->searched_pos1;
        searched2 = this->part_sol_copy->searched_pos2;
        result1 = this->part_sol_copy->found_pos1;
        result2 = this->part_sol_copy->found_pos2;
        xyz_first_str = this->part_sol_copy->xyz_first_string;
        xyz_second_str = this->part_sol_copy->xyz_second_string;
    }

    // default position
    label = SE_DEFAULT_POS_STRING;
    if(searched1 != NULL_VALUE){
        label += xyz_first_str + SE_COLON_STRING;
        label += (default1 != 0) ? QString::number(default1,16) : SE_ZERO_POS_STRING;
        if(searched2 != NULL_VALUE)
            label += SE_SPACE_STRING;
    }
    if(searched2 != NULL_VALUE){
        label += xyz_second_str + SE_COLON_STRING;
        label += (default2 != 0) ? QString::number(default2,16) : SE_ZERO_POS_STRING;
    }

    // searched position
    label += "\n";
    label += SE_SEARCHED_POS_STRING;
    if(searched1 != NULL_VALUE){
        label += xyz_first_str + SE_COLON_STRING;
        label += (searched1 != 0) ? AdjustHexString(QString::number(searched1,16), 8, "0") : SE_ZERO_POS_STRING;
        if(searched2 != NULL_VALUE)
            label += SE_SPACE_STRING;
    }
    if(searched2 != NULL_VALUE){
        label += xyz_second_str + SE_COLON_STRING;
        label += (searched2 != 0) ? AdjustHexString(QString::number(searched2,16), 8, "0") : SE_ZERO_POS_STRING;
    }

    // resulted position
    label += "\n";
    label += SE_RESULT_POS_STRING;
    if(result1 != NULL_VALUE){
        label += xyz_first_str + SE_COLON_STRING;
        label += (result1 != 0) ? QString::number(result1,16) : SE_ZERO_POS_STRING;
        if(result2 != NULL_VALUE)
            label += SE_SPACE_STRING;
    }
    if(result2 != NULL_VALUE){
        label += xyz_second_str + SE_COLON_STRING;
        label += (result2 != 0) ? QString::number(result2,16) : SE_ZERO_POS_STRING;
    }

    label += "\n\n";

// -------- actions -------------

    if(this->full_sol_copy != nullptr){ // if full solution
        // full solutions always start with the actions from the full solution copy
        sc = this->full_sol_copy;
        goto add_action_text;
    }

    partial_solution_actions:
    // if partial solution or full solution from a sub search
    // partial solutions only have the actions from the partial solution copy
    // for full solutions from a sub search, the partial solution is practically the second half of the solution

    if(this->full_sol_copy != nullptr){ // if full solution resulted from a sub search
        // description of resetting the position that was found as partial solution
        label += SE_RESET_POS_STRING_1;
        label += (this->part_sol_copy->found_first) ?
                    this->part_sol_copy->xyz_first_string : this->part_sol_copy->xyz_second_string;
        label += SE_RESET_POS_STRING_2;
        label += "\n";
    }

    goto_partial_solution_actions = false;
    sc = this->part_sol_copy;
    goto add_action_text;

    // -----------------------------------------------

    done:

    if(this->solution_description_label.text() != label)
    {
        this->solution_description_label.setText(label);
        this->solution_description_label.adjustSize();
    }
    bottom_frame_height = SE_BOTTOM_ADDITIONAL_HEIGHT;
    bottom_frame_height += this->solution_description_label.height();
    this->bottom_frame.setFixedHeight(bottom_frame_height);
    return SUCCESS;

    // ----------------------------------------------

    add_action_text:

    uint action_count = sc->action_amounts.size();
    if(sc->action_angles.size() != action_count || sc->action_names.size() != action_count){
        qInfo() << "error in SolutionElement::SetText():";
        qInfo() << "solution copy has different sizes of action_count, action_names and action_angles";
        if(action_count > sc->action_angles.size())
            action_count = sc->action_angles.size();
        if(action_count > sc->action_names.size())
            action_count = sc->action_names.size();
    }

    int availableActionNameWidth = bottom_frame.width() - SE_ACTION_WITHOUT_NAME_LENGTH;
    QString longestActionName;
    for(uint i = 0; i < action_count; i++)
    {
        if(sc->action_names[i].size() > longestActionName.size())
            longestActionName = sc->action_names[i];
    }
    longestActionName = solution_description_label.fontMetrics().elidedText(longestActionName, Qt::TextElideMode::ElideRight, availableActionNameWidth);
    int actionNameCharLimit = longestActionName.length();// TODO this is probably not nexessary with the new format

    HWORD currentAngle = 0;
    QString temp_str;
    for(uint i = 0; i < action_count; i++)
    {
        if (i == 0 || currentAngle != sc->action_angles[i])
        {
            if(i > 0)
                label += "\n\n";

            currentAngle = sc->action_angles[i];
            temp_str = QString::number(currentAngle,16);
            if(temp_str.size() != SE_ACTION_ANGLE_LENGTH)
                temp_str = AdjustHexString(temp_str,SE_ACTION_ANGLE_LENGTH);
            label += "angle " + temp_str + ":   ";
        }
        else
        {
            label += "\n              ";
        }

        if (sc->action_amounts[i] < 10)
            label += " ";
        label += QString::number(sc->action_amounts[i],10) + " ";

        label += SE_ACTION_PREFIX_STRING;
        QString temp_str = sc->action_names[i];
        if(temp_str.size() > actionNameCharLimit){
            temp_str = temp_str.left(actionNameCharLimit-3);
            temp_str += "...";
        }
        else if(temp_str.size() < actionNameCharLimit){
            QString temp_str2;
            for(uint j = temp_str.size(); j < actionNameCharLimit; j++){
                temp_str2 += " ";
            }
#ifdef SE_ACTION_NAME_RIGHT_ALIGNED    // action name right aligned
            temp_str2 += temp_str;
            temp_str = temp_str2;
#else                                   // action name left aligned
            temp_str += temp_str2;
#endif
        }
        label += temp_str;
    }

    // -------------------------------
    if(goto_partial_solution_actions)
        goto partial_solution_actions;
    else
        goto done;
}

void SolutionElement::SetEnabled(bool enable)
{
    top_frame.setEnabled(enable);
    bottom_frame.setEnabled(enable);
    header_label.setEnabled(enable);
    fold_button.setEnabled(enable);
    delete_button.setEnabled(enable);
}
void PartialSolutionElement::SetEnabled(bool enable)
{
    top_frame.setEnabled(enable);
    bottom_frame.setEnabled(enable);
    header_label.setEnabled(enable);
    fold_button.setEnabled(enable);
    delete_button.setEnabled(enable);
    sub_search_button.setEnabled(enable);
}

// Calculates the y-coordinate for an element based on the previous element's geometry
int Posefi::CalculatePartialSolutionElement_YPos(uint order_index){
    if(order_index >= this->PSE_Order.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::CalculatePartialSolutionElementHeight";
        return 0;
    }
    int y = SE_TOP_FRAME_GAP;
    if(order_index == 0)
        return y;
    int element_index_of_prev = this->PSE_Order[order_index - 1];
    if(element_index_of_prev >= (int)this->PartialSolutionElements.size() || element_index_of_prev < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::CalculatePartialSolutionElementHeight";
        return ARRAY_OUT_OF_RANGE;
    }
    // Add the height and y-pos of the previous element. However, if the previous element is currently
    // in an animation, take its end value. If there is not an animation, don't take the animation-end-value,
    // because the geometry is not exclusively set through animations (there is at least one case on page change).
    PartialSolutionElement* pse_prev = this->PartialSolutionElements[element_index_of_prev];
    if(pse_prev->geometry_animation.currentTime() < pse_prev->geometry_animation.totalDuration()){
        QRect r = pse_prev->geometry_animation.endValue().toRect();
        y += r.height();
        y += r.y();
    }
    else{
        y += pse_prev->top_frame.height();
        y += pse_prev->top_frame.y();
    }
    return y;
}

// Calculates the y-coordinate for an element based on the previous element's geometry
int Posefi::CalculateFullSolutionElement_YPos(uint order_index){
    if(order_index >= this->FSE_Order.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::CalculateFullSolutionElementHeight";
        return 0;
    }
    int y = SE_TOP_FRAME_GAP;
    if(order_index == 0)
        return y;
    int element_index_of_prev = this->FSE_Order[order_index - 1];
    if(element_index_of_prev >= (int)this->FullSolutionElements.size() || element_index_of_prev < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::CalculateFullSolutionElementHeight";
        return ARRAY_OUT_OF_RANGE;
    }
    // Add the height and y-pos of the previous element. However, if the previous element is currently
    // in an animation, take its end value. If there is not an animation, don't take the animation-end-value,
    // because the geometry is not exclusively set through animations (there is at least one case on page change).
    SolutionElement* fse_prev = this->FullSolutionElements[element_index_of_prev];
    if(fse_prev->geometry_animation.currentTime() < fse_prev->geometry_animation.totalDuration()){
        QRect r = fse_prev->geometry_animation.endValue().toRect();
        y += r.height();
        y += r.y();
    }
    else{
        y += fse_prev->top_frame.height();
        y += fse_prev->top_frame.y();
    }
    return y;
}

int Posefi::PartialSolutionFoldButton_press(int element_index){
    if(element_index < 0 || element_index >= (int)this->PartialSolutionElements.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PartialSolutionFoldButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    int h_end;
    PartialSolutionElement* pse = this->PartialSolutionElements[element_index];
    if(pse->folded){
        h_end = SE_HEADER_HEIGHT + pse->bottom_frame.height() + PSE_BOTTOM_FRAME_GAP;
        pse->folded = false;
    }
    else{
        h_end = SE_HEADER_HEIGHT;
        pse->folded = true;
    }

    // fold / unfold animation
    int w_start,h_start,x_start,y_start;
    w_start = pse->top_frame.width();
    h_start = pse->top_frame.height();
    x_start = pse->top_frame.x();
    y_start = pse->top_frame.y();
    pse->geometry_animation.setStartValue(QRect(x_start,y_start,w_start,h_start));
    QRect r_end = pse->geometry_animation.endValue().toRect();
    r_end.setHeight(h_end);
    pse->geometry_animation.setEndValue(r_end);
    pse->geometry_animation.setDuration(PSE_UP_DOWN_ANIMATION_DURATION);
    pse->geometry_animation.start();

    // animations of the following elements moving up or down
    int order_index = pse->order_index;
    if(order_index < 0 || order_index >= (int)this->PSE_Order.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PartialSolutionFoldButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    for(uint i = order_index + 1; i < this->PSE_Order.size(); i++){
        int ei = this->PSE_Order[i];
        int prev_ei =this->PSE_Order[i-1];
        if(ei < 0)
            break;
        if(ei >= (int)this->PartialSolutionElements.size() || prev_ei >= (int)this->PartialSolutionElements.size()){
            qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PartialSolutionFoldButton_press";
            return ARRAY_OUT_OF_RANGE;
        }
        PartialSolutionElement* pse_i = this->PartialSolutionElements[ei];
        QRect r_prev = this->PartialSolutionElements[prev_ei]->geometry_animation.endValue().toRect();
        QRect r_start = pse_i->top_frame.rect();
        r_start.moveTopLeft(QPoint(pse_i->top_frame.x(),pse_i->top_frame.y()));
        QRect r_end;
        if(pse_i->geometry_animation.currentTime() < pse_i->geometry_animation.totalDuration())
            r_end = pse_i->geometry_animation.endValue().toRect();
        else
            r_end = r_start;
        r_end.moveTop(r_prev.height() + r_prev.y() + SE_TOP_FRAME_GAP);
        pse_i->geometry_animation.stop();
        pse_i->geometry_animation.setStartValue(r_start);
        pse_i->geometry_animation.setEndValue(r_end);
        pse_i->geometry_animation.setDuration(PSE_UP_DOWN_ANIMATION_DURATION);
        pse_i->geometry_animation.start();
    }

    this->AdjustPartialSolutionScrollAreaHeight();

    return SUCCESS;
}

int Posefi::FullSolutionFoldButton_press(int element_index){
    if(element_index < 0 || element_index >= (int)this->FullSolutionElements.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FullSolutionFoldButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    int h_end;
    SolutionElement* fse = this->FullSolutionElements[element_index];
    if(fse->folded){
        h_end = SE_HEADER_HEIGHT + fse->bottom_frame.height() + FSE_BOTTOM_FRAME_GAP;
        fse->folded = false;
    }
    else{
        h_end = SE_HEADER_HEIGHT;
        fse->folded = true;
    }

    // fold / unfold animation
    int w_start,h_start,x_start,y_start;
    w_start = fse->top_frame.width();
    h_start = fse->top_frame.height();
    x_start = fse->top_frame.x();
    y_start = fse->top_frame.y();
    fse->geometry_animation.setStartValue(QRect(x_start,y_start,w_start,h_start));
    QRect r_end = fse->geometry_animation.endValue().toRect();
    r_end.setHeight(h_end);
    fse->geometry_animation.setEndValue(r_end);
    fse->geometry_animation.setDuration(FSE_UP_DOWN_ANIMATION_DURATION);
    fse->geometry_animation.start();

    // animations of the following elements moving up or down
    int order_index = fse->order_index;
    if(order_index < 0 || order_index >= (int)this->FSE_Order.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FullSolutionFoldButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    for(uint i = order_index + 1; i < this->FSE_Order.size(); i++){
        int ei = this->FSE_Order[i];
        int prev_ei =this->FSE_Order[i-1];
        if(ei < 0)
            break;
        if(ei >= (int)this->FullSolutionElements.size() || prev_ei >= (int)this->FullSolutionElements.size()){
            qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FullSolutionFoldButton_press";
            return ARRAY_OUT_OF_RANGE;
        }
        SolutionElement* fse_i = this->FullSolutionElements[ei];
        QRect r_prev = this->FullSolutionElements[prev_ei]->geometry_animation.endValue().toRect();
        QRect r_start = fse_i->top_frame.rect();
        r_start.moveTopLeft(QPoint(fse_i->top_frame.x(),fse_i->top_frame.y()));
        QRect r_end;
        if(fse_i->geometry_animation.currentTime() < fse_i->geometry_animation.totalDuration())
            r_end = fse_i->geometry_animation.endValue().toRect();
        else
            r_end = r_start;
        r_end.moveTop(r_prev.height() + r_prev.y() + SE_TOP_FRAME_GAP);
        fse_i->geometry_animation.stop();
        fse_i->geometry_animation.setStartValue(r_start);
        fse_i->geometry_animation.setEndValue(r_end);
        fse_i->geometry_animation.setDuration(FSE_UP_DOWN_ANIMATION_DURATION);
        fse_i->geometry_animation.start();
    }

    this->AdjustFullSolutionScrollAreaHeight();

    return SUCCESS;
}

int Posefi::PartialSolutionDeleteButton_press(int element_index){
    if((int)PSE_Order.size() <= element_index){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PartialSolutionDeleteButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    PartialSolutionElement* pse = this->PartialSolutionElements[element_index];

    // TODO fix bug
    //pse->header_label.setText("error: displaying old solution. Delete all and restart search to fix the problem.");

    int order_index = pse->order_index;
    if(order_index < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PartialSolutionDeleteButton_press; order_index is negative";
        return ARRAY_OUT_OF_RANGE;
    }
    // shift all entries after [order_index] in the order one position back
    uint order_end_index = this->PSE_Order.size() - 1;
    for(uint i = order_index; i < order_end_index; i++){

        int new_element_index = this->PSE_Order[i+1];
        int new_order_index = i;
        this->PSE_Order[i] = new_element_index;
        if(new_element_index >= 0)
            this->PartialSolutionElements[new_element_index]->order_index = new_order_index;
    }

    // add the negative equivalent of the index at the back
    this->PSE_Order[order_end_index] = -(element_index +1);
    int oi = -(pse->order_index + 1); // negative equivalent of the order_index
    pse->order_index = oi; // negative is interpreted as disabled

    // animation to make the element disappear
    QRect r_start = pse->top_frame.rect();
    r_start.moveTopLeft(QPoint(pse->top_frame.x(),pse->top_frame.y()));
    QRect r_end = pse->geometry_animation.endValue().toRect();
    r_end.moveLeft(PSE_FROM_RIGHT_ANIMATION_X1);
    pse->geometry_animation.stop();
    pse->geometry_animation.setStartValue(r_start);
    pse->geometry_animation.setEndValue(r_end);
    pse->opacity_animation.setStartValue(1);
    pse->opacity_animation.setEndValue(0);
    pse->geometry_animation.setDuration(PSE_REMOVE_ANIMATION_DURATION);
    pse->opacity_animation.setDuration(PSE_REMOVE_ANIMATION_DURATION);
    pse->geometry_animation.start();
    pse->opacity_animation.start();
    this->current_ps_disable_animations++;

    if(pse->part_sol_copy != nullptr){
        uint ps_indices_index = this->current_ps_page * this->solutions_per_page + order_index;

        //ps_indices.removeAt(ps_indices_index);// remove this once cleanSolutionMess is done
        // Bug workaround
        ps_indices[ps_indices_index] = (ps_indices[ps_indices_index] * -1) -1;
        cleanPartialSolutionMess();
    }

    pse->delete_button.setEnabled(false);
    pse->fold_button.setEnabled(false);
    pse->sub_search_button.setEnabled(false);

    ManagePartialSolutionPages();

    return SUCCESS;
}

int Posefi::FullSolutionDeleteButton_press(int element_index){
    if((int)FSE_Order.size() <= element_index){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FullSolutionDeleteButton_press";
        return ARRAY_OUT_OF_RANGE;
    }
    SolutionElement* fse = this->FullSolutionElements[element_index];

    // TODO fix bug
    //fse->header_label.setText("error: displaying old solution. Delete all and restart search to fix the problem.");

    int order_index = fse->order_index;
    if(order_index < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FullSolutionDeleteButton_press; order_index is negative";
        return ARRAY_OUT_OF_RANGE;
    }
    // shift all entries after [order_index] in the order one position back
    uint order_end_index = this->FSE_Order.size() - 1;
    for(uint i = order_index; i < order_end_index; i++){

        int new_element_index = this->FSE_Order[i+1];
        int new_order_index = i;
        this->FSE_Order[i] = new_element_index;
        if(new_element_index >= 0)
            this->FullSolutionElements[new_element_index]->order_index = new_order_index;
    }

    // add the negative equivalent of the index at the back
    this->FSE_Order[order_end_index] = -(element_index +1);
    int oi = -(fse->order_index + 1); // negative equivalent of the order_index
    fse->order_index = oi; // negative is interpreted as disabled

    // animation to make the element disappear
    QRect r_start = fse->top_frame.rect();
    r_start.moveTopLeft(QPoint(fse->top_frame.x(),fse->top_frame.y()));
    QRect r_end = fse->geometry_animation.endValue().toRect();
    r_end.moveLeft(FSE_FROM_RIGHT_ANIMATION_X1);
    fse->geometry_animation.stop();
    fse->geometry_animation.setStartValue(r_start);
    fse->geometry_animation.setEndValue(r_end);
    fse->opacity_animation.setStartValue(1);
    fse->opacity_animation.setEndValue(0);
    fse->geometry_animation.setDuration(FSE_REMOVE_ANIMATION_DURATION);
    fse->opacity_animation.setDuration(FSE_REMOVE_ANIMATION_DURATION);
    fse->geometry_animation.start();
    fse->opacity_animation.start();
    this->current_fs_disable_animations++;




    // remove element from the fs_indices array
    if(fse->full_sol_copy != nullptr){
        uint fs_indices_index = this->current_fs_page * this->solutions_per_page + order_index;

        //fs_indices.removeAt(fs_indices_index);// remove this once cleanFullSolutionMess is done
        // Bug workaround
        fs_indices[fs_indices_index] = (fs_indices[fs_indices_index] * -1) -1;
        cleanFullSolutionMess();
    }

    fse->delete_button.setEnabled(false);
    fse->fold_button.setEnabled(false);

    ManageFullSolutionPages();

    return SUCCESS;
}

void Posefi::cleanSolutionMess(QList<SolutionCopy>& solCopies, QList<int>& solIndices)
{
    int deletedIdx = -1;

    for(int i = 0; i < solIndices.size(); i++)
    {
        if(solIndices[i] < 0)
        {
            deletedIdx = i;
            break;
        }
    }
    if(deletedIdx == -1)
    {
        qInfo() << "error in Posefi::cleanSolutionMess";
        return;
    }

    int deletedSolCopyIdx = solIndices[deletedIdx] * -1 - 1;
    for(int i = 0; i < solIndices.size(); i++)
    {
        if(solIndices[i] > deletedSolCopyIdx)
        {
            solIndices[i]--;
        }
    }
    solIndices.removeAt(deletedIdx);
    solCopies.removeAt(deletedSolCopyIdx);
}

int Posefi::ResizePartialSolutionsWidth(){
    int top_frame_width = this->ui->scrollAreaPartialSolutionsContents->width() - 2*SE_TOP_FRAME_GAP;
    int bottom_frame_width = top_frame_width - 2*PSE_BOTTOM_FRAME_GAP;
    for(uint i = 0; i < this->PartialSolutionElements.size(); i++){
        PartialSolutionElement* pse = this->PartialSolutionElements[i];
        pse->top_frame.setFixedWidth(top_frame_width);
        pse->header_label.setFixedWidth(top_frame_width - 2 * pse->header_label.x());
        pse->fold_button.setFixedWidth(top_frame_width - SE_FOLD_BUTTON_GAP_RIGHT);
        pse->bottom_frame.setFixedWidth(bottom_frame_width);
        int del_x = pse->top_frame.width() - pse->delete_button.width() - PSE_DELETE_BUTTON_GAP_RIGHT;
        pse->delete_button.move(del_x,PSE_DELETE_BUTTON_Y);
        //pse->SetText();
    }
    return SUCCESS;
}

int Posefi::ResizeFullSolutionsWidth(){
    int top_frame_width = this->ui->scrollAreaFullSolutionsContents->width() - 2*SE_TOP_FRAME_GAP;
    int bottom_frame_width = top_frame_width - 2*FSE_BOTTOM_FRAME_GAP;
    for(uint i = 0; i < this->FullSolutionElements.size(); i++){
        SolutionElement* fse = this->FullSolutionElements[i];
        fse->top_frame.setFixedWidth(top_frame_width);
        fse->header_label.setFixedWidth(top_frame_width - 2 * fse->header_label.x());
        fse->fold_button.setFixedWidth(top_frame_width - SE_FOLD_BUTTON_GAP_RIGHT);
        fse->bottom_frame.setFixedWidth(bottom_frame_width);
        int del_x = fse->top_frame.width() - fse->delete_button.width() - PSE_DELETE_BUTTON_GAP_RIGHT;
        fse->delete_button.move(del_x,FSE_DELETE_BUTTON_Y);

        //int nameCount = fse->full_sol_copy->action_names.size();
        //int amountCount = fse->full_sol_copy->action_amounts.size();
        //int angleCount = fse->full_sol_copy->action_angles.size();
        //if(nameCount != amountCount || nameCount != angleCount)
        //    continue;
        //fse->SetText();
        // TODO bugged
    }
    return SUCCESS;
}

int Posefi::PartialSolutionSubSearchButton_press(int element_index){
    if(element_index < 0 || element_index >= (int)this->PartialSolutionElements.size()){
        qInfo() << "error in Posefi::PartialSolutionSubSearchButton_press: (element_index < 0 || element_index >= this->PartialSolutionElements.size())";
        return ARRAY_OUT_OF_RANGE;
    }
    return this->InitiateSubSearch(this->PartialSolutionElements[element_index]->part_sol_copy);
}

// Sets the height to fit in the lowest element that either is active or in an animation
int Posefi::AdjustPartialSolutionScrollAreaHeight(){
    int visible_height = this->ui->scrollAreaPartialSolutions->height() - 2;
    if (this->ui->scrollAreaPartialSolutions->horizontalScrollBar()->maximum())
        visible_height -= SCROLLBAR_SIZE;
    int used_height = SE_TOP_FRAME_GAP;
    for(int i = (int)this->PSE_Order.size()-1; i >= 0; i--){
        int element_index = this->PSE_Order[i];
        if(element_index >= 0){
            if(element_index >= (int)this->PartialSolutionElements.size()){
                qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::AdjustPartialSolutionScrollAreaHeight()";
                return ARRAY_OUT_OF_RANGE;
            }
            used_height += this->PartialSolutionElements[element_index]->top_frame.height();
            used_height += this->PartialSolutionElements[element_index]->top_frame.y();
            break;
        }
        else{
            element_index = -element_index - 1;
            if(element_index >= (int)this->PartialSolutionElements.size()){
                qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::AdjustPartialSolutionScrollAreaHeight()";
                return ARRAY_OUT_OF_RANGE;
            }
            if(this->PartialSolutionElements[element_index]->opacity_animation.currentTime()
                    < this->PartialSolutionElements[element_index]->opacity_animation.totalDuration()){
                used_height += this->PartialSolutionElements[element_index]->top_frame.height();
                used_height += this->PartialSolutionElements[element_index]->top_frame.y();
                break;
            }
        }
    }

    int h_old = this->ui->scrollAreaPartialSolutionsContents->height();
    int h_new = (visible_height >= used_height) ? visible_height : used_height;
    if(h_old != h_new){
        this->ui->scrollAreaPartialSolutionsContents->setFixedHeight(h_new);
        this->pse_scroll_resize_timer.start(PSE_SCROLL_AREA_RESIZE_TIMER);
    }

    return SUCCESS;
}

// Sets the height to fit in the lowest element that either is active or in an animation
int Posefi::AdjustFullSolutionScrollAreaHeight(){
    int visible_height = this->ui->scrollAreaFullSolutions->height() - 2;
    if (this->ui->scrollAreaFullSolutions->horizontalScrollBar()->maximum())
        visible_height -= SCROLLBAR_SIZE;
    int used_height = SE_TOP_FRAME_GAP;
    for(int i = (int)this->FSE_Order.size()-1; i >= 0; i--){
        int element_index = this->FSE_Order[i];
        if(element_index >= 0){
            if(element_index >= (int)this->FullSolutionElements.size()){
                qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::AdjustFullSolutionScrollAreaHeight()";
                return ARRAY_OUT_OF_RANGE;
            }
            used_height += this->FullSolutionElements[element_index]->top_frame.height();
            used_height += this->FullSolutionElements[element_index]->top_frame.y();
            break;
        }
        else{
            element_index = -element_index - 1;
            if(element_index >= (int)this->FullSolutionElements.size()){
                qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::AdjustFullSolutionScrollAreaHeight()";
                return ARRAY_OUT_OF_RANGE;
            }
            if(this->FullSolutionElements[element_index]->opacity_animation.currentTime()
                    < this->FullSolutionElements[element_index]->opacity_animation.totalDuration()){
                used_height += this->FullSolutionElements[element_index]->top_frame.height();
                used_height += this->FullSolutionElements[element_index]->top_frame.y();
                break;
            }
        }
    }

    int h_old = this->ui->scrollAreaFullSolutionsContents->height();
    int h_new = (visible_height >= used_height) ? visible_height : used_height;
    if(h_old != h_new){
        this->ui->scrollAreaFullSolutionsContents->setFixedHeight(h_new);
        this->fse_scroll_resize_timer.start(FSE_SCROLL_AREA_RESIZE_TIMER);
    }

    return SUCCESS;
}

int Posefi::PSE_OpacityAnimationEnd(int element_index){

    if(element_index >= (int)this->PartialSolutionElements.size() || element_index < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PSE_OpacityAnimationEnd";
        qInfo() << "(element_index >= (int)this->PartialSolutionElements.size() || element_index < 0)";
        return ARRAY_OUT_OF_RANGE;
    }
    PartialSolutionElement* pse = this->PartialSolutionElements[element_index];

    // If the finished animation was an element disappearing:
    if(pse->order_index < 0){   // negative order_index means the element got deactivated
        pse->top_frame.setEnabled(false);
        pse->top_frame.hide();
        pse->part_sol_copy = nullptr;
        pse->full_sol_copy = nullptr;

        this->current_ps_disable_animations--;
        if(this->current_ps_disable_animations < 0){
            qInfo() << "error in Posefi::PSE_OpacityAnimationEnd: (this->current_disable_animations < 0)";
            this->current_ps_disable_animations = 0;
        }

        // if a page change is comming up don't bother moving the following elements
        if(this->upcoming_ps_page_change){
            // if all pse are done with the page change animation
            if(this->current_ps_disable_animations == 0){
                this->ShowNewPartialSolutions();
            }
        }
        else{
            // animation of to adjust the y-pos of following elements:
            int order_index = -(pse->order_index + 1); // revert order_index again to positive
            if(order_index > 0){
                int ei_prev = this->PSE_Order[order_index-1];
                if(ei_prev >= (int)this->PartialSolutionElements.size() || ei_prev < 0){
                    qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PSE_OpacityAnimationEnd";
                    qInfo() << "(ei_prev >= (int)this->PartialSolutionElements.size() || ei_prev < 0)";
                    return ARRAY_OUT_OF_RANGE;
                }
            }
            for(uint i = order_index; i < this->PSE_Order.size(); i++){
                int ei = this->PSE_Order[i];
                if(ei < 0)
                    break;
                if(ei >= (int)this->PartialSolutionElements.size()){
                    qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PSE_OpacityAnimationEnd";
                    qInfo() << "(ei >= (int)this->PartialSolutionElements.size())";
                    return ARRAY_OUT_OF_RANGE;
                }

                PartialSolutionElement* pse_ = this->PartialSolutionElements[ei];

                QRect r_start = pse_->top_frame.rect();
                r_start.moveTopLeft(QPoint(pse_->top_frame.x(),pse_->top_frame.y()));
                QRect r_end = pse_->geometry_animation.endValue().toRect();
                int y_end = SE_TOP_FRAME_GAP;
                if (i > 0){
                    int prev_ei = this->PSE_Order[i-1];
                    if(prev_ei < 0 || prev_ei >= (int)this->PartialSolutionElements.size()){
                        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PSE_OpacityAnimationEnd";
                        qInfo() << "(prev_ei < 0 || prev_ei >= (int)this->PartialSolutionElements.size())";
                        return ARRAY_OUT_OF_RANGE;
                    }
                    PartialSolutionElement* prev_pse = this->PartialSolutionElements[prev_ei];
                    y_end += prev_pse->geometry_animation.endValue().toRect().y();
                    y_end += prev_pse->geometry_animation.endValue().toRect().height();
                }
                r_end.moveTop(y_end);
                pse_->geometry_animation.stop();
                pse_->geometry_animation.setStartValue(r_start);
                pse_->geometry_animation.setEndValue(r_end);
                pse_->geometry_animation.setDuration(PSE_UP_DOWN_ANIMATION_DURATION);
                pse_->geometry_animation.start();
            }

            // If there are more elements to come don't adjust the scroll area size yet.
            // The scroll area would shrink and then instantly expand again.
            // And if there are no further solutions there is no need to call ShowNewPartialSolutions().
            bool next_page = ((this->current_ps_page + 1) * this->solutions_per_page < this->ps_indices.size() + 1);
            //(add the 1 to ps_indices.size() so reflect the solution count before we removed the solution which triggered this function)
            if(next_page)
                this->ShowNewPartialSolutions();
            else
                this->AdjustPartialSolutionScrollAreaHeight();

        }
    }

    return SUCCESS;
}

int Posefi::FSE_OpacityAnimationEnd(int element_index){
    if(element_index >= (int)this->FullSolutionElements.size() || element_index < 0){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FSE_OpacityAnimationEnd";
        return ARRAY_OUT_OF_RANGE;
    }
    SolutionElement* fse = this->FullSolutionElements[element_index];

    // If the finished animation was an element disappearing:
    if(fse->order_index < 0){   // negative order_index means the element got deactivated
        fse->top_frame.setEnabled(false);
        fse->top_frame.hide();
        fse->full_sol_copy = nullptr;
        fse->part_sol_copy = nullptr;

        this->current_fs_disable_animations--;
        if(this->current_fs_disable_animations < 0){
            qInfo() << "error in Posefi::FSE_OpacityAnimationEnd: (this->current_disable_animations < 0)";
            this->current_fs_disable_animations = 0;
        }

        // if a page change is comming up don't bother moving the following elements
        if(this->upcoming_fs_page_change){
            // if all fse are done with the page change animation
            if(this->current_fs_disable_animations == 0){
                this->ShowNewFullSolutions();
            }
        }
        else{
            // animation of to adjust the y-pos of following elements:
            int order_index = -(fse->order_index + 1); // revert order_index again to positive
            if(order_index > 0){
                int ei_prev = this->FSE_Order[order_index-1];
                if(ei_prev >= (int)this->FullSolutionElements.size() || ei_prev < 0){
                    qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FSE_OpacityAnimationEnd";
                    return ARRAY_OUT_OF_RANGE;
                }
            }
            for(uint i = order_index; i < this->FSE_Order.size(); i++){
                int ei = this->FSE_Order[i];
                if(ei < 0)
                    break;
                if(ei >= (int)this->FullSolutionElements.size()){
                    qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FSE_OpacityAnimationEnd";
                    return ARRAY_OUT_OF_RANGE;
                }

                SolutionElement* fse_ = this->FullSolutionElements[ei];

                QRect r_start = fse_->top_frame.rect();
                r_start.moveTopLeft(QPoint(fse_->top_frame.x(),fse_->top_frame.y()));
                QRect r_end = fse_->geometry_animation.endValue().toRect();
                int y_end = SE_TOP_FRAME_GAP;
                if (i > 0){
                    int prev_ei = this->FSE_Order[i-1];
                    if(prev_ei < 0 || prev_ei >= (int)this->FullSolutionElements.size()){
                        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FSE_OpacityAnimationEnd";
                        return ARRAY_OUT_OF_RANGE;
                    }
                    SolutionElement* prev_fse = this->FullSolutionElements[prev_ei];
                    y_end += prev_fse->geometry_animation.endValue().toRect().y();
                    y_end += prev_fse->geometry_animation.endValue().toRect().height();
                }
                r_end.moveTop(y_end);
                fse_->geometry_animation.stop();
                fse_->geometry_animation.setStartValue(r_start);
                fse_->geometry_animation.setEndValue(r_end);
                fse_->geometry_animation.setDuration(FSE_UP_DOWN_ANIMATION_DURATION);
                fse_->geometry_animation.start();
            }

            // If there are more elements to come don't adjust the scroll area size yet.
            // The scroll area would shrink and then instantly expand again.
            // And if there are no further solutions there is no need to call ShowNewFullSolutions().
            bool next_page = ((this->current_fs_page + 1) * this->solutions_per_page < this->fs_indices.size() + 1);
            //(add the 1 to fs_indices.size() so reflect the solution count before we removed the solution which triggered this function)
            if(next_page)
                this->ShowNewFullSolutions();
            else
                this->AdjustFullSolutionScrollAreaHeight();

        }
    }

    return SUCCESS;
}

// Buttons don't do a check for the cursor position after they moved from an animation.
// Therefore this function is called at the end of geometry animations, to accurately display the hovering.
int Posefi::PSE_GeometryAnimationEnd(int element_index){
    if(element_index >= (int)this->PartialSolutionElements.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::PSE_GeometryAnimationEnd";
        return ARRAY_OUT_OF_RANGE;
    }
    //QMouseEvent eve( (QEvent::MouseMove), QPoint(100,100));

    //qApp->sendEvent(this,&eve);
    QPoint p = QCursor::pos();
    PartialSolutionElement* se = this->PartialSolutionElements[element_index];
    //if(se->fold_button.underMouse()){
    if(se->fold_button.geometry().contains(QCursor::pos())){//local poss i guess
        //qInfo() << "under mouse: " << element_index;
        //se->fold_button.setStyleSheet("QPushButton { background-color: #40E0E0FF;}");
    }
    QCursor::setPos(0,0);
    se->fold_button.repaint();
    QCursor::setPos(p);
    //se->delete_button.update();
    //se->sub_search_button.update();
    //se->fold_button.setStyle(&QStyle(QStyle::State_MouseOver));
    //se->fold_button.setst
    return SUCCESS;
}

int Posefi::FSE_GeometryAnimationEnd(int element_index){
    if(element_index >= (int)this->FullSolutionElements.size()){
        qInfo() << "ARRAY_OUT_OF_RANGE error in Posefi::FSE_GeometryAnimationEnd";
        return ARRAY_OUT_OF_RANGE;
    }
    SolutionElement* se = this->FullSolutionElements[element_index];
    se->fold_button.update();
    se->delete_button.update();
    return SUCCESS;
}

int Posefi::ManagePartialSolutionPages(){

    // case: no solutions
    if(this->ps_indices.empty()){
        this->current_ps_page = -1;
        this->ui->partialSolutionsFirstButton->setEnabled(false);
        this->ui->partialSolutionsPrevButton->setEnabled(false);
        this->ui->partialSolutionsNextButton->setEnabled(false);
        this->ui->partialSolutionsLastButton->setEnabled(false);
        this->ui->partialSolutionsShowLabel->setText("");
        this->partialSolutionPageEdit->setText("");
        this->partialSolutionPageEdit->setEnabled(false);
        this->ui->partialSolutionsDeleteButton->setEnabled(false);
        this->ui->partialSolutionsSortButton->setEnabled(false);
        return SUCCESS;
    }
    else
    {
        this->ui->partialSolutionsDeleteButton->setEnabled(true);
        this->ui->partialSolutionsSortButton->setEnabled(true);
    }

    if(this->current_ps_page < 0)
        this->current_ps_page = 0;

    int page_count = this->ps_indices.size() / this->solutions_per_page;
    if(page_count * this->solutions_per_page < this->ps_indices.size())
        page_count++;

    if(page_count > 1)
        this->partialSolutionPageEdit->setEnabled(true);

    bool not_last_page = ((page_count-1) > this->current_ps_page);

    // case: current page is empty (happens when you delete all elements from the last page)
    if(!not_last_page && this->current_ps_page > 0){
        if(this->ps_indices.size() <= this->current_ps_page * this->solutions_per_page){
            this->PartialSolutionChangePage(this->current_ps_page - 1);
            return SUCCESS;
        }
    }

    this->ui->partialSolutionsNextButton->setEnabled(not_last_page);
    this->ui->partialSolutionsLastButton->setEnabled(not_last_page);

    bool not_first_page = (this->current_ps_page > 0);
    this->ui->partialSolutionsFirstButton->setEnabled(not_first_page);
    this->ui->partialSolutionsPrevButton->setEnabled(not_first_page);

    uint sol_total = this->ps_indices.size();
    uint from_sol = (this->current_ps_page * solutions_per_page) + 1;
    uint to_sol = ((this->current_ps_page + 1) * solutions_per_page);
    if(to_sol > sol_total)
        to_sol = sol_total;
    QString s = QString::number(from_sol,10);
    s += PSE_FROM_TO_STRING;
    s += QString::number(to_sol,10);
    s += PSE_OF_STRING;
    s += QString::number(sol_total,10);
    this->ui->partialSolutionsShowLabel->setText(s);

    this->partialSolutionPageEdit->setText(QString::number(this->current_ps_page + 1,10));

    return SUCCESS;
}

int Posefi::ManageFullSolutionPages(){

    // case: no solutions
    if(this->fs_indices.empty()){
        this->current_fs_page = -1;
        this->ui->fullSolutionsFirstButton->setEnabled(false);
        this->ui->fullSolutionsPrevButton->setEnabled(false);
        this->ui->fullSolutionsNextButton->setEnabled(false);
        this->ui->fullSolutionsLastButton->setEnabled(false);
        this->ui->fullSolutionsShowLabel->setText("");
        this->fullSolutionPageEdit->setText("");
        this->fullSolutionPageEdit->setEnabled(false);
        this->ui->fullSolutionsDeleteButton->setEnabled(false);
        this->ui->fullSolutionsSortButton->setEnabled(false);
        return SUCCESS;
    }
    else
    {
        this->ui->fullSolutionsDeleteButton->setEnabled(true);
        this->ui->fullSolutionsSortButton->setEnabled(true);
    }

    if(this->current_fs_page < 0)
        this->current_fs_page = 0;

    int page_count = this->fs_indices.size() / this->solutions_per_page;
    if(page_count * this->solutions_per_page < this->fs_indices.size())
        page_count++;

    if(page_count > 1)
        this->fullSolutionPageEdit->setEnabled(true);

    bool not_last_page = ((page_count-1) > this->current_fs_page);

    // case: current page is empty (happens when you delete all elements from the last page)
    if(!not_last_page && this->current_fs_page > 0){
        if(this->fs_indices.size() <= this->current_fs_page * this->solutions_per_page){
            this->FullSolutionChangePage(this->current_fs_page - 1);
            return SUCCESS;
        }
    }

    this->ui->fullSolutionsNextButton->setEnabled(not_last_page);
    this->ui->fullSolutionsLastButton->setEnabled(not_last_page);

    bool not_first_page = (this->current_fs_page > 0);
    this->ui->fullSolutionsFirstButton->setEnabled(not_first_page);
    this->ui->fullSolutionsPrevButton->setEnabled(not_first_page);

    uint sol_total = this->fs_indices.size();
    uint from_sol = (this->current_fs_page * solutions_per_page) + 1;
    uint to_sol = ((this->current_fs_page + 1) * solutions_per_page);
    if(to_sol > sol_total)
        to_sol = sol_total;
    QString s = QString::number(from_sol,10);
    s += FSE_FROM_TO_STRING;
    s += QString::number(to_sol,10);
    s += FSE_OF_STRING;
    s += QString::number(sol_total,10);
    this->ui->fullSolutionsShowLabel->setText(s);

    this->fullSolutionPageEdit->setText(QString::number(this->current_fs_page + 1,10));

    return SUCCESS;
}

int Posefi::PartialSolutionFirstPageButton_press(){
    return this->PartialSolutionChangePage(0);
}
int Posefi::PartialSolutionPrevPageButton_press(){
    return this->PartialSolutionChangePage(this->current_ps_page - 1);
}
int Posefi::PartialSolutionNextPageButton_press(){
    return this->PartialSolutionChangePage(this->current_ps_page + 1);
}
int Posefi::PartialSolutionLastPageButton_press(){
    int page_count = (this->ps_indices.size() / this->solutions_per_page);
    if((page_count * this->solutions_per_page) < this->ps_indices.size())
        page_count++;
    return this->PartialSolutionChangePage(page_count - 1);
}

int Posefi::FullSolutionFirstPageButton_press(){
    return this->FullSolutionChangePage(0);
}
int Posefi::FullSolutionPrevPageButton_press(){
    return this->FullSolutionChangePage(this->current_fs_page - 1);
}
int Posefi::FullSolutionNextPageButton_press(){
    return this->FullSolutionChangePage(this->current_fs_page + 1);
}
int Posefi::FullSolutionLastPageButton_press(){
    int page_count = (this->fs_indices.size() / this->solutions_per_page);
    if((page_count * this->solutions_per_page) < this->fs_indices.size())
        page_count++;
    return this->FullSolutionChangePage(page_count - 1);
}

int Posefi::PartialSolutionChangePage(int page){
    // If the new page is the same as the old page, it will result in a reload of the page.
    // That can be necessary when the solutions are sorted.

    if(page < 0){
        qInfo() << "error in Posefi::PartialSolutionChangePage: (page < 0)";
        return IMPOSSIBLE_PAGE_CHANGE;
    }

    int page_count = (this->ps_indices.size() / this->solutions_per_page);
    if((page_count * this->solutions_per_page) < this->ps_indices.size())
        page_count++;

    if(page > (page_count - 1)){
        qInfo() << "error in Posefi::PartialSolutionChangePage: (page > (page_count - 1))";
        return IMPOSSIBLE_PAGE_CHANGE;
    }

    this->current_ps_page = page;
    this->upcoming_ps_page_change = true;

    // Make all active elements fade out and add one to this->current_disable_animations.
    // Once all disable animations are done, in this->PSE_OpacityAnimationEnd(...) it will detect that
    // and call this->ShowNewPartialSolutions().
    if(!this->PSE_Order.empty()){
        int pse_index = this->PSE_Order[0];
        while(pse_index >= 0){
            PartialSolutionElement* pse = this->PartialSolutionElements[pse_index];
            this->PSE_FadeOutAnimation(pse,PSE_PAGE_CHANGE_FADEOUT_DURATION);
            this->Disable_PSE(pse);
            this->current_ps_disable_animations++;
            pse_index = this->PSE_Order[0];
            // The this->Disable_PSE(...) function puts element 0 at the end of the array and shifts
            // the later values forward. Therefore the next element to check will again be at index 0.
        }
    }

    return SUCCESS;
}

int Posefi::FullSolutionChangePage(int page){
    // If the new page is the same as the old page, it will result in a reload of the page.
    // That can be necessary when the solutions are sorted.

    if(page < 0){
        qInfo() << "error in Posefi::FullSolutionChangePage: (page < 0)";
        return IMPOSSIBLE_PAGE_CHANGE;
    }

    int page_count = (this->fs_indices.size() / this->solutions_per_page);
    if((page_count * this->solutions_per_page) < this->fs_indices.size())
        page_count++;

    if(page > (page_count - 1)){
        qInfo() << "error in Posefi::FullSolutionChangePage: (page > (page_count - 1))";
        return IMPOSSIBLE_PAGE_CHANGE;
    }

    this->current_fs_page = page;
    this->upcoming_fs_page_change = true;

    // Make all active elements fade out and add one to this->current_disable_animations.
    // Once all disable animations are done, in this->FSE_OpacityAnimationEnd(...) it will detect that
    // and call this->ShowNewFullSolutions().
    if(!this->FSE_Order.empty()){
        int fse_index = this->FSE_Order[0];
        while(fse_index >= 0){
            SolutionElement* fse = this->FullSolutionElements[fse_index];
            this->FSE_FadeOutAnimation(fse,FSE_PAGE_CHANGE_FADEOUT_DURATION);
            this->Disable_FSE(fse);
            this->current_fs_disable_animations++;
            fse_index = this->FSE_Order[0];
            // The this->Disable_FSE(...) function puts element 0 at the end of the array and shifts
            // the later values forward. Therefore the next element to check will again be at index 0.
        }
    }

    return SUCCESS;
}

int Posefi::PSE_FadeOutAnimation(PartialSolutionElement* pse, uint duration){
    if(pse == nullptr){
        qInfo() << "error in Posefi::PSE_AnimationFadeout: (pse == nullptr)";
        return NULL_POINTER;
    }
    pse->opacity_animation.stop();
    pse->opacity_animation.setStartValue(1);
    pse->opacity_animation.setEndValue(0);
    pse->opacity_animation.setDuration(duration);
    pse->opacity_animation.start();
    pse->SetEnabled(false);
    return SUCCESS;
}

int Posefi::FSE_FadeOutAnimation(SolutionElement* fse, uint duration){
    if(fse == nullptr){
        qInfo() << "error in Posefi::FSE_AnimationFadeout: (fse == nullptr)";
        return NULL_POINTER;
    }
    fse->opacity_animation.stop();
    fse->opacity_animation.setStartValue(1);
    fse->opacity_animation.setEndValue(0);
    fse->opacity_animation.setDuration(duration);
    fse->opacity_animation.start();
    fse->SetEnabled(false);
    return SUCCESS;
}

int Posefi::PSE_FadeInAnimation(PartialSolutionElement* pse, uint duration){
    if(pse == nullptr){
        qInfo() << "error in Posefi::PSE_AnimationFadein: (pse == nullptr)";
        return NULL_POINTER;
    }
    pse->opacity_animation.stop();
    pse->opacity_animation.setStartValue(0);
    pse->opacity_animation.setEndValue(1);
    pse->opacity_animation.setDuration(duration);
    pse->opacity_animation.start();
    pse->SetEnabled(true);
    return SUCCESS;
}

int Posefi::FSE_FadeInAnimation(SolutionElement* fse, uint duration){
    if(fse == nullptr){
        qInfo() << "error in Posefi::FSE_AnimationFadein: (fse == nullptr)";
        return NULL_POINTER;
    }
    fse->opacity_animation.stop();
    fse->opacity_animation.setStartValue(0);
    fse->opacity_animation.setEndValue(1);
    fse->opacity_animation.setDuration(duration);
    fse->opacity_animation.start();
    fse->SetEnabled(true);
    return SUCCESS;
}

// This function calculates the necessary geometry all by itself and starts the geometry + opasity animation.
// It does not set visibility, fold button state or reactivates the element in any other way.
int Posefi::PSE_AppearFromRightAnimation(PartialSolutionElement* pse){
    if(pse == nullptr){
        qInfo() << "error in Posefi::PSE_AnimationMoveX: (pse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = pse->order_index;
    if(order_index < 0)
        order_index = -order_index - 1;
    uint duration = PSE_FROM_RIGHT_ANIMATION_DURATION;
    int x1 = PSE_FROM_RIGHT_ANIMATION_X1;
    int x2 = SE_TOP_FRAME_GAP;
    int y = CalculatePartialSolutionElement_YPos(order_index);
    int w = pse->top_frame.width();
    int h = SE_FOLDED_HEIGHT;
    pse->geometry_animation.stop();
    pse->geometry_animation.setStartValue(QRect(x1,y,w,h));
    pse->geometry_animation.setEndValue(QRect(x2,y,w,h));
    pse->geometry_animation.setDuration(duration);
    pse->geometry_animation.start();
    this->PSE_FadeInAnimation(pse,duration);
    return SUCCESS;
}

// This function calculates the necessary geometry all by itself and starts the geometry + opasity animation.
// It does not set visibility, fold button state or reactivates the element in any other way.
int Posefi::FSE_AppearFromRightAnimation(SolutionElement* fse){
    if(fse == nullptr){
        qInfo() << "error in Posefi::FSE_AnimationMoveX: (fse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = fse->order_index;
    if(order_index < 0)
        order_index = -order_index - 1;
    uint duration = FSE_FROM_RIGHT_ANIMATION_DURATION;
    int x1 = FSE_FROM_RIGHT_ANIMATION_X1;
    int x2 = SE_TOP_FRAME_GAP;
    int y = CalculateFullSolutionElement_YPos(order_index);
    int w = fse->top_frame.width();
    int h = SE_FOLDED_HEIGHT;
    fse->geometry_animation.stop();
    fse->geometry_animation.setStartValue(QRect(x1,y,w,h));
    fse->geometry_animation.setEndValue(QRect(x2,y,w,h));
    fse->geometry_animation.setDuration(duration);
    fse->geometry_animation.start();
    this->FSE_FadeInAnimation(fse,duration);
    return SUCCESS;
}

// This function sets the necessary geometry all by itself and starts the geometry + opasity animation.
// It does not disable the element in any way though.
int Posefi::PSE_DisappearToRightAnimation(PartialSolutionElement* pse){
    if(pse == nullptr){
        qInfo() << "error in Posefi::PSE_AnimationMoveX: (pse == nullptr)";
        return NULL_POINTER;
    }
    uint duration = PSE_REMOVE_ANIMATION_DURATION;
    int x1 = SE_TOP_FRAME_GAP;
    int x2 = PSE_TO_RIGHT_ANIMATION_X2;
    int y = pse->top_frame.y();
    int w = pse->top_frame.width();
    int h = pse->top_frame.height();
    pse->geometry_animation.stop();
    pse->geometry_animation.setStartValue(QRect(x1,y,w,h));
    pse->geometry_animation.setEndValue(QRect(x2,y,w,h));
    pse->geometry_animation.setDuration(duration);
    pse->geometry_animation.start();
    this->PSE_FadeOutAnimation(pse,duration);
    return SUCCESS;
}

// This function sets the necessary geometry all by itself and starts the geometry + opasity animation.
// It does not disable the element in any way though.
int Posefi::FSE_DisappearToRightAnimation(SolutionElement* fse){
    if(fse == nullptr){
        qInfo() << "error in Posefi::FSE_AnimationMoveX: (fse == nullptr)";
        return NULL_POINTER;
    }
    uint duration = FSE_REMOVE_ANIMATION_DURATION;
    int x1 = SE_TOP_FRAME_GAP;
    int x2 = FSE_TO_RIGHT_ANIMATION_X2;
    int y = fse->top_frame.y();
    int w = fse->top_frame.width();
    int h = fse->top_frame.height();
    fse->geometry_animation.stop();
    fse->geometry_animation.setStartValue(QRect(x1,y,w,h));
    fse->geometry_animation.setEndValue(QRect(x2,y,w,h));
    fse->geometry_animation.setDuration(duration);
    fse->geometry_animation.start();
    this->FSE_FadeOutAnimation(fse,duration);
    return SUCCESS;
}

// Calculates and sets the necessary geometry of pse->top_frame, then calls PSE_FadeInAnimation(...) for the animation.
int Posefi::PSE_PageChangeFadeInAnimation(PartialSolutionElement* pse){
    if(pse == nullptr){
        qInfo() << "error in Posefi::PSE_AnimationMoveX: (pse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = pse->order_index;
    if(order_index < 0)
        order_index = -order_index - 1;
    int x = SE_TOP_FRAME_GAP;
    int y = CalculatePartialSolutionElement_YPos(order_index);
    int w = pse->top_frame.width();
    int h = SE_FOLDED_HEIGHT;
    //pse->top_frame.setGeometry(x,y,w,h);
    pse->geometry_animation.stop();
    pse->geometry_animation.setStartValue(QRect(x,y,w,h));
    pse->geometry_animation.setEndValue(QRect(x,y,w,h));
    pse->geometry_animation.setDuration(PSE_PAGE_CHANGE_FADEIN_DURATION);
    pse->geometry_animation.start();
    // couldn't find a bug that happens if you don't run the geometry animation

    return this->PSE_FadeInAnimation(pse,PSE_PAGE_CHANGE_FADEIN_DURATION);
}

// Calculates and sets the necessary geometry of fse->top_frame, then calls FSE_FadeInAnimation(...) for the animation.
int Posefi::FSE_PageChangeFadeInAnimation(SolutionElement* fse){
    if(fse == nullptr){
        qInfo() << "error in Posefi::FSE_AnimationMoveX: (fse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = fse->order_index;
    if(order_index < 0)
        order_index = -order_index - 1;
    int x = SE_TOP_FRAME_GAP;
    int y = CalculateFullSolutionElement_YPos(order_index);
    int w = fse->top_frame.width();
    int h = SE_FOLDED_HEIGHT;
    //fse->top_frame.setGeometry(x,y,w,h);
    fse->geometry_animation.stop();
    fse->geometry_animation.setStartValue(QRect(x,y,w,h));
    fse->geometry_animation.setEndValue(QRect(x,y,w,h));
    fse->geometry_animation.setDuration(FSE_PAGE_CHANGE_FADEIN_DURATION);
    fse->geometry_animation.start();
    // couldn't find a bug that happens if you don't run the geometry animation

    return this->FSE_FadeInAnimation(fse,FSE_PAGE_CHANGE_FADEIN_DURATION);
}

int Posefi::Disable_PSE(PartialSolutionElement* pse){
    if(pse == nullptr){
        qInfo() << "error in Disable_PSE: (pse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = pse->order_index;
    if(order_index < 0){    // This happens if the element was already disabled
        qInfo() << "error in Disable_PSE: (order_index < 0)";
        return ARRAY_OUT_OF_RANGE;
    }
    if((uint)order_index >= this->PSE_Order.size()){ // This should not happen
        qInfo() << "error in Disable_PSE: ((uint)order_index >= this->PartialSolutionElementsOrder.size())";
        return ARRAY_OUT_OF_RANGE;
    }
    int element_index = this->PSE_Order[order_index];
    if(element_index < 0 || (uint)element_index >= this->PartialSolutionElements.size()){// This should not happen
        qInfo() << "error in Disable_PSE: (element_index < 0 || element_index >= this->PartialSolutionElements.size())";
        return ARRAY_OUT_OF_RANGE;
    }

    // shift all entries after [order_index] in the order one position back
    uint order_end_index = this->PSE_Order.size() - 1;
    for(uint i = order_index; i < order_end_index; i++){

        int new_element_index = this->PSE_Order[i+1];
        int new_order_index = i;
        this->PSE_Order[i] = new_element_index;
        if(new_element_index >= 0)
            this->PartialSolutionElements[new_element_index]->order_index = new_order_index;
    }

    // add the negative equivalent of the index at the back
    this->PSE_Order[order_end_index] = -(element_index +1);
    int oi = -(pse->order_index + 1); // negative equivalent of the order_index
    pse->order_index = oi; // negative is interpreted as disabled

    return SUCCESS;
}

int Posefi::Disable_FSE(SolutionElement* fse){
    if(fse == nullptr){
        qInfo() << "error in Disable_FSE: (fse == nullptr)";
        return NULL_POINTER;
    }
    int order_index = fse->order_index;
    if(order_index < 0){    // This happens if the element was already disabled
        qInfo() << "error in Disable_FSE: (order_index < 0)";
        return ARRAY_OUT_OF_RANGE;
    }
    if((uint)order_index >= this->FSE_Order.size()){ // This should not happen
        qInfo() << "error in Disable_FSE: ((uint)order_index >= this->FullSolutionElementsOrder.size())";
        return ARRAY_OUT_OF_RANGE;
    }
    int element_index = this->FSE_Order[order_index];
    if(element_index < 0 || (uint)element_index >= this->FullSolutionElements.size()){// This should not happen
        qInfo() << "error in Disable_FSE: (element_index < 0 || element_index >= this->FullSolutionElements.size())";
        return ARRAY_OUT_OF_RANGE;
    }

    // shift all entries after [order_index] in the order one position back
    uint order_end_index = this->FSE_Order.size() - 1;
    for(uint i = order_index; i < order_end_index; i++){

        int new_element_index = this->FSE_Order[i+1];
        int new_order_index = i;
        this->FSE_Order[i] = new_element_index;
        if(new_element_index >= 0)
            this->FullSolutionElements[new_element_index]->order_index = new_order_index;
    }

    // add the negative equivalent of the index at the back
    this->FSE_Order[order_end_index] = -(element_index +1);
    int oi = -(fse->order_index + 1); // negative equivalent of the order_index
    fse->order_index = oi; // negative is interpreted as disabled

    return SUCCESS;
}

int Posefi::PartialSolutionPageEdit_change(){
    if(this->current_ps_page < 0){
        this->partialSolutionPageEdit->setText("");
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    QString s = this->partialSolutionPageEdit->text();
    if(!DecCheck(s,false)){
        this->partialSolutionPageEdit->setText(QString::number(this->current_ps_page + 1,10));
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    int page_count = this->ps_indices.size() / this->solutions_per_page;
    if(page_count * this->solutions_per_page < this->ps_indices.size())
        page_count++;

    int new_page = s.toLong(nullptr,10) - 1;
    if(new_page >= page_count)
        new_page = page_count - 1;
    if(new_page < 0 ||  new_page == this->current_ps_page){
        this->partialSolutionPageEdit->setText(QString::number(this->current_ps_page + 1,10));
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    this->PartialSolutionChangePage(new_page);

    // Unhandled case: If the line edit loses focus because of a page change button press,
    // it leads to two page changes in a row. Optimally in this case the line edit page change
    // would be canceled.

    return SUCCESS;
}

int Posefi::FullSolutionPageEdit_change(){
    if(this->current_fs_page < 0){
        this->fullSolutionPageEdit->setText("");
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    QString s = this->fullSolutionPageEdit->text();
    if(!DecCheck(s,false)){
        this->fullSolutionPageEdit->setText(QString::number(this->current_fs_page + 1,10));
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    int page_count = this->fs_indices.size() / this->solutions_per_page;
    if(page_count * this->solutions_per_page < this->fs_indices.size())
        page_count++;

    int new_page = s.toLong(nullptr,10) - 1;
    if(new_page >= page_count)
        new_page = page_count - 1;
    if(new_page < 0 ||  new_page == this->current_fs_page){
        this->fullSolutionPageEdit->setText(QString::number(this->current_fs_page + 1,10));
        return IMPOSSIBLE_PAGE_CHANGE;
    }
    this->FullSolutionChangePage(new_page);

    // Unhandled case: If the line edit loses focus because of a page change button press,
    // it leads to two page changes in a row. Optimally in this case the line edit page change
    // would be canceled.

    return SUCCESS;
}

int Posefi::PartialSolutionEnableSubSearchButtons(){
    for(uint i = 0; i < this->PartialSolutionElements.size(); i++){
        this->PartialSolutionElements[i]->sub_search_button.setEnabled(true);
    }
    return SUCCESS;
}
int Posefi::PartialSolutionDisableSubSearchButtons(){
    for(uint i = 0; i < this->PartialSolutionElements.size(); i++){
        this->PartialSolutionElements[i]->sub_search_button.setEnabled(false);
    }
    return SUCCESS;
}


// -----------------------------------------------------------------------
// ----- Sub Search ------------------------------------------------------


int Posefi::InitiateSubSearch(SolutionCopy* ps){
    if(ps == nullptr){
        qInfo() << "error in Posefi::InitiateSubSearch: (ps == nullptr)";
        return NULL_POINTER;
    }
    qInfo() << "fff ";
    // TODO
    return SUCCESS;
}




// ------------------------------------------------------------------------
// ----- Menu Bar ---------------------------------------------------------

#ifndef QT_NO_CONTEXTMENU
void Posefi::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    //menu.addAction(cutAct);
    //menu.addAction(copyAct);
    //menu.addAction(pasteAct);
    //menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

int Posefi::SetColorTheme(){
    switch(this->color_theme){
        case LIGHT_THEME:
        this->table_error_color = QBrush(TABLE_LIGHT_ERROR);
        this->table_normal_color = QBrush(TABLE_LIGHT);
        this->table_grey_color = QBrush(TABLE_LIGHT_GREY);
        this->table_error_grey_color = QBrush(TABLE_LIGHT_ERROR_GREY);
        this->text_error_color = QBrush(TEXT_LIGHT_ERROR);
        this->text_normal_color = QBrush(TEXT_LIGHT);
        this->text_grey_color = QBrush(TEXT_LIGHT_GREY);
        this->text_error_grey_color = QBrush(TEXT_LIGHT_ERROR_GREY);
        qApp->setStyleSheet(this->light_theme_css);
        break;

    default: // DARK_THEME
        this->table_error_color = QBrush(TABLE_DARK_ERROR);
        this->table_normal_color = QBrush(TABLE_DARK);
        this->table_grey_color = QBrush(TABLE_DARK_GREY);
        this->table_error_grey_color = QBrush(TABLE_DARK_ERROR_GREY);
        this->text_error_color = QBrush(TEXT_DARK_ERROR);
        this->text_normal_color = QBrush(TEXT_DARK);
        this->text_grey_color = QBrush(TEXT_DARK_GREY);
        this->text_error_grey_color = QBrush(TEXT_DARK_ERROR_GREY);

        if(this->ui->pauseButton->isEnabled()){

        }
        qApp->setStyleSheet(this->dark_theme_css);
        break;
    }
    return SUCCESS;
}

int Posefi::LoadColorThemes(){
    QFile File(DARK_THEME_CSS);
    int r = SUCCESS;
    if(File.open(QFile::ReadOnly)){
        this->dark_theme_css = QLatin1String(File.readAll());
    }
    else{
        r = FILE_NOT_OPEN;
        qInfo() << "Unable to open " << "\"" << DARK_THEME_CSS << "\"";
    }

    QFile File2(LIGHT_THEME_CSS);
    if(File2.open(QFile::ReadOnly))
        this->light_theme_css = QLatin1String(File2.readAll());
    else {
        r = FILE_NOT_OPEN;
        qInfo() << "Unable to open " << "\"" << LIGHT_THEME_CSS << "\"";
    }

    return r;
}

// -------- Storage and loading ---------------------

int Posefi::OpenFile(){
    // TODO
    return SUCCESS;
}

int Posefi::SaveFile(){
    // TODO
    return SUCCESS;
}

int Posefi::SaveInputs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Input Data", lastDirectory, "Input Data (*.psfi);;All Files (*)");
    if (fileName.isEmpty())
        return FILE_NOT_OPEN;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, "Unable to open file", file.errorString());
        return FILE_NOT_OPEN;
    }
    QTextStream out(&file);
    StreamInputsToFile(out);
    lastDirectory = fileName;
    file.close();

    if(!fileName.isEmpty())
    {
        qInfo() << fileName;
        lastDirectory = fileName;
        QSettings settings("Posefi");
        settings.setValue("dir", QVariant(lastDirectory));
    }

    return SUCCESS;
}

void Posefi::StreamInputsToFile(QTextStream& stream)
{
    stream << QString::number(xyzLable_radioButtons.checkedId()) << "\n";
    stream << QString::number(findXYZ_radioButtons.checkedId()) << "\n";
    stream << QString::number(ui->checkBoxX_Resettable->checkState()) << "\n";
    stream << QString::number(ui->checkBoxZ_Resettable->checkState()) << "\n";
    stream << defaultFirstLineEdit->text() << "\n";
    stream << defaultSecondLineEdit->text() << "\n";

    QString checkBoxesActions;
    QString checkBoxesSearched;

    // actions
    for(int r = 0; r < ui->actionTable->rowCount(); r++)
    {
        for(int c = 0; c < ui->actionTable->columnCount(); c++)
        {
            stream << ui->actionTable->item(r, c)->text();
            if(c == ui->actionTable->columnCount() - 1)
                stream << "\n";
            else
                stream << "\t";
        }
        if(LeftSideActionTable.size() > r)
        {
            if (!checkBoxesActions.isEmpty())
                checkBoxesActions += "\t";
            int state = LeftSideActionTable[r]->CheckBox.checkState();
            checkBoxesActions += QString::number(state);
        }
    }
    stream << "actions end\n";

    // searched
    for(int r = 0; r < ui->wantedPosTable->rowCount(); r++)
    {
        for(int c = 0; c < ui->wantedPosTable->columnCount(); c++)
        {
            stream << ui->wantedPosTable->item(r, c)->text();
            if(c == ui->wantedPosTable->columnCount() - 1)
                stream << "\n";
            else
                stream << "\t";
        }
        if(LeftSideWantedTable.size() > r)
        {
            if (!checkBoxesSearched.isEmpty())
                checkBoxesSearched += "\t";
            int state = LeftSideWantedTable[r]->CheckBox.checkState();
            checkBoxesSearched += QString::number(state);
        }
    }
    stream << "searched end\n";

    // active checkboxes actions
    stream << checkBoxesActions << "\n";
    stream << checkBoxesSearched << "\n";
}

int Posefi::LoadInputs()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Input Data", lastDirectory,  "Input Data (*.psfi);;All Files (*)");
    if (fileName.isEmpty())
        return FILE_NOT_OPEN;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Unable to open file", file.errorString());
        return FILE_NOT_OPEN;
    }
    QTextStream in(&file);
    StreamInputsFromFile(in);

    if(!fileName.isEmpty())
    {
        lastDirectory = fileName;
        QSettings settings("Posefi");
        settings.setValue("dir", QVariant(lastDirectory));
    }

    return SUCCESS;
}

void Posefi::StreamInputsFromFile(QTextStream& stream)
{
    QString line;

    // xy / xz / yz
    stream.readLineInto(&line);
    int xyzToggle = line.toInt();
    if(xyzToggle == 0)
        ui->radioButton_xy->setChecked(true);
    else if(xyzToggle == 2)
        ui->radioButton_yz->setChecked(true);
    else
        ui->radioButton_xz->setChecked(true);
    LabelXYZ_RadioButton_toggle(xyzToggle);

    // find x / find z / find both
    stream.readLineInto(&line);
    int findToggle = line.toInt();
    if(findToggle == 0)
        ui->radioButton_xyz_first->setChecked(true);
    else if(findToggle == 1)
        ui->radioButton_xyz_second->setChecked(true);
    else
        ui->radioButton_xyz_both->setChecked(true);
    FindXZ_RadioButton_toggle(findToggle);

    // x / z resettable
    stream.readLineInto(&line);
    int resettableX = line.toInt();
    ui->checkBoxX_Resettable->setCheckState(static_cast<Qt::CheckState>(resettableX));
    stream.readLineInto(&line);
    int resettableZ = line.toInt();
    ui->checkBoxZ_Resettable->setCheckState(static_cast<Qt::CheckState>(resettableZ));

    // default position
    stream.readLineInto(&line);
    defaultFirstLineEdit->setText(line);
    stream.readLineInto(&line);
    defaultSecondLineEdit->setText(line);

    // actions
    stream.readLineInto(&line);
    while(line != "actions end")
    {
        QStringList row = line.split("\t");
        int colCount = ui->actionTable->columnCount();
        if(row.size() != colCount)
        {
            qInfo() << "Error: Column count of action missmatch!";
            continue;
        }
        AddAction();
        int r = ui->actionTable->rowCount() - 1;
        QStringList::iterator it = row.begin();
        for(int c = 0; c < colCount; c++, it++)
        {
            ui->actionTable->item(r, c)->setText(*it);
        }
        stream.readLineInto(&line);
    }

    // searched positions
    stream.readLineInto(&line);
    while(line != "searched end")
    {
        QStringList row = line.split("\t");
        int colCount = ui->wantedPosTable->columnCount();
        if(row.size() != colCount)
        {
            qInfo() << "Error: Column count of searched missmatch!";
            continue;
        }
        AddWantedPos();
        int r = ui->wantedPosTable->rowCount() - 1;
        QStringList::iterator it = row.begin();
        for(int c = 0; c < colCount; c++, it++)
        {
            ui->wantedPosTable->item(r, c)->setText(*it);
        }
        stream.readLineInto(&line);
    }

    // check boxes actions
    stream.readLineInto(&line);
    QStringList actionCheckBoxStrings = line.split("\t");
    QStringList::iterator itActionCheck = actionCheckBoxStrings.begin();
    int rAction = 0;
    while(itActionCheck != actionCheckBoxStrings.end() && rAction < LeftSideActionTable.size())
    {
        int active = (*itActionCheck).toInt();
         LeftSideActionTable[rAction]->CheckBox.setCheckState(static_cast<Qt::CheckState>(active));
        itActionCheck++;
        rAction++;
    }

    // check boxes searched
    stream.readLineInto(&line);
    QStringList searchedCheckBoxStrings = line.split("\t");
    QStringList::iterator itSearchedCheck = searchedCheckBoxStrings.begin();
    int rSearched = 0;
    while(itSearchedCheck != searchedCheckBoxStrings.end() && rSearched < LeftSideWantedTable.size())
    {
        int active = (*itSearchedCheck).toInt();
        LeftSideWantedTable[rSearched]->CheckBox.setCheckState(static_cast<Qt::CheckState>(active));
        itSearchedCheck++;
        rSearched++;
    }
}

void Posefi::LoadSettings()
{
    QSettings settings("Posefi");
    QVariant dirVar = settings.value("dir");
    if(dirVar.isValid())
        lastDirectory = dirVar.toString();
}
void Posefi::SaveSettings()
{


}



// ----------------------------------------------------------------
// ----- Preparations for running the search ----------------------

// checks if both tables have at least one active entry, then sets the start button accordingly
int Posefi::SetStartButtonState(){

    // don't change state during a search
    // but set it after the search since the tables could have been edited during the search
    if(this->main_search_is_running)
        return SUCCESS;

    int action_rc = this->ui->actionTable->rowCount();
    int wanted_rc = this->ui->wantedPosTable->rowCount();
    bool action_state = false;
    bool wanted_state = false;
    if(action_rc && wanted_rc && this->is_first_default_pos_valid && this->is_second_default_pos_valid){
        if ((int)this->LeftSideActionTable.size() < action_rc || (int)this->LeftSideWantedTable.size() < wanted_rc){
            qInfo() << "array out of range error in SetStartButtonState()";
            return ARRAY_OUT_OF_RANGE;
        }
        for(int i = 0; i < action_rc; i++){
            if (this->LeftSideActionTable[i]->Used){
                bool invalid = false;
                for(uint j = 0; j < LeftSideActionTable[i]->is_entry_valid.size(); j++)
                {
                    if((j == ACTION_POS1 && ui->radioButton_xyz_second->isChecked()) || (j == ACTION_POS2 && ui->radioButton_xyz_first->isChecked()))
                        continue;
                    if(!LeftSideActionTable[i]->is_entry_valid[j])
                    {
                        invalid = true;
                        break;
                    }

                }
                if(!invalid)
                {
                    action_state = true;
                    break;
                }
            }
        }
        for(int i = 0; i < wanted_rc; i++){// TODO BUG
            if (this->LeftSideWantedTable[i]->Used){
                bool invalid = false;
                for(uint j = 0; j < LeftSideWantedTable[i]->is_entry_valid.size(); j++)
                {
                    if(((j == WANTED_POS1 || j == WANTED_TOL1 || j == WANTED_AND1) && ui->radioButton_xyz_second->isChecked())
                        || ((j == WANTED_POS2 || j == WANTED_TOL2 || j == WANTED_AND2) && ui->radioButton_xyz_first->isChecked()))
                        continue;
                    if(!LeftSideWantedTable[i]->is_entry_valid[j])
                    {
                        invalid = true;
                        break;
                    }
                }
                if(!invalid)
                {
                    wanted_state = true;
                    break;
                }
            }
        }
    }
    this->ui->startButton->setEnabled(action_state && wanted_state);
    return SUCCESS;
}

int Posefi::StartButton_press(){
    if (this->TableAutoCompletion() == ARRAY_OUT_OF_RANGE){
        // TODO error message
        return ARRAY_OUT_OF_RANGE;
    }

    // TODO error message if data is likely invalid (or implement this in the ValidDataCheck() function)
    this->ValidDataCheck();
    this->CreateMainSearch();
    return SUCCESS;
}

// TODO light theme buttons
int Posefi::PauseButton_press(){
    if(this->main_search_is_paused){
        this->ui->pauseButton->setProperty("Paused", QVariant(false));
        this->ui->pauseButton->style()->unpolish(this->ui->pauseButton);
        this->ui->pauseButton->style()->polish(this->ui->pauseButton);
        this->main_search_is_paused = false;
        this->RunningMainSearch();
    }
    else{
        this->ui->pauseButton->setProperty("Paused", QVariant(true));
        this->ui->pauseButton->style()->unpolish(this->ui->pauseButton);
        this->ui->pauseButton->style()->polish(this->ui->pauseButton);
        this->main_search_is_paused = true;
    }
    return SUCCESS;
}

int Posefi::CancelButton_press(){
    this->main_search_is_running = false;
    this->ui->pauseButton->setEnabled(false);
    this->ui->cancelButton->setEnabled(false);
    this->ui->progressBarMainSearch->hide();
    this->ui->mainSearchProgressLabel->setText("");
    this->SetStartButtonState();
    return SUCCESS;
}

int Posefi::FinishMainSearch(){
    this->ui->mainSearchProgressLabel->setText(ProgressLabelString(this->main_search.iterations_done,this->main_search.iterations_total));
    this->ui->progressBarMainSearch->setValue(100);

    this->main_search_is_running = false;
    this->ui->pauseButton->setEnabled(false);
    this->ui->cancelButton->setEnabled(false);
    this->SetStartButtonState();
    return SUCCESS;
}

int Posefi::DefaultFirstLineEdit_change(const QString& s){
    bool b = HexCheck(s);
    if(b != this->is_first_default_pos_valid){
        this->is_first_default_pos_valid = b;
        this->ui->redFrameDefaultPos1->setVisible(!b);
    }
    SetStartButtonState();
    return SUCCESS;
}

int Posefi::DefaultSecondLineEdit_change(const QString& s){
    bool b = HexCheck(s);
    if(b != this->is_second_default_pos_valid){
        this->is_second_default_pos_valid = b;
        this->ui->redFrameDefaultPos2->setVisible(!b);
    }
    SetStartButtonState();
    return SUCCESS;
}

int Posefi::TableAutoCompletion(){
    UINT action_rc = this->ui->actionTable->rowCount();
    UINT wanted_rc = this->ui->wantedPosTable->rowCount();
    if (this->LeftSideActionTable.size() < action_rc || this->LeftSideWantedTable.size() < wanted_rc){
        qInfo() << "array out of range error in TableAutoCompletion()";
        return ARRAY_OUT_OF_RANGE;
    }
    for (UINT r = 0; r < action_rc; r++){
        if (!this->LeftSideActionTable[r]->Used)
            continue;

        for (UINT c = 0; c < this->ui->actionTable->columnCount(); c++){
            if ((       ActionColumnData[c].xyz_case == IS_XYZ_FIRST && !this->find_xyz_first)
                    || (ActionColumnData[c].xyz_case == IS_XYZ_SECOND && !this->find_xyz_second)
                    || !this->LeftSideActionTable[r]->is_entry_valid[c]
                    || this->ui->actionTable->item(r,c)->text().size() == ActionColumnData[c].desired_length)
                continue;

            switch(ActionColumnData[c].string_case){
            case HEX:
                this->ui->actionTable->item(r,c)->setText(AdjustHexString(this->ui->actionTable->item(r,c)->text(), ActionColumnData[c].desired_length, ActionColumnData[c].filler_character));
                break;
            case DEC:
                if (!this->ui->actionTable->item(r,c)->text().length())
                    this->ui->actionTable->item(r,c)->setText("0");
                break;
            case NAME:
                if (!this->ui->actionTable->item(r,c)->text().size())
                    this->ui->actionTable->item(r,c)->setText(ActionColumnData[c].filler_character + QString::fromStdString(std::to_string(r)));
                break;
            }
        }
    }

    for (UINT r = 0; r < wanted_rc; r++){
        if (!this->LeftSideWantedTable[r]->Used)
            continue;

        for (UINT c = 0; c < this->ui->wantedPosTable->columnCount(); c++){
            if ((       WantedColumnData[c].xyz_case == IS_XYZ_FIRST && !this->find_xyz_first)
                    || (WantedColumnData[c].xyz_case == IS_XYZ_SECOND && !this->find_xyz_second)
                    || !this->LeftSideWantedTable[r]->is_entry_valid[c]
                    || this->ui->wantedPosTable->item(r,c)->text().size() == WantedColumnData[c].desired_length)
                continue;

            switch(WantedColumnData[c].string_case){
            case HEX:
                if ((WantedColumnData[c].filler_character == "F" || WantedColumnData[c].filler_character == "f") && this->ui->wantedPosTable->item(r,c)->text() == "")
                    this->ui->wantedPosTable->item(r,c)->setText(AdjustHexString(this->ui->wantedPosTable->item(r,c)->text(), WantedColumnData[c].desired_length, WantedColumnData[c].filler_character));
                else
                    this->ui->wantedPosTable->item(r,c)->setText(AdjustHexString(this->ui->wantedPosTable->item(r,c)->text(), WantedColumnData[c].desired_length, "0"));
                break;
            case DEC:
                if (!this->ui->wantedPosTable->item(r,c)->text().length())
                    this->ui->wantedPosTable->item(r,c)->setText("0");
                break;
            case NAME:
                if (!this->ui->wantedPosTable->item(r,c)->text().size())
                    this->ui->wantedPosTable->item(r,c)->setText(WantedColumnData[c].filler_character + QString::fromStdString(std::to_string(r)));
                break;
            }
        }
    }

    this->defaultFirstLineEdit->setText(AdjustHexString(this->defaultFirstLineEdit->text(),8,"0"));
    this->defaultSecondLineEdit->setText(AdjustHexString(this->defaultSecondLineEdit->text(),8,"0"));

    return SUCCESS;
}

// Check if data appears to contain errors
int Posefi::ValidDataCheck(){
    // TODO
    return 0;
}

int Posefi::CreateMainSearch(){
    main_search.ClearData();
    main_search_partial_solutions_printed = 0;
    main_search_full_solutions_printed = 0;

    // convert default position
    WORD pos1 = (this->find_xyz_first) ? (this->defaultFirstLineEdit->text()).toULong(nullptr,16) : NULL_VALUE;
    WORD pos2 = (this->find_xyz_second) ? (this->defaultSecondLineEdit->text()).toULong(nullptr,16) : NULL_VALUE;
    bool r1 = this->ui->checkBoxX_Resettable->isChecked();
    bool r2 = this->ui->checkBoxZ_Resettable->isChecked();
    uint xyz_index = this->xyzLable_radioButtons.checkedId();
    if(this->main_search.SetDefaultPosition(pos1, pos2, this->find_xyz_first, this->find_xyz_second, r1, r2, xyz_index)
            & NO_POSITION_SEARCHED)
        qInfo() << "NO_POSITION_SEARCHED error in Posefi::CreateMainSearch()";

    // convert actions
    QString name;
    HWORD angle;
    int min, max;
    int cost;

    uint action_count = this->ui->actionTable->rowCount();
    if(action_count > this->LeftSideActionTable.size()){
        action_count = this->LeftSideActionTable.size();
        qInfo() << "error in Posefi::CreateMainSearch: (action_count > this->LeftSideActionTable.size())";
    }
    for (uint i = 0; i < action_count; i++){

        // check if action is active and if all entries have valid values. If not, skip the action.
        if(!this->LeftSideActionTable[i]->Used)
            continue;
        bool entries_valid = true;
        for(uint j = 0; j < LeftSideActionTable[i]->is_entry_valid.size(); j++)
        {
            if((j == ACTION_POS1 && ui->radioButton_xyz_second->isChecked()) || (j == ACTION_POS2 && ui->radioButton_xyz_first->isChecked()))
                continue;
            if(!LeftSideActionTable[i]->is_entry_valid[j])
            {
                entries_valid = false;
                break;
            }
        }

        if(!entries_valid)
            continue;

        // gather data from the row and add an action to the "Search" object
        name = this->ui->actionTable->item(i,ACTION_NAME)->text();
        pos1 = (this->find_xyz_first) ? this->ui->actionTable->item(i,ACTION_POS1)->text().toULong(nullptr,16) : NULL_VALUE;
        pos2 = (this->find_xyz_second) ? this->ui->actionTable->item(i,ACTION_POS2)->text().toULong(nullptr,16) : NULL_VALUE;
        angle = this->ui->actionTable->item(i,ACTION_ANGLE)->text().toUInt(nullptr,16);
        min = this->ui->actionTable->item(i,ACTION_MIN)->text().toInt();
        max = this->ui->actionTable->item(i,ACTION_MAX)->text().toInt();
        cost = this->ui->actionTable->item(i,ACTION_COST)->text().toInt();
//#define CREATE_MAIN_SEARCH_DEBUG
#ifndef CREATE_MAIN_SEARCH_DEBUG
        this->main_search.AddAction(name, pos1, pos2, angle, min, max, cost);
#else
        QString debug_amount_order;
        this->main_search.AddAction(name, pos1, pos2, angle, min, max, cost, &debug_amount_order);
        qInfo() << debug_amount_order;
#endif
    }

    // convert searched positions
    int tol1, tol2;
    WORD and1, and2;
    int maxcost;

    uint searched_count = this->ui->wantedPosTable->rowCount();
    if(searched_count > this->LeftSideWantedTable.size()){
        searched_count = this->LeftSideWantedTable.size();
        qInfo() << "error in Posefi::CreateMainSearch: (searched_count > this->LeftSideWantedTable.size())";
    }

    for (int i = 0; i < this->ui->wantedPosTable->rowCount(); i++){

        // check if searched is active and if all entries have valid values. If not, skip the searched.
        if(!this->LeftSideWantedTable[i]->Used)
            continue;
        bool entries_valid = true;
        for(uint j = 0; j < LeftSideWantedTable[i]->is_entry_valid.size(); j++)
        {
            if(((j == WANTED_POS1 || j == WANTED_TOL1 || j == WANTED_AND1) && ui->radioButton_xyz_second->isChecked())
                || ((j == WANTED_POS2 || j == WANTED_TOL2 || j == WANTED_AND2) && ui->radioButton_xyz_first->isChecked()))
                continue;
            if(!LeftSideWantedTable[i]->is_entry_valid[j])
            {
                entries_valid = false;
                break;
            }
        }
        if(!entries_valid)
            continue;

        // gather data from the row and add a searched to the "Search" object
        name = this->ui->wantedPosTable->item(i,WANTED_NAME)->text();
        pos1 = (this->find_xyz_first) ? this->ui->wantedPosTable->item(i,WANTED_POS1)->text().toULong(nullptr,16) : NULL_VALUE;
        pos2 = (this->find_xyz_second) ? this->ui->wantedPosTable->item(i,WANTED_POS2)->text().toULong(nullptr,16) : NULL_VALUE;
        tol1 = (this->find_xyz_first) ? this->ui->wantedPosTable->item(i,WANTED_TOL1)->text().toInt() : NULL_VALUE;
        tol2 = (this->find_xyz_second) ? this->ui->wantedPosTable->item(i,WANTED_TOL2)->text().toInt() : NULL_VALUE;
        and1 = (this->find_xyz_first) ? this->ui->wantedPosTable->item(i,WANTED_AND1)->text().toULong(nullptr,16) : NULL_VALUE;
        and2 = (this->find_xyz_second) ? this->ui->wantedPosTable->item(i,WANTED_AND2)->text().toULong(nullptr,16) : NULL_VALUE;
        maxcost = this->ui->wantedPosTable->item(i,WANTED_MAXCOST)->text().toInt();
        this->main_search.AddSearched(name, pos1, pos2, tol1, tol2, and1, and2, maxcost);
    }

#ifndef CREATE_MAIN_SEARCH_DEBUG
    this->main_search.ConvertPositionData();
#else
    QString debug_str;
    if(this->main_search.ConvertPositionData(&debug_str) != SUCCESS)
        qInfo() << debug_str;
#endif

    this->PrintMainSearch();

    this->ui->progressBarMainSearch->show();
    this->ui->progressBarMainSearch->setValue(0);
    this->ui->mainSearchProgressLabel->setText((ProgressLabelString(this->main_search.iterations_done,this->main_search.iterations_total)));
    this->main_search_is_running = true;
    this->main_search_is_paused = false;

    this->ui->startButton->setEnabled(false);
    this->ui->pauseButton->setEnabled(true);
    this->ui->pauseButton->setProperty("Paused", QVariant(false));
    this->ui->pauseButton->style()->unpolish(this->ui->pauseButton);
    this->ui->cancelButton->setEnabled(true);

    qInfo() << "running main search...";
    this->RunningMainSearch();

    return SUCCESS;
}

int Posefi::RunningMainSearch(){
    if(!this->main_search_is_running || this->main_search_is_paused)
        return SUCCESS;
    for(uint64_t i = 0; i < ITERATIONS_PER_SEARCH_CYCLE; i++){
        int result = this->main_search.Iteration();
        if(result & FOUND_PARTIAL_SOLUTION){
            for(uint j = this->main_search_partial_solutions_printed; j < this->main_search.partial_solutions.size(); j++){

                this->partial_solution_copies.push_back(SolutionCopy(this->main_search,PARTIAL_SOLUTION,j));
                this->ps_indices.push_back(ps_indices.size());
                //AddOrReplaceSolution(main_search, partial_solution_copies, PARTIAL_SOLUTION, partial_solution_copies_free_slots, ps_indices, j);

                this->ShowNewPartialSolutions();
                this->PrintSolution(PARTIAL_SOLUTION,this->main_search,j);
                this->main_search_partial_solutions_printed++;
            }
        }
        if(result & FOUND_FULL_SOLUTION){
            for(uint j = this->main_search_full_solutions_printed; j < this->main_search.full_solutions.size(); j++){

                this->full_solution_copies.push_back(SolutionCopy(this->main_search,FULL_SOLUTION,j));
                this->fs_indices.push_back(fs_indices.size());
                //AddOrReplaceSolution(main_search, full_solution_copies, FULL_SOLUTION, full_solution_copies_free_slots, fs_indices, j);

                this->ShowNewFullSolutions();
                this->PrintSolution(FULL_SOLUTION,this->main_search,j);
                this->main_search_full_solutions_printed++;
            }
        }
        if(this->main_search.AdvanceActionAmounts() & SEARCH_DONE){
            this->FinishMainSearch();
            return SEARCH_DONE;
        }

    }
    this->ui->mainSearchProgressLabel->setText(ProgressLabelString(this->main_search.iterations_done,this->main_search.iterations_total));
    float progress_percent = ((float)this->main_search.iterations_done / (float)this->main_search.iterations_total) * 100;
    this->ui->progressBarMainSearch->setValue(progress_percent);

    QEvent* ev = new QEvent(MainSearchType);
    qApp->postEvent(this, ev);

    return SUCCESS;
}

void Posefi::AddOrReplaceSolution(Search& search, QList<SolutionCopy>& copies, int solutionType, QList<uint>& freeSlots, QList<int>& solutionCopyIndices, int solutionIndex)
{
    if(freeSlots.empty())
    {
        copies.push_back(SolutionCopy(search, solutionType, solutionIndex));
        int copyIdx = solutionCopyIndices.size();
        solutionCopyIndices.push_back(copyIdx);
    }
    else
    {
        int copyIdx = freeSlots.back();
        SolutionCopy& solCopy = copies[copyIdx];
        solCopy.Replace(search, solutionType, solutionIndex);
        solutionCopyIndices.push_back(copyIdx);
        freeSlots.resize(freeSlots.size() - 1);
    }
}


SolutionCopy::SolutionCopy(Search& search, int solution_type, uint solution_index)
{
    CopyData(search, solution_type, solution_index);
}

SolutionCopy::SolutionCopy(const SolutionCopy* other)
{
    CopyData(other);
}

void SolutionCopy::CopyData(const SolutionCopy* other)
{
    searched_name = other->searched_name;
    searched_pos1 = other->searched_pos1;
    searched_pos2 = other->searched_pos2;
    default_pos1 = other->default_pos1;
    default_pos2 = other->default_pos2;
    found_pos1 = other->found_pos1;
    found_pos2 = other->found_pos2;
    found_first = other->found_first;
    found_second = other->found_second;
    costs = other->costs;
    action_names = other->action_names;
    action_angles = other->action_angles;
    action_amounts = other->action_amounts;
    xyz_first_string = other->xyz_first_string;
    xyz_second_string = other->xyz_second_string;
}

void SolutionCopy::Replace(Search& search, int solution_type, uint solution_index)
{
    action_names.clear();
    action_angles.clear();
    action_amounts.clear();
    CopyData(search, solution_type, solution_index);
}

void SolutionCopy::CopyData(Search& search, int solution_type, uint solution_index)
{
    Solution* sol;
    if(solution_type == PARTIAL_SOLUTION){
        if(solution_index >= search.partial_solutions.size()){
            qInfo() << "error in SolutionCopy::SolutionCopy, solution_index out of range";
            return;
        }
        sol = &(search.partial_solutions[solution_index]);
    }
    else{
        if(solution_index >= search.full_solutions.size()){
            qInfo() << "error in SolutionCopy::SolutionCopy, solution_index out of range";
            return;
        }
        sol = &(search.full_solutions[solution_index]);
    }

    Searched* sea = &(search.searched[sol->searched_id]);

    this->searched_name = sea->name;
    this->searched_pos1 = sea->pos1;
    this->searched_pos2 = sea->pos2;
    this->found_pos1 = sol->pos1;
    this->found_pos2 = sol->pos2;
    this->found_first = sol->found_first;
    this->found_second = sol->found_second;
    this->default_pos1 = search.defaultPos1;
    this->default_pos2 = search.defaultPos2;
    this->costs = sol->costs;

    QString* str_1 = new QString[3] PSE_FIRST_LABELS;
    QString* str_2 = new QString[3] PSE_SECOND_LABELS;
    this->xyz_first_string = str_1[sol->xyz_index];
    this->xyz_second_string = str_2[sol->xyz_index];
    delete[] str_1;
    delete[] str_2;

    QStringList names;
    QList<HWORD> angles;
    QList<uint> amounts;

    for (uint i = 0; i < search.actions.size(); i++)
    {
        int amo = sol->action_amounts[i];
        if(amo == 0)
            continue;
        HWORD ang = search.actions[i].angle;
        QString nam = search.actions[i].name;

        if(amo < 0){
            amo *= -1;
            if(ang >= 0x8000)
                ang -= 0x8000;
            else
                ang += 0x8000;
        }
        names.push_back(nam);
        angles.push_back(ang);
        amounts.push_back((uint)amo);
    }

    QList<int> actionOrder;
    for (uint i = 0; i < angles.size(); i++) actionOrder.append(i);
    std::sort(actionOrder.begin(), actionOrder.end(), [angles] (const int& a, const int& b)
    {
        return angles[a] < angles[b];
    });

    for (int idx : actionOrder)
    {
        action_names.push_back(names[idx]);
        action_angles.push_back(angles[idx]);
        action_amounts.push_back((uint)amounts[idx]);
    }


}

// ----------------------- Edit solutions -----------------------
/*
SolutionsEdit::SolutionsEdit(QWidget* parent, std::vector<SolutionCopy>* solutionCopies, QToolButton* deleteBtn, QToolButton* sortBtn)
    : QObject(parent), m_solutions(solutionCopies), m_deleteBtn(deleteBtn), m_sortBtn(sortBtn)
{

}*/

void Posefi::deleteAllFullSolutions()
{
    deleteAllSolutions(SolutionTypes::FULL_SOLUTION);
}
void Posefi::deleteAllPartialSolutions()
{
    deleteAllSolutions(SolutionTypes::PARTIAL_SOLUTION);
}

void Posefi::deleteAllSolutions(SolutionTypes type)
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search

    if(type == SolutionTypes::FULL_SOLUTION)
    {
        if(FSE_Order.size() > 0)
        {
            int idx = FSE_Order[0];
            while(idx >= 0)
            {
                SolutionElement* se = FullSolutionElements[idx];
                FSE_DisappearToRightAnimation(se);
                current_fs_disable_animations++;
                Disable_FSE(se);
                idx = FSE_Order[0];
            }
            fs_indices.clear();
            full_solution_copies.clear();
            ManageFullSolutionPages();
        }
    }
    else
    {
        if(PSE_Order.size() > 0)
        {
            int idx = PSE_Order[0];
            while(idx >= 0)
            {
                PartialSolutionElement* se = PartialSolutionElements[idx];
                PSE_DisappearToRightAnimation(se);
                current_ps_disable_animations++;
                Disable_PSE(se);
                idx = PSE_Order[0];
            }
            ps_indices.clear();
            partial_solution_copies.clear();
            ManagePartialSolutionPages();
        }
    }

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortPartialSolutionsByCost()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search

    QList<QPair<uint, int>> pairs;
    for(uint i : ps_indices)
    {
        QPair<uint, int> p(i, partial_solution_copies[i].costs);
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
              {
                  return a.second < b.second;
              });
    for(uint i = 0; i < ps_indices.size(); i++)
    {
        ps_indices[i] = pairs[i].first;
    }
    PartialSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortFullSolutionsByCost()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search

    QList<QPair<uint, int>> pairs;
    for(uint i : fs_indices)
    {
        QPair<uint, int> p(i, full_solution_copies[i].costs);
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
    {
        return a.second < b.second;
    });
    for(uint i = 0; i < fs_indices.size(); i++)
    {
        fs_indices[i] = pairs[i].first;
    }
    FullSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortPartialSolutionsByAngleCount()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search


    QList<QPair<uint, int>> pairs;
    for(uint i : ps_indices)
    {
        QList<HWORD> uniqueAngles;
        for(HWORD angle : partial_solution_copies[i].action_angles)
        {
            if(!uniqueAngles.contains(angle))
                uniqueAngles.append(angle);
        }
        QPair<uint, int> p(i, uniqueAngles.size());
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
              {
                  return a.second < b.second;
              });
    for(uint i = 0; i < ps_indices.size(); i++)
    {
        ps_indices[i] = pairs[i].first;
    }
    PartialSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortFullSolutionsByAngleCount()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search


    QList<QPair<uint, int>> pairs;
    for(uint i : fs_indices)
    {
        QList<HWORD> uniqueAngles;
        for(HWORD angle : full_solution_copies[i].action_angles)
        {
            if(!uniqueAngles.contains(angle))
                uniqueAngles.append(angle);
        }
        QPair<uint, int> p(i, uniqueAngles.size());
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
              {
                  return a.second < b.second;
              });
    for(uint i = 0; i < fs_indices.size(); i++)
    {
        fs_indices[i] = pairs[i].first;
    }
    FullSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortPartialSolutionsByActionCount()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search

    QList<QPair<uint, int>> pairs;
    for(uint i : ps_indices)
    {
        QPair<uint, int> p(i, partial_solution_copies[i].action_amounts.size());
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
              {
                  return a.second < b.second;
              });
    for(uint i = 0; i < ps_indices.size(); i++)
    {
        ps_indices[i] = pairs[i].first;
    }
    PartialSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}

void Posefi::sortFullSolutionsByActionCount()
{
    bool mainRunning = main_search_is_running;
    bool subRunning = sub_search_is_running;

    // pause searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search

    QList<QPair<uint, int>> pairs;
    for(uint i : fs_indices)
    {
        QPair<uint, int> p(i, full_solution_copies[i].action_amounts.size());
        pairs.append(p);
    }
    std::sort(pairs.begin(), pairs.end(), [](const QPair<uint, int>& a, const QPair<uint, int>& b)
              {
                  return a.second < b.second;
              });
    for(uint i = 0; i < fs_indices.size(); i++)
    {
        fs_indices[i] = pairs[i].first;
    }
    FullSolutionChangePage(0);

    // continue searches
    if(mainRunning)
        PauseButton_press();
    //if(subRunning)
    // TODO pause sub Search
}


// ------------- Sub Search Struct ----------------------------------------
Posefi::SubSearchWidget::SubSearchWidget(QWidget* parent)
{
    defaultPosLabel = new QLabel(parent);
    defaultCoordinateLabel = new QLabel(parent);
    defaultPosEdit = new AutoSelectLineEdit(parent);
    defaultPosEdit->setAlignment(Qt::AlignCenter);
    startBtn = new QPushButton(parent);;
    pauseBtn = new QPushButton(parent);;
    stopBtn = new QPushButton(parent);;
    updateActionsBtn = new QPushButton(parent);;
    progressBar = new QProgressBar(parent);;
    wantedTable = new QTableWidget(parent);;
    actionTable = new QTableView(parent);;

    defaultPosLabel->setGeometry(340,10,111,16);
    defaultPosLabel->setText("Default Position");
    defaultCoordinateLabel->setGeometry(320,32,16,16);
    defaultPosEdit->setGeometry(420,32,65,23);

    //defaultCoordinateLabel->setText("x:");              //

    SolutionCopy testSol;
    testSol.searched_name = "searched name";
    testSol.searched_pos1 = 0x41234567;
    testSol.searched_pos2 = 0x41234567;
    testSol.default_pos1 = 0x41234567;
    testSol.default_pos2 = 0x41234567;
    testSol.found_pos1 = 0x41234567;
    testSol.found_pos2 = 0x41234567;
    testSol.found_first = true;
    testSol.found_second = false;
    testSol.costs = 69;
    testSol.action_names = {"action1", "action2", "action3"};
    testSol.action_angles = {0x4000, 0x8000, 0xC000};
    testSol.action_amounts = {9, 6 , 3};
    testSol.xyz_first_string = "x:";
    testSol.xyz_second_string = "z:";

    setPartialSolution(&testSol);

}

void Posefi::SubSearchWidget::setPartialSolution(const SolutionCopy* sol)
{
    if (!partialSolutionCopy)
        partialSolutionCopy = new SolutionCopy(sol);
    else
        partialSolutionCopy->CopyData(sol);

    if(partialSolutionCopy->found_first)
    {
        defaultCoordinateLabel->setText(partialSolutionCopy->xyz_second_string);
        defaultPosEdit->setText(QString::number(partialSolutionCopy->found_pos2, 16));
    }

    else
    {
        defaultCoordinateLabel->setText(partialSolutionCopy->xyz_first_string);
        defaultPosEdit->setText(QString::number(partialSolutionCopy->found_pos1, 16));
    }
}


// ------------- Converter functions ----------------------------------------
// --- (I saw too late that Qt has tools to check and adjust invalid string, but this works too)

bool HexCheck(QString str){
    QString compare = "1234567890AaBbCcDdEeFf";
    for (UINT i = 0; i < str.size(); i++){
        bool ok = false;
        for (UINT j = 0; j < compare.size(); j++){
            if(str[i] == compare[j]){
                ok = true;
                continue;
            }
        }
        if(!ok)
            return false;
    }
    return true;
}

bool DecCheck(QString str, bool signed_){
    if(str == "+" || str == "-")
        return false;
    QString compare = (signed_) ? "+-1234567890" : "+1234567890";
    for (UINT i = 0; i < str.size(); i++){
        bool ok = false;
        UINT start;
        if (signed_)
            start = (i == 0) ? 0 : 2;
        else
            start = (i == 0) ? 0 : 1;
        for (UINT j = start; j < compare.size(); j++){
            if(str[i] == compare[j]){
                ok = true;
                continue;
            }
        }
        if(!ok)
            return false;
    }
    return true;
}

QString AdjustHexString(QString in, UINT length, QString filler){
    int str_size = in.size();
    QString out = "";
    if (str_size > length){
        out = in;
        out.remove(0, str_size - length);
    }
    else if (str_size < length){
        for (int i = 0; i < (length - str_size); i++){
            out += filler;
        }
        out += in;
    }
    else
        out = in;
    return out;
}

QString ProgressLabelString(uint64_t current, uint64_t total){
    int divisor = total > PROGRESS_LABEL_DIVISOR ? PROGRESS_LABEL_DIVISOR : 1;
    int decimals = total > PROGRESS_LABEL_DIVISOR ? PROGRESS_LABEL_DECIMALS : 0;
    QString suffix = total > PROGRESS_LABEL_DIVISOR ? PROGRESS_LABEL_SUFFIX : "";


    QString current_str = QString::number((float)current/(float)divisor,'f',decimals);
    QString total_str = QString::number((float)total/(float)divisor,'f',decimals);
    return current_str + suffix + " / " + total_str + suffix;
}


// --- Debug functions ----------------------------------



int Posefi::PrintMainSearch(){
    qInfo() << "------------------------------------\n";
    qInfo() << "Main Search:\n";
    return this->PrintSearch(this->main_search);
}

int Posefi::PrintSubSearch(){
    qInfo() << "Sub Search:\n";
    return this->PrintSearch(this->sub_search);
}

int Posefi::PrintSearch(Search& s){
    bool first_searched = s.search_first;
    bool second_searched = s.search_second;
    qInfo() << "search first: " << first_searched << " search second: " << second_searched << "\n";
    QString xy;
    QString yz;

    if (first_searched){
        xy = (this->xyzLable_radioButtons.checkedId() == 2) ? "Y" : "X";
        qInfo() << xy << "-POSITION DATA:";
        qInfo() << "smallest exponent: " << QString::number(s.smallestExponent1,16);
        qInfo() << "resettable:        " << s.first_resettable << "\n";

        qInfo() << "default position:";
        qInfo() << "WORD:            " << QString::number(s.defaultPos1,16);
        qInfo() << "mantissa:        " << QString::number(s.defaultManti1,16);
        qInfo() << "exponent:        " << QString::number(s.defaultExponent1,16);
        qInfo() << "sign:            " << s.defaultSign1;
        qInfo() << "shift:           " << QString::number(s.defaultShift1,16);
        qInfo() << "shifted value:   " << QString::number(s.shiftedDefaultPos1,16) << "\n";
    }

    if (second_searched){
        yz = (this->xyzLable_radioButtons.checkedId() == 0) ? "Y" : "Z";
        qInfo() << yz << "-POSITION DATA:";
        qInfo() << "smallest exponent: " << QString::number(s.smallestExponent2,16);
        qInfo() << "resettable:        " << s.second_resettable << "\n";

        qInfo() << "default position:";
        qInfo() << "WORD:            " << QString::number(s.defaultPos2,16);
        qInfo() << "mantissa:        " << QString::number(s.defaultManti2,16);
        qInfo() << "exponent:        " << QString::number(s.defaultExponent2,16);
        qInfo() << "sign:            " << s.defaultSign2;
        qInfo() << "shift:           " << QString::number(s.defaultShift2,16);
        qInfo() << "shifted value:   " << QString::number(s.shiftedDefaultPos2,16) << "\n";
    }

    qInfo() << "SEARCHED:";
    for(uint i = 0; i < s.searched.size(); i++){
        QString str = s.searched[i].name + " | ";
        if(first_searched)
            str += xy + "-pos=" + QString::number(s.searched[i].pos1,16) + " | ";
        if(second_searched)
            str += yz + "-pos=" + QString::number(s.searched[i].pos2,16) + " | ";
        if(first_searched)
            str += xy + "-tol=" + QString::number(s.searched[i].tol1,10) + " | ";
        if(second_searched)
            str += yz + "-tol=" + QString::number(s.searched[i].tol2,10) + " | ";
        if(first_searched)
            str += xy + "-and=" + QString::number(s.searched[i].and1,16) + " | ";
        if(second_searched)
            str += yz + "-and=" + QString::number(s.searched[i].and2,16) + " | ";
        str += "max-cost=" + QString::number(s.searched[i].max_cost,10);
        qInfo() << str;
    }

    qInfo() << "\nACTIONS:\n";
    for(uint i = 0; i < s.actions.size(); i++){
        QString str = s.actions[i].name + " | ";
        if(first_searched && s.actions[i].pos1 != nullptr)
            str += QString::number(s.actions[i].pos1->pos,16) + " | ";
        if(second_searched && s.actions[i].pos2 != nullptr)
            str += QString::number(s.actions[i].pos2->pos,16) + " | ";
        str += QString::number(s.actions[i].angle,16) + " | ";
        str += QString::number(s.actions[i].min,10) + " | ";
        str += QString::number(s.actions[i].max,10) + " | ";
        str += QString::number(s.actions[i].cost,10) + " | ";
        qInfo() << str;

        if(first_searched && s.actions[i].pos1 != nullptr){
            str = "about " + xy +": ";
            str += "manti=" + QString::number(s.actions[i].pos1->manti,16) + " | ";
            str += "exp=" + QString::number(s.actions[i].pos1->exp,16) + " | ";
            str += "sign=" + QString::number(s.actions[i].pos1->sign) + " | ";
            str += "shift=" + QString::number(s.actions[i].pos1->shift,10) + " | ";
            str += "move=" + QString::number(s.actions[i].pos1->move,16);
            qInfo() << str;
        }
        if(second_searched && s.actions[i].pos2 != nullptr){
            str = "about " + yz +": ";
            str += "manti=" + QString::number(s.actions[i].pos2->manti,16) + " | ";
            str += "exp=" + QString::number(s.actions[i].pos2->exp,16) + " | ";
            str += "sign=" + QString::number(s.actions[i].pos2->sign) + " | ";
            str += "shift=" + QString::number(s.actions[i].pos2->shift,10) + " | ";
            str += "move=" + QString::number(s.actions[i].pos2->move,16);
            qInfo() << str;

        }

        str = "amount total: " + QString::number(s.actions[i].amounts_total,10) + ", amount order: ";
        if(s.actions[i].amounts_order != nullptr){
            for (int j = 0; j < s.actions[i].amounts_total; j++){
                str += QString::number(s.actions[i].amounts_order[j],10);
                if (j < s.actions[i].amounts_total-1)
                    str += " | ";
            }
        }
        else
            str += "nullptr";
        qInfo() << str;


        str = "costs: ";
        if(s.actions[i].costs != nullptr){
            for (int j = 0; j < s.actions[i].amounts_total; j++){
                str += QString::number(s.actions[i].costs[j],10);
                if (j < s.actions[i].amounts_total-1)
                    str += " | ";
            }
        }
        else
            str += "nullptr";
        qInfo() << str;

        if(first_searched){
            str = "moves " + xy + ": ";
            if(s.actions[i].moves_1st != nullptr){
                for (int j = 0; j < s.actions[i].amounts_total; j++){
                    str += QString::number(s.actions[i].moves_1st[j],16);
                    if (j < s.actions[i].amounts_total-1)
                        str += " | ";
                }
            }
            else
                str += "nullptr";
            qInfo() << str;
        }

        if(second_searched){
            str = "moves " + yz + ": ";
            if(s.actions[i].moves_2nd != nullptr){
                for (int j = 0; j < s.actions[i].amounts_total; j++){
                    str += QString::number(s.actions[i].moves_2nd[j],16);
                    if (j < s.actions[i].amounts_total-1)
                        str += " | ";
                }
            }
            else
                str += "nullptr";
            qInfo() << str;
        }
        qInfo() << "";
    }

    qInfo() << "\n---------------------------";
    return SUCCESS;
}

int Posefi::PrintSolution(int solution_type, Search& search, uint index){
    Solution* solution;
    Searched* searched;
    if(solution_type == PARTIAL_SOLUTION){
        qInfo() << "\nPARTIAL COLUTION FOUND:";
        solution = &search.partial_solutions[index];
        searched = &search.searched[solution->searched_id];
    }
    else{
        qInfo() << "\nFULL COLUTION FOUND:";
        solution = &search.full_solutions[index];
        searched = &search.searched[solution->searched_id];
    }

    qInfo() << "searched name: " << searched->name;
    qInfo() << "searched pos:  " << QString::number(searched->pos1,16) << ", " << QString::number(searched->pos2,16);
    qInfo() << "found pos:     1st: " << solution->found_first << ", 2nd: " << solution->found_second;
    qInfo() << "costs:         " << solution->costs;
    qInfo() << "actions:";
    for(uint i = 0; i < search.actions.size(); i++){
        int amount = solution->action_amounts[i];
        if(!amount)
            continue;
        Action* action = &search.actions[i];
        uint16_t angle = action->angle;
        if(amount < 0){
            amount *= -1;
            if(angle < 0x8000)
                angle += 0x8000;
            else
                angle -= 0x8000;
        }
        qInfo() << "  - " << action->name << ": angle =" << QString::number(angle,16) << " | amount =" << amount;
    }
    qInfo() << "";
    return SUCCESS;
}

// ---------------------------------------------
// --- Sub classes -----------------------------


AutoSelectLineEdit::AutoSelectLineEdit(QWidget* parent): QLineEdit(parent)
{
    setMaxLength(8);
    setAlignment(Qt::AlignLeft);
}

AutoSelectLineEdit::~AutoSelectLineEdit(){}

void AutoSelectLineEdit::focusInEvent(QFocusEvent *e){
    // First let the base class process the event
    QLineEdit::focusInEvent(e);
    // Then select the text by a single shot timer, so that everything will
    // be processed before (calling selectAll() directly won't work)
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
}
