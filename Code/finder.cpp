#include "finder.h"
#include <QDebug>

Action::Action(QString name, WORD first_pos, WORD second_pos, HWORD angle, int min, int max, int cost, QString* debug){
    this->name = name;
    this->angle = angle;
    if (min <= max){
        this->min = min;
        this->max = max;
    }
    else {
        this->max = min;
        this->min = max;
    }
    this->amounts_total = this->max - this->min + 1;
    this->amounts_order = new int[this->amounts_total];

//#define NAIVE_ORDER
#ifndef NAIVE_ORDER
    // sort the order by taking the absolute values of the amounts and putting them in order low to high
    // this way the algorithm starts with the lowest amounts
    if(this->min >= 0 && this->max >= 0){
        int val = this->min;
        for (int i = 0; i < this->amounts_total; i++){
            this->amounts_order[i] = val;
            val++;
        }
    }
    else if (this->min < 0 && this->max <= 0){
        int val = this->max;
        for (int i = 0; i < this->amounts_total; i++){
            this->amounts_order[i] = val;
            val--;
        }
    }
    else{
        this->amounts_order[0] = 0;
        int x = (this->max >= -this->min) ? this->max : -this->min;
        int index = 1;
        for(int i = 1; i <= x; i++){
            if (this->max >= i){
                this->amounts_order[index] = i;
                index++;
            }
            if (-this->min >= i){
                this->amounts_order[index] = -i;
                index++;
            }
        }
    }
#else
    //Naive order
    int j = 0;
    for (int i = this->min; i<= this->max; i++){
        this->amount_order[j] = i;
        j++;
    }
#endif
    if (debug != nullptr){
        *debug = "amount order: ";
        for(int i = 0; i < this->amounts_total; i++){
            *debug += QString::number(this->amounts_order[i], 10);
            if (i != this->amounts_total-1)
                *debug += ", ";
        }
    }

    if(first_pos != NULL_VALUE){
        pos1 = new ActionCoordinate(first_pos);
        first_used = true;
    }
    else
        first_used = false;
    if(second_pos != NULL_VALUE){
        pos2 = new ActionCoordinate(second_pos);
        second_used = true;
    }
    else
        second_used = false;

    // apply cost and calculate the costs equivalent to the amounts
    this->cost = cost;
    this->costs = new int[this->amounts_total];
    for(int i = 0; i < this->amounts_total; i++){
        int a = this->amounts_order[i];
        if(a < 0)
            a *= -1;
        costs[i] = a * cost;
    }
}

// Apparently Qt handles deallocations automatically. Doing it manually actually crashes o_O
Action::~Action(){
    //if (this->amount_order != nullptr)
        //delete[] this->amount_order;
}

void Searched::UpdateCosts(int new_costs){
    if(this->max_cost > 0)
        return;
    int c = new_costs - this->max_cost;
    if(this->max_costs_currently > c)
        this->max_costs_currently = c;
}

int Search::SetDefaultPosition(WORD default_first, WORD default_second, bool search_first, bool search_second,
                               bool first_resettable, bool second_resettable, uint xyz_index){

    defaultPos1 = default_first;
    defaultPos2 = default_second;
    defaultManti1 = MANTISSA(default_first);
    defaultManti2 = MANTISSA(default_second);
    defaultExponent1 = EXPONENT(default_first);
    defaultExponent2 = EXPONENT(default_second);
    defaultSign1 = SIGN(default_first);
    defaultSign2 = SIGN(default_second);
    this->search_first = search_first;
    this->search_second = search_second;
    this->first_resettable = first_resettable;
    this->second_resettable = second_resettable;
    this->xyz_index = (xyz_index <= 2) ? xyz_index : 1;

    // decide which to calculate first, if not both:
    // case: no position searched -> BUGGED
    if (!this->search_first && !this->search_second)
        return NO_POSITION_SEARCHED;

    // case: only one is searched -> choose the searched one of course
    if (!(this->search_first && this->search_second)){
        add_to_1st = this->search_first;
        add_to_2nd = this->search_second;
    }
    // case: both searched but neither resettable -> just choose one
    else if (!this->first_resettable && !this->second_resettable){
        add_to_1st = true;
        add_to_2nd = false;
    }
    // case: both searched and at least one resettable -> always calculate a position if its position is resettable
    // (in case both are resettable, an iteration will take twice the time, but at the same time there are twice as many chances for a hit)
    else {
        add_to_1st = this->first_resettable;
        add_to_2nd = this->second_resettable;
    }

    return SUCCESS;
}

int Search::AddAction(QString name, WORD first_pos, WORD second_pos, HWORD angle, int min, int max, int cost, QString* debug){
    if (debug == nullptr)
        this->actions.push_back(Action(name,first_pos, second_pos, angle, min, max, cost));
    else
        this->actions.push_back(Action(name,first_pos, second_pos, angle, min, max, cost, debug));
    return SUCCESS;
}

int Search::AddSearched(QString name, WORD first_pos, WORD second_pos, int tolerance_first,
                        int tolerance_second, WORD and_first, WORD and_second, int max_cost){
    Searched s(name, first_pos, second_pos, tolerance_first, tolerance_second, and_first, and_second, max_cost);
    this->searched.push_back(s);
    return SUCCESS;
}

int Search::ClearData(){
    // Since this function is called outside of a Qt-object-destroy context, delete these manually I guess?
    for (uint i = 0; i < this->actions.size(); i++){
        if(this->actions[i].amounts_order != nullptr){
            delete[] this->actions[i].amounts_order;
            this->actions[i].amounts_order = nullptr;
        }
        if(this->actions[i].costs != nullptr){
            delete[] this->actions[i].costs;
            this->actions[i].costs = nullptr;
        }
        if(this->actions[i].moves_1st != nullptr){
            delete[] this->actions[i].moves_1st;
            this->actions[i].moves_1st = nullptr;
        }
        if(this->actions[i].moves_2nd != nullptr){
            delete[] this->actions[i].moves_2nd;
            this->actions[i].moves_2nd = nullptr;
        }
    }
    this->actions.clear();
    this->searched.clear();
    this->full_solutions.clear();
    this->partial_solutions.clear();
    return SUCCESS;
}

// Call this after all actions have been gathered.
// This sets the smallest exponens, and the shift-val and intManti-val of the action objects.
int Search::ConvertPositionData(QString* debug){

    if (this->actions.empty()){
        if (debug != nullptr)
            *debug = "ARRAY_SIZE_ZERO error in Search::ConvertPositionData";
        return ARRAY_SIZE_ZERO;
    }

    int return_val = SUCCESS;

    // set the exponent of the default positions as the default for smallest exponent
    if (this->search_first)
        this->smallestExponent1 = this->defaultExponent1;
    if (this->search_second)
        this->smallestExponent2 = this->defaultExponent2;

    // determine the smallest exponent
    for (uint i = 0; i < this->actions.size(); i++){
        if (this->search_first && (this->actions[i].pos1->exp < this->smallestExponent1 || !this->smallestExponent1) && this->actions[i].pos1->exp){
            this->smallestExponent1 = this->actions[i].pos1->exp;
        }
        if (this->search_second && (this->actions[i].pos2->exp < this->smallestExponent2 || !this->smallestExponent2) && this->actions[i].pos2->exp){
            this->smallestExponent2 = this->actions[i].pos2->exp;
        }
    }

    // Get shift values, shift mantissas and set the move-values (difference of action position and default position).
    // In case the difference between smallest and highest exponent is big, there is the possibility that the
    // shift value ends up being too high. Too high means that the shiftedDefaultPos-value overflows. If this application
    // is used in the Zelda64 context it was designed for, I don't expect this to ever happen. I implemented a check but
    // otherwise this exception is not handled.
    if(this->search_first){
        if (this->defaultExponent1){
            this->defaultShift1 = SHIFT(this->defaultExponent1,this->smallestExponent1);
            this->shiftedDefaultPos1 = this->defaultManti1 << this->defaultShift1;
            WORD overflow_check = this->shiftedDefaultPos1 >> this->defaultShift1;
            if(overflow_check != this->defaultManti1){
                if (debug != nullptr)
                    *debug += "INT_OVERFLOW error in Search::ConvertPositionData, default data of first\n";
                return_val |= INT_OVERFLOW;
            }
            if (!defaultSign1)
                this->shiftedDefaultPos1 *= -1;
        }
        else{
            this->defaultShift1 = 0;
            this->shiftedDefaultPos1 = 0;
        }
    }
    if(this->search_second){
        if (this->defaultExponent2){
            this->defaultShift2 = SHIFT(this->defaultExponent2,this->smallestExponent2);
            this->shiftedDefaultPos2 = this->defaultManti2 << this->defaultShift2;
            WORD overflow_check = this->shiftedDefaultPos2 >> this->defaultShift2;
            if(overflow_check != this->defaultManti2){
                if (debug != nullptr)
                    *debug += "INT_OVERFLOW error in Search::ConvertPositionData, default data of second\n";
                return_val |= INT_OVERFLOW;
            }
            if (!defaultSign2)
                this->shiftedDefaultPos2 *= -1;
        }
        else {
            this->defaultShift2 = 0;
            this->shiftedDefaultPos2 = 0;
        }
    }
    for (uint i = 0; i < this->actions.size(); i++){
        if (this->search_first){
            if (this->actions[i].pos1->exp){
                this->actions[i].pos1->shift = SHIFT(this->actions[i].pos1->exp,this->smallestExponent1);
                this->actions[i].pos1->move = this->actions[i].pos1->manti << this->actions[i].pos1->shift;
                WORD overflow_check = this->actions[i].pos1->move >> this->actions[i].pos1->shift;
                if(overflow_check != this->actions[i].pos1->manti){
                    if (debug != nullptr)
                        *debug += "INT_OVERFLOW error in Search::ConvertPositionData, data of action " + this->actions[i].name + ", pos1\n";
                    return_val |= INT_OVERFLOW;
                }
                if (!this->actions[i].pos1->sign)
                    this->actions[i].pos1->move *= -1;
                this->actions[i].pos1->move -= this->shiftedDefaultPos1;
            }
            else {
                this->actions[i].pos1->shift = this->defaultShift1;
                this->actions[i].pos1->move = this->shiftedDefaultPos1 * -1;
            }
        }
        if (this->search_second){
            if (this->actions[i].pos2->exp){
                this->actions[i].pos2->shift = SHIFT(this->actions[i].pos2->exp,this->smallestExponent2);
                this->actions[i].pos2->move = this->actions[i].pos2->manti << this->actions[i].pos2->shift;
                WORD overflow_check = this->actions[i].pos2->move >> this->actions[i].pos2->shift;
                if(overflow_check != this->actions[i].pos2->manti){
                    if (debug != nullptr)
                        *debug += "INT_OVERFLOW error in Search::ConvertPositionData, data of action " + this->actions[i].name + ", pos2\n";
                    return_val |= INT_OVERFLOW;
                }
                if (!this->actions[i].pos2->sign)
                    this->actions[i].pos2->move *= -1;
                this->actions[i].pos2->move -= this->shiftedDefaultPos2;
            }
            else {
                this->actions[i].pos2->shift = this->defaultShift2;
                this->actions[i].pos2->move = this->shiftedDefaultPos2 * -1;
            }
        }
    }

    // create arrays with already multiplied move values, according to the amounts
    if(this->search_first){
        for(uint i = 0; i < this->actions.size(); i++){
            this->actions[i].moves_1st = new DWORD[this->actions[i].amounts_total];
            if(this->actions[i].amounts_order == nullptr){
                if (debug != nullptr)
                    *debug += "NULL_POINTER error in Search::ConvertPositionData, amounts_order of action" + QString::number(i) + " = nullptr\n";
                return_val |= NULL_POINTER;
            }
            for(int j = 0; j < this->actions[i].amounts_total; j++){
                this->actions[i].moves_1st[j] = this->actions[i].pos1->move * this->actions[i].amounts_order[j];
            }
        }
    }
    if(this->search_second){
        for(uint i = 0; i < this->actions.size(); i++){
            this->actions[i].moves_2nd = new DWORD[this->actions[i].amounts_total];
            if(this->actions[i].amounts_order == nullptr){
                if (debug != nullptr)
                    *debug += "NULL_POINTER error in Search::ConvertPositionData, amounts_order of action" + QString::number(i) + " = nullptr\n";
                return_val |= NULL_POINTER;
            }
            for(int j = 0; j < this->actions[i].amounts_total; j++){
                this->actions[i].moves_2nd[j] = this->actions[i].pos2->move * this->actions[i].amounts_order[j];
            }
        }
    }

    // calculate total iteration count, set variables which track iterations to 0
    this->iterations_done = 0;
    this->iterations_total = 1;
    for (uint i = 0; i < this->actions.size(); i++){
        this->iterations_total *= this->actions[i].amounts_total;
        this->actions[i].amount_index = 0;
    }
    qInfo() << iterations_total;

    return return_val;
}

int Search::Iteration(QString* output){
    // since this is called frequently, no extra debug checks

    if(iterations_done == 0) // skip first iteration because action amounts are all 0
    {
        iterations_done++;
        return SUCCESS;
    }

    int return_val = SUCCESS;

    int costs = 0;
    DWORD sum_1st, sum_2nd;
    WORD new_pos1, new_pos2;
    WORD new_exp1, new_exp2;
    std::vector<int> hits1, hits2;
    std::vector<uint> costs_ok;

    bool calculate_first_order = this->add_to_1st;
    bool calculate_second_order = this->add_to_2nd;
    bool compare_first_order = false;
    bool compare_second_order = false;
    bool calculated_first = false;
    bool calculated_second = false;

    // calculate costs of this iteration
    for(uint i = 0; i < this->actions.size(); i++){
        costs += this->actions[i].costs[this->actions[i].amount_index];
    }
    // check all searched max costs and add searched to the costs_ok array
    for(uint i = 0; i < this->searched.size(); i++){
        int i_max_cost = this->searched[i].max_costs_currently;
        if(costs <= i_max_cost){
            costs_ok.push_back(i);
        }
    }
    // return if costs are too high for any searched
    if(costs_ok.empty()){
        return_val = COSTS_TOO_HIGH;
        goto done;
    }

    if(!calculate_first_order)
        goto calculate_second;

    calculate_first:
    sum_1st = shiftedDefaultPos1;
    new_pos1 = 0;
    new_exp1 = smallestExponent1;
    for(uint i = 0; i < this->actions.size(); i++){
        sum_1st += this->actions[i].moves_1st[this->actions[i].amount_index];
    }
    if (sum_1st){
        if(sum_1st < 0){
            new_pos1 |= 0x80000000;
            sum_1st *= -1;
        }
        while(sum_1st > MANTISSA_MAX){
            sum_1st >>= 1;
            new_exp1 += EXPONENT_UNIT;
        }
        while(sum_1st < MANTISSA_MIN){
            sum_1st <<= 1;
            new_exp1 -= EXPONENT_UNIT;
        }
        sum_1st &= ~0x00800000;
        new_pos1 |= sum_1st | new_exp1;
    }
    //calculate_first_order = false;  // this is never actually used
    compare_first_order = true;
    calculated_first = true;

    if(!calculate_second_order)
        goto comparing;

    calculate_second:
    sum_2nd = shiftedDefaultPos2;
    new_pos2 = 0;
    new_exp2 = smallestExponent2;
    for(uint i = 0; i < this->actions.size(); i++){
        sum_2nd += this->actions[i].moves_2nd[this->actions[i].amount_index];
    }
    if(sum_2nd){
        if(sum_2nd < 0){
            new_pos2 |= 0x80000000;
            sum_2nd *= -1;
        }
        while(sum_2nd > MANTISSA_MAX){
            sum_2nd >>= 1;
            new_exp2 += EXPONENT_UNIT;
        }
        while(sum_2nd < MANTISSA_MIN){
            sum_2nd <<= 1;
            new_exp2 -= EXPONENT_UNIT;
        }
        sum_2nd &= ~0x00800000;
        new_pos2 |= sum_2nd | new_exp2;
    }
    calculate_second_order = false;
    compare_second_order = true;
    calculated_second = true;

    comparing:  // comparing searched to calculated values
    if (compare_first_order && !compare_second_order)
        goto compare_first;
    else if (!compare_first_order && compare_second_order)
        goto compare_second;
    // should not be possible to land here without at least one compare-order

    // comparing both
    for(uint i = 0; i < costs_ok.size(); i++){
        Searched* searched_p = &this->searched[costs_ok[i]];
        for(int t = -(searched_p->tol1); t <= searched_p->tol1; t++){
            WORD p = new_pos1 + t;
            p &= searched_p->and1;
            if(p == searched_p->compare1){
                hits1.push_back(costs_ok[i]);
                break;
            }
        }
        for(int t = -(searched_p->tol2); t <= searched_p->tol2; t++){
            WORD p = new_pos2 + t;
            p &= searched_p->and2;
            if(p == searched_p->compare2){
                hits2.push_back(costs_ok[i]);
                break;
            }
        }
    }
    compare_first_order = false;
    compare_second_order = false;
    goto after_comparison;

    compare_first:
    for(uint i = 0; i < costs_ok.size(); i++){
        Searched* searched_p = &this->searched[costs_ok[i]];
        for(int t = -(searched_p->tol1); t <= searched_p->tol1; t++){
            WORD p = new_pos1 + t;
            p &= searched_p->and1;
            if(p == searched_p->compare1){
                hits1.push_back(costs_ok[i]);
                break;
            }
        }
    }
    compare_first_order = false;
    goto after_comparison;

    compare_second:
    for(uint i = 0; i < costs_ok.size(); i++){
        Searched* searched_p = &this->searched[costs_ok[i]];
        for(int t = -(searched_p->tol2); t <= searched_p->tol2; t++){
            WORD p = new_pos2 + t;
            p &= searched_p->and2;
            if(p == searched_p->compare2){
                hits2.push_back(costs_ok[i]);
                break;
            }
        }
    }
    compare_second_order = false;

    after_comparison:
    if(hits1.empty() && hits2.empty())
        goto done;

    // gather solution data for searches with only one position searched
    if(!(this->search_first && this->search_second)){
        if(this->search_first){
            for(uint i = 0; i < hits1.size(); i++){
                this->AddFullSolution(hits1[i],costs,true,false,new_pos1,NULL_VALUE,this->xyz_index);
                this->searched[hits1[i]].UpdateCosts(costs);
                return_val |= FOUND_FULL_SOLUTION;
            }
        }
        else{
            for(uint i = 0; i < hits2.size(); i++){
                this->AddFullSolution(hits2[i],costs,false,true,NULL_VALUE,new_pos2,this->xyz_index);
                this->searched[hits2[i]].UpdateCosts(costs);
                return_val |= FOUND_FULL_SOLUTION;
            }
        }
        goto done;
    }

    // solution data for searches with both positions searched

    // we need both positions calculated in any case
    if(!calculated_first)   // Is this condition even possible?
        goto calculate_first;
    else if (!calculated_second)
        goto calculate_second;

    // no resettables means we need a direct hit of both coordinates
    if(!this->first_resettable && !this->second_resettable){
        if(hits1.empty() || hits2.empty())
            goto done;
        for(uint i = 0; i < hits1.size(); i++){
            uint searched_id = hits1[i];
            for(uint j = 0; j < hits2.size(); j++){
                if((uint)hits2[j] == searched_id){
                    this->AddFullSolution(searched_id,costs,true,true,new_pos1,new_pos2,this->xyz_index);
                    this->searched[searched_id].UpdateCosts(costs);
                    return_val |= FOUND_FULL_SOLUTION;
                }
            }
        }
        goto done;
    }

    // with resettables look for a direct hit first, add to full solutions and set those hits to negative
    for(uint i = 0; i < hits1.size(); i++){
        int searched_id = hits1[i];
        for(uint j = 0; j < hits2.size(); j++){
            if(searched_id == hits2[j]){
                this->AddFullSolution(searched_id,costs,true,true,new_pos1,new_pos2,this->xyz_index);
                this->searched[searched_id].UpdateCosts(costs);
                return_val |= FOUND_FULL_SOLUTION;
                hits1[i] = -1;
                hits2[j] = -1;
                break;
            }
        }
    }

    // everything left goes to partial solutions, if the coordinate is resettable
    if(this->first_resettable){
        for(uint i = 0; i < hits1.size(); i++){
            int searched_id = hits1[i];
            if (searched_id < 0) // ignore the negative ones since they were full solutions
                continue;
            this->AddPartialSolution(searched_id,costs,true,false,new_pos1,new_pos2,this->xyz_index);
            this->searched[searched_id].UpdateCosts(costs);
            return_val |= FOUND_PARTIAL_SOLUTION;
        }
    }
    if(this->second_resettable){
        for(uint i = 0; i < hits2.size(); i++){
            int searched_id = hits2[i];
            if (searched_id < 0) // ignore the negative ones since they were full solutions
                continue;
            this->AddPartialSolution(searched_id,costs,false,true,new_pos1,new_pos2,this->xyz_index);
            this->searched[searched_id].UpdateCosts(costs);
            return_val |= FOUND_PARTIAL_SOLUTION;
        }
    }

    done:

    // debug output
    if(output != nullptr){
        if(return_val & COSTS_TOO_HIGH)
            *output = "COSTS_TOO_HIGH | costs =" + QString::number(costs);
        else{
            *output += "result = " + QString::number(return_val,16);
            *output += " , new_pos1 = " + QString::number(new_pos1,16) + " , new_pos2 = " + QString::number(new_pos2,16);
            *output += " , full solutions total: " + QString::number(this->full_solutions.size());
            *output += " , partial solutions total: " + QString::number(this->partial_solutions.size());
            *output += ", costs: " + QString::number(costs);


        }
    }


    this->iterations_done++;

    return return_val;
}

void Search::AddPartialSolution(uint searched_id, int costs, bool found_first, bool found_second, WORD pos1, WORD pos2, uint xyz_index){
    this->partial_solutions.push_back(Solution(searched_id, costs, found_first, found_second, pos1, pos2, xyz_index));
    for(uint i = 0; i < this->actions.size(); i++){
        this->partial_solutions.back().action_amounts.push_back(this->actions[i].amounts_order[this->actions[i].amount_index]);
    }
}

void Search::AddFullSolution(uint searched_id, int costs, bool found_first, bool found_second, WORD pos1, WORD pos2, uint xyz_index){
    this->full_solutions.push_back(Solution(searched_id, costs, found_first, found_second, pos1, pos2, xyz_index));
    for(uint i = 0; i < this->actions.size(); i++){
        this->full_solutions.back().action_amounts.push_back(this->actions[i].amounts_order[this->actions[i].amount_index]);
    }
}

int Search::AdvanceActionAmounts(){
    for(uint i = 0; i < this->actions.size(); i++){
        this->actions[i].amount_index++;
        if(this->actions[i].amount_index >= this->actions[i].amounts_total){
            if(i == (this->actions.size() - 1))
                return SEARCH_DONE;
            this->actions[i].amount_index = 0;
        }
        else
            break;
    }
    return SUCCESS;
}







