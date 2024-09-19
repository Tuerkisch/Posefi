#ifndef FINDER_H
#define FINDER_H

// BUGS:
// issue with resettable cases

#include "Constants.h"
//#include <QString>

// A 32-bit-float uses the lower 23 bits for the mantissa, but it is always treated as if it had the 24th bit set as well.
// (Zero is the exception)
#define MANTISSA(val)           ((val & 0x007FFFFF) | 0x00800000)
#define SIGN(val)               ((val >> 31) == 0)  // Highest bit is sign (set bit means negative)
#define EXPONENT(val)           (val & 0x7F800000)  // keep exponent not shifted to make the conversion after calculation faster
#define SHIFT(exp,smallest_exp) (((exp & 0x7F800000) - (smallest_exp & 0x7F800000)) >> 23)

// In the context of the calculation, shift the calculated sum left or right, until it is in the valid range.
#define MANTISSA_MIN    0x00800000  //
#define MANTISSA_MAX    0x00FFFFFF  //
#define EXPONENT_UNIT   0x00800000  // If exponent value is not shifted right, this is equivalent to one unit


typedef uint32_t WORD;
typedef uint16_t HWORD;
typedef int64_t DWORD;

struct Action;
struct Coordinate;
struct ActionCoordinate;
struct Solution;


struct Action{
    Action(QString name, WORD first_pos, WORD second_pos, HWORD angle, int min, int max, int cost, QString* debug = nullptr);
    ~Action();
    QString name;
    HWORD angle;
    int min, max;
    int cost;

    bool first_used, second_used;
    ActionCoordinate* pos1;
    ActionCoordinate* pos2;

    int amounts_total;
    int amount_index;   // used during the calculation as index for amount_order and move_multiplied
    int* amounts_order = nullptr;
    int* costs = nullptr;
    DWORD* moves_1st = nullptr;
    DWORD* moves_2nd = nullptr;
};

struct Coordinate{
    Coordinate(WORD pos):
        pos(pos){
        manti = MANTISSA(pos);
        exp = EXPONENT(pos);
        sign = SIGN(pos);
    }
    WORD pos;   // raw coordinate data
    WORD manti; // mantissa before shift
    WORD exp;   // exponent
    bool sign;  // positive or negative
    int shift;  // the mantissa will be shifted left by this amount
};

struct ActionCoordinate : Coordinate {
    ActionCoordinate(WORD pos) : Coordinate(pos){

    }
    DWORD move; // mantissa shifted according to the exponent minus the shifted mantissa of the default position
};

struct Searched{
    Searched(QString name, WORD first_pos, WORD second_pos, int tolerance_first, int tolerance_second,
             WORD and_first, WORD and_second, int max_cost):
             name(name), pos1(first_pos), pos2(second_pos), and1(and_first), and2(and_second), max_cost(max_cost){
        if (pos1 != NULL_VALUE){
            this->compare1 = pos1 & and1;
            this->tol1 = (tolerance_first >= 0) ? tolerance_first : -tolerance_first;
        }
        if (pos2 != NULL_VALUE){
            this->compare2 = pos2 & and2;
            this->tol2 = (tolerance_second >= 0) ? tolerance_second : -tolerance_second;
        }
        //max_costs_initialized = false;
        // If max_cost > 0, then max_costs_currently will always stay the same.
        // Otherwise max_costs_currently will allow any cost for the first found solution (technically the max is 0xFFFF),
        // and from there on it will be adjusted by UpdateCosts.
        max_costs_currently = max_cost > 0 ? max_cost : 0xFFFF;
    }
    void UpdateCosts(int new_costs);

    QString name;
    WORD pos1, pos2;    // raw position (probably not even necessary to store this, but whatever)
    int tol1, tol2;     // tolerance
    WORD and1, and2;    // values for the &-operator
    WORD compare1, compare2; // the position data used during the calculation

    // if max_cost is negative or zero, keep track of what the lowest costs so far have been
    // and only allow (costs < lowest_costs - max_cost)
    int max_cost;
    int max_costs_currently;
};



class Search{
public:
    Search(WORD default_first, WORD default_second, bool search_first, bool search_second, bool first_resettable,
           bool second_resettable, uint xyz_index):
           search_first(search_first), search_second(search_second), first_resettable(first_resettable),
           second_resettable(second_resettable), defaultPos1(default_first), defaultPos2(default_second){
        defaultManti1 = MANTISSA(default_first);
        defaultManti2 = MANTISSA(default_second);
        defaultExponent1 = EXPONENT(default_first);
        defaultExponent2 = EXPONENT(default_second);
        defaultSign1 = SIGN(default_first);
        defaultSign2 = SIGN(default_second);
        this->xyz_index = (xyz_index <= 2) ? xyz_index : 1;
    }
    Search(){}

    int SetDefaultPosition(WORD default_first, WORD default_second, bool search_first, bool search_second,
                           bool first_resettable, bool second_resettable, uint xyz_index);
    int AddAction(QString name, WORD first_pos, WORD second_pos, HWORD angle, int min, int max, int cost, QString* debug = nullptr);
    int AddSearched(QString name, WORD first_pos, WORD second_pos, int tolerance_first,
                    int tolerance_second, WORD and_first, WORD and_second, int max_cost);
    int ConvertPositionData(QString* debug = nullptr);
    int ClearData();
    int Iteration(QString* output = nullptr);
    void AddFullSolution(uint searched_id, int costs, bool found_first, bool found_second, WORD pos1, WORD pos2, uint xyz_index);
    void AddPartialSolution(uint searched_id, int costs, bool found_first, bool found_second, WORD pos1, WORD pos2, uint xyz_index);
    int AdvanceActionAmounts();

    uint xyz_index; // index for the labels xy / xz / yz

    uint smallestExponent1, smallestExponent2;
    bool search_first, search_second;
    bool first_resettable, second_resettable;
    std::vector<Action> actions;
    std::vector<Searched> searched;

    // default position data
    WORD defaultPos1, defaultPos2;
    WORD defaultManti1, defaultManti2;
    WORD defaultExponent1, defaultExponent2;
    bool defaultSign1, defaultSign2;
    int defaultShift1, defaultShift2;
    DWORD shiftedDefaultPos1, shiftedDefaultPos2;

    // even if both positions are searched, if there are not both positions resettable,
    // it is more efficient to ignore one position at first
    bool add_to_1st, add_to_2nd;

    // create arrays with already multiplied move nad cost values to speed up the calculation (I think?)
    DWORD** moves_1st = nullptr;
    DWORD** moves_2nd = nullptr;
    int** costs;

    // info for user interface
    uint64_t iterations_total;
    uint64_t iterations_done;

    // solution data
    std::vector<Solution> partial_solutions;
    std::vector<Solution> full_solutions;

};

struct Solution{

    uint searched_id;
    int costs;
    bool found_first;
    bool found_second;
    WORD pos1;
    WORD pos2;
    std::vector<uint> action_amounts;
    uint xyz_index;
    Solution(uint searched_id, int costs, bool found_first, bool found_second, WORD pos1, WORD pos2, uint xyz_index):
            searched_id(searched_id), costs(costs), found_first(found_first),
            found_second(found_second), pos1(pos1), pos2(pos2), xyz_index(xyz_index) {}
};



#endif // FINDER_H
