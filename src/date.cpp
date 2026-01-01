/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <tuple>
#include <random>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <date.hpp>

namespace date_time {

Date::Date(int d, int m, int y)
{
    set(d, m, y);
}

Date::Date(const char* p)
{
    if (std::string date{p}; std::count(date.begin(), date.end(), '/') == 2) {
        auto index = date.find('/');
        int d = std::stoi(date.substr(0, index));
        date.erase(0, index + 1);
        index = date.find('/');
        int m = std::stoi(date.substr(0, index));
        date.erase(0, index + 1);
        int y = std::stoi(date);
        set(d, m, y);
    } else {
        throw bad_day{"wrong input format!"};
    }
}

Date::Date(const std::time_t& timer)
{
    std::string_view date{std::ctime(&timer)};
    int d = std::stoi(std::string{date.substr(8, date.find_first_of(' ', 8))});
    int m = Date::convert_month(date.substr(4, 3));
    int y = std::stoi(std::string{date.substr(date.find_last_of(' '))});
    set(d, m, y);
}

bool Date::check_arguments(int d, int m, int y)
{
    if ((y >= year_base) && (m >= 1 && m <= 12)){
        return (d >= 1 && d <= year_table[isleap(y)][m]);
    }
    return false;
}

int Date::get_month_day() const
{
    return m_day;
}

int Date::get_month() const
{
    return m_month;
}

std::string Date::convert_month(int m)
{
    switch (m){
    case 1:  return "January";
    case 2:  return "February";
    case 3:  return "March";
    case 4:  return "April";
    case 5:  return "May";
    case 6:  return "June";
    case 7:  return "July";
    case 8:  return "August";
    case 9:  return "September";
    case 10: return "October";
    case 11: return "November";
    case 12: return "December";
    default: return ""; // wrong argument
    }
}

int Date::convert_month(const std::string_view& m)
{
    if (m == "Jan"){ return 1;}
    if (m == "Feb"){ return 2;}
    if (m == "Mar"){ return 3;}
    if (m == "Apr"){ return 4;}
    if (m == "May"){ return 5;}
    if (m == "Jun"){ return 6;}
    if (m == "Jul"){ return 7;}
    if (m == "Aug"){ return 8;}
    if (m == "Sep"){ return 9;}
    if (m == "Oct"){ return 10;}
    if (m == "Nov"){ return 11;}
    if (m == "Dec"){ return 12;}
    return -1; // wrong argument
}

int Date::get_year() const
{
    return m_year;
}

int Date::get_year_day() const
{
    int days{};
    for (auto i = 1; i < m_month; ++i){
        days += year_table[isleap(m_year)][i];
    }
    days += m_day;
    return days;
}

Date::Weekday Date::get_week_day() const
{
    return m_weekday;
}

void Date::set_weekday()
{
    std::uint64_t days{};
    for (auto i = year_base; i < m_year; ++i){
        isleap(i) ? days += 366 : days += 365;
    }
    days += get_year_day();
    days %= 7;
    m_weekday = static_cast<Weekday>(days);
}

Date& Date::set_month_day(int _day)
{
    set(_day, m_month, m_year);
    return *this;
}

Date& Date::set_month(int _month)
{
    set(m_day, _month, m_year);
    return *this;
}

Date& Date::set_year(int _year)
{
    set(m_day, m_month, _year);
    return *this;
}

Date& Date::set(int d, int m, int y)
{
    if (check_arguments(d, m, y)){
        m_day = d;
        m_month = m;
        m_year = y;
        set_weekday();
    } else {
        throw bad_day{};
    }
    return *this;
}

Date Date::operator-(int day) const
{
    Date temp{*this};
    for(auto i = 0; i < day; ++i){
        --temp;
    }
    return temp;
}

Date& Date::operator+=(int day)
{
    for(auto i = 0; i < day; ++i){
        operator++();
    }
    return *this;
}

Date& Date::operator-=(int day)
{
    for(auto i = 0; i < day; ++i){
        operator--();
    }
    return *this;
}

Date& Date::operator++()
{
    if (m_month == 12 && m_day == 31){
        set(1, 1, m_year + 1);
    } else if (m_day == year_table[isleap(m_year)][m_month]){
        set(1, m_month + 1, m_year);
    } else {
        set_month_day(m_day + 1);
    }
    return *this;
}

Date Date::operator++(int)
{
    Date temp{*this};
    operator++();
    return temp;
}

Date& Date::operator--()
{
    if (m_month == 1 && m_day == 1){
        set(31, 12, m_year - 1);
    } else if (m_day == 1){
        set(year_table[isleap(m_year) + 2][m_month], m_month - 1, m_year);
    } else {
        set_month_day(m_day - 1);
    }
    return *this;
}

Date Date::operator--(int)
{
    Date temp{*this};
    operator--();
    return temp;
}

std::int64_t operator-(const Date& d1, const Date& d2)
{
    if (d1 == d2){ return 0;}
    if (d1.get_year() == d2.get_year()){
        return (d1.get_year_day() - d2.get_year_day());
    }
    bool negative{false};
    int year_diff = d1.get_year() - d2.get_year();
    if (year_diff < 0){
        negative = true;
        year_diff *= -1;
    }
    std::int64_t day_diff{};
    for (auto i = 0; i < year_diff; ++i){
        Date::isleap(i) ? day_diff += 366 : day_diff += 365;
    }
    if (negative){
        day_diff += d2.get_year_day() - d1.get_year_day();
        return (-day_diff);
    }
    day_diff += d1.get_year_day() - d2.get_year_day();
    return day_diff;
}

Date operator+(const Date& date, int n)
{
    Date temp{date};
    for (auto i = 0; i < n; ++i){
        ++temp;
    }
    return temp;
}

Date operator+(int n, const Date& date)
{
    return operator+(date, n);
}

Date::Weekday& operator++(Date::Weekday& r)
{
    int weekday = static_cast<int>(r);
    if (weekday == 6) {
        r = Date::Weekday::Sunday;
    } else {
        ++weekday;
        r = static_cast<Date::Weekday>(weekday);
    }
    return r;
}

Date::Weekday operator++(Date::Weekday& r, int)
{
    Date::Weekday temp{r};
    operator++(r);
    return temp;
}

Date::Weekday& operator--(Date::Weekday& r)
{
    int weekday = static_cast<int>(r);
    if (weekday == 0) {
        r = Date::Weekday::Saturday;
    } else {
        --weekday;
        r = static_cast<Date::Weekday>(weekday);
    }
    return r;
}

Date::Weekday operator--(Date::Weekday& r, int)
{
    Date::Weekday temp{r};
    operator--(r);
    return temp;
}

Date Date::random_date()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution rand_d(1, 31);
    std::uniform_int_distribution rand_m(1, 12);
    std::uniform_int_distribution rand_y(random_min_year, random_max_year);
    int d{}, m{}, y{};
    while (!check_arguments(d, m, y)){
        d = rand_d(gen);
        m = rand_m(gen);
        y = rand_y(gen);
    }
    return Date{d, m, y};
}

std::string Date::to_string() const
{
    std::ostringstream oss;
    oss << m_day << " " << Date::convert_month(m_month)
        << " " << m_year << " " << m_weekday;
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Date& date)
{
    return os << date.to_string();
}

std::ostream& operator<<(std::ostream& os, const Date::Weekday& w)
{
    switch (w){
    case Date::Weekday::Sunday:    os << "Sunday"; break;
    case Date::Weekday::Monday:    os << "Monday"; break;
    case Date::Weekday::Tuesday:   os << "Tuesday"; break;
    case Date::Weekday::Wednesday: os << "Wednesday"; break;
    case Date::Weekday::Thursday:  os << "Thursday"; break;
    case Date::Weekday::Friday:    os << "Friday"; break;
    case Date::Weekday::Saturday:  os << "Saturday"; break;
    }
    return os;
}

std::istream& operator>>(std::istream& is, Date& date)
{
    std::string input;
    is >> input;
    int nondigits{};
    for (auto& c : input){
        if (!std::isdigit(c)){
            ++nondigits;
        }
    }
    if (nondigits == 2){
        int d = std::stoi(input.substr(0, input.find_first_not_of("0123456789")));
        input.erase(0, input.find_first_not_of("0123456789") + 1);
        int m = std::stoi(input.substr(0, input.find_first_not_of("0123456789")));
        input.erase(0, input.find_first_not_of("0123456789") + 1);
        int y = std::stoi(input);
        date.set(d, m, y);
    } else {
        throw bad_day{"wrong input format!"};
    }
    return is;
}

} /* namespace date_time */
