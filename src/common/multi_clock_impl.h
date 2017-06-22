#ifndef ZERO_MULTI_CLOCK_IMPL_H
#define ZERO_MULTI_CLOCK_IMPL_H

#include "multi_clock.h"

#include "w_base.h"

template<class key, class value>
multi_clock<key, value>::multi_clock(key clocksize, clk_idx clocknumber, key invalid_index) {
    _clocksize = clocksize;
    _values = new value[_clocksize]();
    _clocks = new index_pair[_clocksize]();
    _invalid_index = invalid_index;
    
    _clocknumber = clocknumber;
    _hands = new key[_clocknumber]();
    _sizes = new key[_clocknumber]();
    for (clk_idx i = 0; i <= _clocknumber - 1; i++) {
        _hands[i] = _invalid_index;
    }
    _invalid_clock_index = _clocknumber;
    _clock_membership = new clk_idx[_clocksize]();
    for (key i = 0; i <= _clocksize - 1; i++) {
        _clock_membership[i] = _invalid_clock_index;
    }
}

template<class key, class value>
multi_clock<key, value>::~multi_clock() {
    _clocksize = 0;
    delete[](_values);
    delete[](_clocks);
    delete[](_clock_membership);
    
    _clocknumber = 0;
    delete[](_hands);
    delete[](_sizes);
}

template<class key, class value>
bool multi_clock<key, value>::get_head(const clk_idx clock, value &head_value) {
    if (!empty(clock)) {
        head_value = _values[_hands[clock]];
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        head_value = _values[_invalid_index];
        w_assert1(_hands[clock] == _invalid_index);
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::set_head(const clk_idx clock, const value new_value) {
    if (!empty(clock)) {
        _values[_hands[clock]] = new_value;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::get_head_index(const clk_idx clock, key &head_index) {
    if (!empty(clock)) {
        head_index = _hands[clock];
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        head_index = _invalid_index;
        w_assert1(head_index == _invalid_index);
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::move_head(const clk_idx clock) {
    if (!empty(clock)) {
        _hands[clock] = _clocks[_hands[clock]]._after;
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::add_tail(const clk_idx clock, const key index) {
    if (valid_index(index) && !contained_index(index)
     && valid_clock_index(clock)) {
        if (empty(clock)) {
            _hands[clock] = index;
            _clocks[index]._before = index;
            _clocks[index]._after = index;
        } else {
            _clocks[index]._before = _clocks[_hands[clock]]._before;
            _clocks[index]._after = _hands[clock];
            _clocks[_clocks[_hands[clock]]._before]._after = index;
            _clocks[_hands[clock]]._before = index;
        }
        _sizes[clock]++;
        _clock_membership[index] = clock;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::add_after(const key inside, const key new_entry) {
    if (valid_index(new_entry) && !contained_index(new_entry)
     && contained_index(inside)) {
        w_assert1(_sizes[_clock_membership[inside]] >= 1);
        _clocks[new_entry]._after = _clocks[inside]._after;
        _clocks[new_entry]._before = inside;
        _clocks[inside]._after = new_entry;
        _clock_membership[new_entry] = _clock_membership[inside];
        _sizes[_clock_membership[inside]]++;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::add_before(const key inside, const key new_entry) {
    if (valid_index(new_entry) && !contained_index(new_entry)
     && contained_index(inside)) {
        w_assert1(_sizes[_clock_membership[inside]] >= 1);
        _clocks[new_entry]._before = _clocks[inside]._before;
        _clocks[new_entry]._after = inside;
        _clocks[inside]._before = new_entry;
        _clock_membership[new_entry] = _clock_membership[inside];
        _sizes[_clock_membership[inside]]++;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::remove_head(const clk_idx clock, key &removed_index) {
    if (!empty(clock)) {
        removed_index = _hands[clock];
        w_assert0(remove(removed_index));
        return true;
    } else {
        removed_index = _invalid_index;
        w_assert1(_hands[clock] == _invalid_index);
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::remove(key &index) {
    if (contained_index(index)) {
        clk_idx clock = _clock_membership[index];
        if (_sizes[clock] == 1) {
            w_assert1(_hands[clock] >= 0 && _hands[clock] <= _clocksize - 1 && _hands[clock] != _invalid_index);
            w_assert1(_clocks[_hands[clock]]._before == _hands[clock]);
            w_assert1(_clocks[_hands[clock]]._after == _hands[clock]);
            
            _clocks[index]._before = _invalid_index;
            _clocks[index]._after = _invalid_index;
            _hands[clock] = _invalid_index;
            _clock_membership[index] = _invalid_clock_index;
            _sizes[clock]--;
            return true;
        } else {
            _clocks[_clocks[index]._before]._after = _clocks[index]._after;
            _clocks[_clocks[index]._after]._before = _clocks[index]._before;
            _hands[clock] = _clocks[index]._after;
            _clocks[index]._before = _invalid_index;
            _clocks[index]._after = _invalid_index;
            _clock_membership[index] = _invalid_clock_index;
            _sizes[clock]--;
            
            w_assert1(_hands[clock] != _invalid_index);
            return true;
        }
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::switch_head_to_tail(const clk_idx source, const clk_idx destination,
                                                  key &moved_index) {
    moved_index = _invalid_index;
    if (!empty(source) && valid_clock_index(destination)) {
        w_assert0(remove_head(source, moved_index));
        w_assert1(moved_index != _invalid_index);
        w_assert0(add_tail(destination, moved_index));
        
        return true;
    } else {
        return false;
    }
}

#endif //ZERO_MULTI_CLOCK_IMPL_H
