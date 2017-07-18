#ifndef ZERO_MULTI_CLOCK_HPP
#define ZERO_MULTI_CLOCK_HPP

#include "multi_clock.h"

#include "w_base.h"
#include "multi_clock_exceptions.h"

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::multi_clock(key clocksize) {
    _clocksize = clocksize;
    _values = new value[_clocksize]();
    _clocks = new index_pair[_clocksize]();

    _hands = new key[_clocknumber]();
    _sizes = new key[_clocknumber]();
    for (clk_idx i = 0; i <= _clocknumber - 1; i++) {
        _hands[i] = _invalid_index;
    }
    _clock_membership = new clk_idx[_clocksize]();
    for (key i = 0; i <= _clocksize - 1; i++) {
        _clock_membership[i] = _invalid_clock_index;
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::~multi_clock() {
    delete[](_values);
    delete[](_clocks);
    delete[](_clock_membership);

    delete[](_hands);
    delete[](_sizes);
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
value multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::get_head(const clk_idx clock)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (/*valid_clock_index(clock) && */!empty(clock)) {
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return _values[_hands[clock]];
    } else {
        w_assert1(_hands[clock] == _invalid_index);
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));
        else if (empty(clock))
            multi_exception.addException(
                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
        return _values[_invalid_index];                         // Suppress compiler warning about missing return statement!
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::set_head(const clk_idx clock, const value new_value)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (/*valid_clock_index(clock) && */!empty(clock)) {
        _values[_hands[clock]] = new_value;
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));
        else if (empty(clock))
            multi_exception.addException(
                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
key multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::get_head_index(const clk_idx clock)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (/*valid_clock_index(clock) && */!empty(clock)) {
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return _hands[clock];
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));
        else if (empty(clock))
            multi_exception.addException(
                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
        return _invalid_index;                                  // Suppress compiler warning about missing return statement!
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::move_head(const clk_idx clock)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (/*valid_clock_index(clock) && */!empty(clock)) {
        _hands[clock] = _clocks[_hands[clock]]._after;
        w_assert1(_clock_membership[_hands[clock]] == clock);
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));
        else if (empty(clock))
            multi_exception.addException(
                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::add_tail(const clk_idx clock, const key index)
                                throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
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
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_index(index))
            multi_exception.addException(
                    multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, index));
        if (contained_index(index))
            multi_exception.addException(
                    multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, index));
        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::add_before(const key inside, const key new_entry)
                                throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {

    if (valid_index(new_entry) && !contained_index(new_entry)
        && contained_index(inside)) {
        w_assert1(_sizes[_clock_membership[inside]] >= 1);
        _clocks[new_entry]._before = _clocks[inside]._before;
        _clocks[new_entry]._after = inside;
        _clocks[inside]._before = new_entry;
        _clock_membership[new_entry] = _clock_membership[inside];
        _sizes[_clock_membership[inside]]++;
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_index(new_entry))
            multi_exception.addException(
                    multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, new_entry));
        if (contained_index(new_entry))
            multi_exception.addException(
                    multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, new_entry));
        if (!valid_index(inside))
            multi_exception.addException(
                    multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, inside));
        else if (!contained_index(inside))
            multi_exception.addException(
                    multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, inside));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::add_after(const key inside, const key new_entry)
                                throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (valid_index(new_entry) && !contained_index(new_entry)
        && contained_index(inside)) {
        w_assert1(_sizes[_clock_membership[inside]] >= 1);
        _clocks[new_entry]._after = _clocks[inside]._after;
        _clocks[new_entry]._before = inside;
        _clocks[inside]._after = new_entry;
        _clock_membership[new_entry] = _clock_membership[inside];
        _sizes[_clock_membership[inside]]++;
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_index(new_entry))
            multi_exception.addException(
                    multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, new_entry));
        if (contained_index(new_entry))
            multi_exception.addException(
                    multi_clock_already_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, new_entry));
        if (!valid_index(inside))
            multi_exception.addException(
                    multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, inside));
        else if (!contained_index(inside))
            multi_exception.addException(
                    multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, inside));

        if (multi_exception.size() == 1) {
            throw multi_exception.getExceptions()[0];
        } else {
            throw multi_exception;
        }
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
key multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::remove_head(const clk_idx clock)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    if (/*valid_clock_index(clock) && */!empty(clock)) {
        key removed_index = _hands[clock];
        remove(removed_index);
        return removed_index;
    } else {
        w_assert1(_hands[clock] == _invalid_index);
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_clock_index(clock))
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));
        else if (empty(clock))
            multi_exception.addException(
                    multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, clock));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
        return _invalid_index;                                  // Suppress compiler warning about missing return statement!
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
void multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::remove(key index)
                                throw (multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
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
        } else {
            _clocks[_clocks[index]._before]._after = _clocks[index]._after;
            _clocks[_clocks[index]._after]._before = _clocks[index]._before;
            _hands[clock] = _clocks[index]._after;
            _clocks[index]._before = _invalid_index;
            _clocks[index]._after = _invalid_index;
            _clock_membership[index] = _invalid_clock_index;
            _sizes[clock]--;

            w_assert1(_hands[clock] != _invalid_index);
        }
    } else {
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);

        if (!valid_index(index))
            multi_exception.addException(multi_clock_invalid_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, index));
        else if (!contained_index(index))
            multi_exception.addException(multi_clock_not_contained_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, index));

        throw_multiple<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(multi_exception);
    }
}

template<class key, class value, uint32_t _clocknumber, key _invalid_index, key _invalid_clock_index>
key multi_clock<key, value, _clocknumber, _invalid_index, _invalid_clock_index>::switch_head_to_tail(const clk_idx source, const clk_idx destination)
                                throw (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_empty_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>,
                                       multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>) {
    key moved_index = _invalid_index;
    try {
        moved_index = remove_head(source);
    } catch (multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> e) {
        // Add exceptions of add_tail that would have been thrown if remove_head would have worked:
        if (!valid_clock_index(destination)) {
            e.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, destination));
        }
        throw e;
    } catch (multi_clock_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> e) {
        // Add exceptions of add_tail that would have been thrown if remove_head would have worked:
        multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> multi_exception
                = multi_clock_multi_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes);
        multi_exception.addException(e);
        if (!valid_clock_index(destination)) {
            multi_exception.addException(
                    multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index>(this, _hands, _sizes, destination));
        }
        throw_multiple(multi_exception);
        return _invalid_index;                                  // Suppress compiler warning about missing return statement!
    }

    w_assert1(moved_index != _invalid_clock_index);

    try {
        add_tail(destination, moved_index);
    } catch (multi_clock_invalid_clock_index_exception<key, value, _clocknumber, _invalid_index, _invalid_clock_index> e) {
        // Rollback the removal:
        add_tail(source, moved_index);
        _hands[source] = moved_index;

        throw e;
    }
}

#endif //ZERO_MULTI_CLOCK_HPP
