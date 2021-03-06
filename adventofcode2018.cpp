// adventofcode2018.cpp

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <future>
#include <thread>
#include <forward_list>
#include <functional>
#include <queue>
#include <stack>
#include <limits>
#include <unordered_set>
#include <unordered_map>

#include <coveo/linq.h>

#include "Dijkstra.h"

using namespace coveo::linq;

void day1_1()
{
    int freq = 0;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "0") {
            break;
        }
        bool plus = (line[0] == '+');
        line = line.substr(1);
        std::istringstream iss(line);
        int mod = 0;
        iss >> mod;
        if (!plus) {
            mod = -mod;
        }
        freq += mod;
    }
    std::cout << freq << std::endl;
}

void day1_2()
{
    std::vector<int> input;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "0") {
            break;
        }
        bool plus = (line[0] == '+');
        line = line.substr(1);
        std::istringstream iss(line);
        int mod = 0;
        iss >> mod;
        if (!plus) {
            mod = -mod;
        }
        input.push_back(mod);
    }

    int freq = 0;
    std::set<int> seen;
    bool go = true;
    while (go) {
        for (auto it = input.begin(); go && it != input.end(); ++it) {
            freq += *it;
            if (!seen.emplace(freq).second) {
                go = false;
            }
        }
    }
    std::cout << freq << std::endl;
}

void day2_1()
{
    int two = 0, three = 0;
    for (;;) {
        std::string word;
        std::getline(std::cin, word);
        if (word == "out") {
            break;
        }
        std::map<char, int> m;
        for (auto it = word.begin(); it != word.end(); ++it) {
            ++m[*it];
        }
        bool has_two = false, has_three = false;
        for (auto it = m.begin(); it != m.end(); ++it) {
            if (it->second == 2) {
                has_two = true;
            } else if (it->second == 3) {
                has_three = true;
            }
            if (has_two && has_three) {
                break;
            }
        }
        if (has_two) {
            ++two;
        }
        if (has_three) {
            ++three;
        }
    }
    int checksum = two * three;
    std::cout << checksum << std::endl;
}

void day2_2()
{
    std::vector<std::string> input;
    for (;;) {
        std::string word;
        std::getline(std::cin, word);
        if (word == "out") {
            break;
        }
        input.push_back(word);
    }

    bool go = true;
    for (auto it1 = input.begin(); go && it1 != input.end(); ++it1) {
        for (auto it2 = input.begin(); go && it2 != input.end(); ++it2) {
            if (it1 != it2) {
                int sim = 0;
                std::string common;
                for (auto wit1 = it1->begin(), wit2 = it2->begin(); wit1 != it1->end() && wit2 != it2->end(); ++wit1, ++wit2) {
                    if (*wit1 == *wit2) {
                        ++sim;
                        common.push_back(*wit1);
                    }
                }
                if (sim == it1->size() - 1) {
                    std::cout << common << std::endl;
                    go = false;
                }
            }
        }
    }
}

void day3_1()
{
    struct piece {
        int id = 0, x = 0, y = 0, width = 0, height = 0;
    };
    std::vector<piece> pieces;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        line = line.substr(1); // drop the #
        std::istringstream iss(line);
        char c = 0;
        piece p;
        iss >> p.id >> c;       // c reads the @
        iss >> p.x >> c;        // c reads the ,
        iss >> p.y >> c;        // c reads the :
        iss >> p.width >> c;    // c reads the x
        iss >> p.height;
        pieces.push_back(p);
    }

    std::vector<std::vector<int>> fabric;
    fabric.resize(1000);
    for (auto&& f : fabric) {
        f.resize(1000);
    }
    for (auto&& p : pieces) {
        for (int x = p.x; x < p.x + p.width; ++x) {
            for (int y = p.y; y < p.y + p.height; ++y) {
                ++fabric[x][y];
            }
        }
    }

    int overlaps = 0;
    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            if (fabric[i][j] > 1) {
                ++overlaps;
            }
        }
    }
    std::cout << overlaps << std::endl;
}

void day3_2()
{
    struct piece {
        int id = 0, x = 0, y = 0, width = 0, height = 0;
    };
    std::vector<piece> pieces;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        line = line.substr(1); // drop the #
        std::istringstream iss(line);
        char c = 0;
        piece p;
        iss >> p.id >> c;       // c reads the @
        iss >> p.x >> c;        // c reads the ,
        iss >> p.y >> c;        // c reads the :
        iss >> p.width >> c;    // c reads the x
        iss >> p.height;
        pieces.push_back(p);
    }

    std::vector<std::vector<int>> fabric;
    fabric.resize(1000);
    for (auto&& f : fabric) {
        f.resize(1000);
    }
    for (auto&& p : pieces) {
        for (int x = p.x; x < p.x + p.width; ++x) {
            for (int y = p.y; y < p.y + p.height; ++y) {
                ++fabric[x][y];
            }
        }
    }

    piece lone;
    for (auto&& p : pieces) {
        bool good = true;
        for (int x = p.x; good && x < p.x + p.width; ++x) {
            for (int y = p.y; good && y < p.y + p.height; ++y) {
                good = fabric[x][y] == 1;
            }
        }
        if (good) {
            lone = p;
            break;
        }
    }
    std::cout << lone.id << std::endl;
}

void day4_1()
{
    struct datentime {
        int y = 0, m = 0, d = 0, h = 0, min = 0;
        bool operator<(const datentime& right) const {
            int cmp = y - right.y;
            if (cmp == 0) {
                cmp = m - right.m;
                if (cmp == 0) {
                    cmp = d - right.d;
                    if (cmp == 0) {
                        cmp = h - right.h;
                        if (cmp == 0) {
                            cmp = min - right.min;
                        }
                    }
                }
            }
            return cmp < 0;
        }
    };
    std::map<datentime, std::string> entries;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        datentime date;
        std::string entry;
        char c = 0;
        iss >> c >> date.y
            >> c >> date.m
            >> c >> date.d
                 >> date.h
            >> c >> date.min
            >> c;
        std::getline(iss, entry);
        while (entry.front() == ' ') {
            entry = entry.substr(1);
        }
        entries.emplace(date, entry);
    }

    struct timeperiod {
        datentime start, end;
    };
    std::map<int, std::vector<timeperiod>> guards;
    std::vector<timeperiod>* pcurperiods = nullptr;
    for (auto&& e : entries) {
        if (e.second.front() == 'G') {
            std::istringstream iss(e.second);
            std::string foo;
            char c = 0;
            int id = 0;
            iss >> foo >> c >> id;
            pcurperiods = &guards[id];
        } else if (e.second.front() == 'f') {
            pcurperiods->push_back({e.first, {}});
        } else {
            pcurperiods->back().end = e.first;
        }
    }

    struct guardinfo {
        int asleep_mins;
        int mins[60];
        guardinfo() : asleep_mins(0), mins() { }
    };
    std::map<int, guardinfo> guardinfos;
    for (auto&& g : guards) {
        auto& gi = guardinfos[g.first];
        for (auto&& tp : g.second) {
            gi.asleep_mins += (tp.end.min - tp.start.min);
            for (int i = tp.start.min; i < tp.end.min; ++i) {
                ++gi.mins[i];
            }
        }
    }

    auto winner = std::max_element(guardinfos.begin(), guardinfos.end(),
                                   [](auto&& left, auto&& right) {
                                       return left.second.asleep_mins < right.second.asleep_mins;
                                   });
    int max_min = 0;
    int max_value = 0;
    for (int i = 0; i < 60; ++i) {
        if (winner->second.mins[i] > max_value) {
            max_min = i;
            max_value = winner->second.mins[i];
        }
    }
    std::cout << winner->first * max_min << std::endl;
}

void day4_2()
{
    struct datentime {
        int y = 0, m = 0, d = 0, h = 0, min = 0;
        bool operator<(const datentime& right) const {
            int cmp = y - right.y;
            if (cmp == 0) {
                cmp = m - right.m;
                if (cmp == 0) {
                    cmp = d - right.d;
                    if (cmp == 0) {
                        cmp = h - right.h;
                        if (cmp == 0) {
                            cmp = min - right.min;
                        }
                    }
                }
            }
            return cmp < 0;
        }
    };
    std::map<datentime, std::string> entries;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        datentime date;
        std::string entry;
        char c = 0;
        iss >> c >> date.y
            >> c >> date.m
            >> c >> date.d
                 >> date.h
            >> c >> date.min
            >> c;
        std::getline(iss, entry);
        while (entry.front() == ' ') {
            entry = entry.substr(1);
        }
        entries.emplace(date, entry);
    }

    struct timeperiod {
        datentime start, end;
    };
    std::map<int, std::vector<timeperiod>> guards;
    std::vector<timeperiod>* pcurperiods = nullptr;
    for (auto&& e : entries) {
        if (e.second.front() == 'G') {
            std::istringstream iss(e.second);
            std::string foo;
            char c = 0;
            int id = 0;
            iss >> foo >> c >> id;
            pcurperiods = &guards[id];
        } else if (e.second.front() == 'f') {
            pcurperiods->push_back({e.first, {}});
        } else {
            pcurperiods->back().end = e.first;
        }
    }

    std::vector<std::map<int, int>> minutes;
    minutes.resize(60);
    for (auto&& g : guards) {
        for (auto&& tp : g.second) {
            for (int i = tp.start.min; i < tp.end.min; ++i) {
                ++minutes[i][g.first];
            }
        }
    }

    int minute = 0;
    int num_sleeps = 0;
    int guard_id = 0;
    for (int i = 0; i < 60; ++i) {
        for (auto&& gi : minutes[i]) {
            if (gi.second > num_sleeps) {
                minute = i;
                num_sleeps = gi.second;
                guard_id = gi.first;
            }
        }
    }
    std::cout << guard_id * minute << std::endl;
}

void day5_1()
{
    std::list<char> chain;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        chain.push_back(line[0]);
    }

    int removals;
    do {
        removals = 0;
        auto it = chain.begin();
        while (it != chain.end()) {
            auto itnext = std::next(it);
            if (itnext == chain.end()) {
                break;
            }
            if (std::tolower(*it) == std::tolower(*itnext) && *it != *itnext) {
                it = chain.erase(it, std::next(itnext));
                ++removals;
            } else {
                ++it;
            }
        }
        std::cout << removals << std::endl;
    } while (removals > 0);
    std::cout << chain.size() << std::endl;
}

void day5_2()
{
    std::list<char> chain;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        chain.push_back(line[0]);
    }

    auto compute = [&](char c) -> int {
        std::list<char> newchain(chain);
        {
            auto it = newchain.begin();
            while (it != newchain.end()) {
                if (std::tolower(*it) == c) {
                    it = newchain.erase(it);
                } else {
                    ++it;
                }
            }
        }

        int removals;
        do {
            removals = 0;
            auto it = newchain.begin();
            while (it != newchain.end()) {
                auto itnext = std::next(it);
                if (itnext == newchain.end()) {
                    break;
                }
                if (std::tolower(*it) == std::tolower(*itnext) && *it != *itnext) {
                    it = newchain.erase(it, std::next(itnext));
                    ++removals;
                } else {
                    ++it;
                }
            }
        } while (removals > 0);
        return static_cast<int>(newchain.size());
    };
    std::vector<std::future<int>> futures;
    for (char c = 'a'; c <= 'z'; ++c) {
        futures.emplace_back(std::async(std::launch::async, compute, c));
    }
    int shortest_len = from(futures)
                     | min([](auto&& f) { return f.get(); });
    std::cout << shortest_len << std::endl;
}

void day6_1()
{
    struct coord {
        int x, y;
        coord(int i = 0, int j = 0) : x(i), y(j) { }
    };
    std::vector<coord> coords;
    int highx = 0, highy = 0;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        coord p;
        char c = 0;
        iss >> p.x >> c >> p.y;
        coords.push_back(p);
        highx = std::max(highx, p.x);
        highy = std::max(highy, p.y);
    }
    ++highx, ++highy;

    auto manhattan = [](coord a, coord b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    };

    std::vector<std::vector<std::map<int, int>>> space;
    space.resize(highx);
    for (auto&& d : space) {
        d.resize(highy);
    }
    for (int i = 0; i < highx; ++i) {
        for (int j = 0; j < highy; ++j) {
            auto&& m = space[i][j];
            for (int k = 0; k < coords.size(); ++k) {
                m[k] = manhattan(coords[k], { i, j });
            }
        }
    }

    struct coverinfo {
        int num = 0;
        bool valid = true;
    };
    std::map<int, coverinfo> cover;
    for (int i = 0; i < highx; ++i) {
        for (int j = 0; j < highy; ++j) {
            auto&& m = space[i][j];
            auto seq = from(m)
                     | order_by([](auto&& p) { return p.second; });
            int closest_id = std::begin(seq)->first;
            int closest_dist = std::begin(seq)->second;
            if (std::next(std::begin(seq))->second > closest_dist) {
                ++cover[closest_id].num;
                if (i == 0 || i == (highx - 1) || j == 0 || j == (highy - 1)) {
                    cover[closest_id].valid = false;
                }
            }
        }
    }

    int largesize = from(cover)
                  | where([](auto&& p) { return p.second.valid; })
                  | order_by_descending([](auto&& p) { return p.second.num; })
                  | select([](auto&& p) { return p.second.num; })
                  | first();
    std::cout << largesize << std::endl;
}

void day6_2()
{
    struct coord {
        int x, y;
        coord(int i = 0, int j = 0) : x(i), y(j) { }
    };
    std::vector<coord> coords;
    int highx = 0, highy = 0;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        coord p;
        char c = 0;
        iss >> p.x >> c >> p.y;
        coords.push_back(p);
        highx = std::max(highx, p.x);
        highy = std::max(highy, p.y);
    }
    ++highx, ++highy;

    auto manhattan = [](coord a, coord b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    };

    int region_size = 0;
    for (int i = 0; i < highx; ++i) {
        for (int j = 0; j < highy; ++j) {
            int tot_dist = 0;
            for (auto&& c : coords) {
                tot_dist += manhattan(c, { i, j });
            }
            if (tot_dist < 10000) {
                ++region_size;
            }
        }
    }
    std::cout << region_size << std::endl;
}

void day7_1()
{
    std::map<char, std::set<char>> prereqs;
    for (char c = 'A'; c <= 'Z'; ++c) {
        prereqs.emplace(std::piecewise_construct,
                        std::make_tuple(c),
                        std::make_tuple());
    }
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        std::string word;
        char before = 0, after = 0;
        iss >> word >> before
            >> word >> word >> word >> word
            >> word >> after;
        prereqs[after].emplace(before);
    }

    std::string path;
    while (!prereqs.empty()) {
        for (auto&& p : prereqs) {
            if (p.second.empty()) {
                path.push_back(p.first);
                for (auto&& p2 : prereqs) {
                    p2.second.erase(path.back());
                }
                prereqs.erase(path.back());
                break;
            }
        }
    }
    std::cout << path << std::endl;
}

void day7_2()
{
    std::map<char, std::set<char>> prereqs;
    for (char c = 'A'; c <= 'Z'; ++c) {
        prereqs.emplace(std::piecewise_construct,
                        std::make_tuple(c),
                        std::make_tuple());
    }
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        std::string word;
        char before = 0, after = 0;
        iss >> word >> before
            >> word >> word >> word >> word
            >> word >> after;
        prereqs[after].emplace(before);
    }

    std::map<char, int> work;
    int sec = 0;
    for (;;) {
        for (auto it = work.begin(); it != work.end(); ) {
            if (--it->second == 0) {
                for (auto&& p : prereqs) {
                    p.second.erase(it->first);
                }
                prereqs.erase(it->first);
                it = work.erase(it);
            } else {
                ++it;
            }
        }
        if (prereqs.empty()) {
            break;
        }
        for (auto it = prereqs.begin(); it != prereqs.end() && work.size() < 5; ++it) {
            if (it->second.empty()) {
                work.emplace(std::piecewise_construct,
                             std::make_tuple(it->first),
                             std::make_tuple(60 + static_cast<int>(it->first - 'A' + 1)));
            }
        }
        ++sec;
    }
    std::cout << sec << std::endl;
}

void day8_1()
{
    struct node {
        std::vector<node> children;
        std::vector<int> meta;
        
        void load() {
            int nchildren = 0, nmeta = 0;
            std::cin >> nchildren >> nmeta;
            while (nchildren-- > 0) {
                children.emplace_back().load();
            }
            while (nmeta-- > 0) {
                std::cin >> meta.emplace_back();
            }
        }

        coveo::enumerable<const node> get_enum() const {
            coveo::enumerable<const node> e = from(coveo::enumerate_one_ref(*this));
            for (const node& c : children) {
                e = from(std::move(e))
                  | concat(c.get_enum());
            }
            return e;
        }
    };
    node root;
    root.load();
    int tot_meta = from(root.get_enum())
                 | select_many([](const node& n) { return n.meta; })
                 | sum([](int i) { return i; });
    std::cout << tot_meta << std::endl;
}

void day8_2()
{
    struct node {
        std::vector<node> children;
        std::vector<int> meta;
        
        void load() {
            int nchildren = 0, nmeta = 0;
            std::cin >> nchildren >> nmeta;
            while (nchildren-- > 0) {
                children.emplace_back().load();
            }
            while (nmeta-- > 0) {
                std::cin >> meta.emplace_back();
            }
        }

        int val() const {
            if (children.empty()) {
                return from(meta)
                     | sum([](int i) { return i; });
            } else {
                int v = 0;
                for (int m : meta) {
                    if (m >= 1 && m <= children.size()) {
                        v += children[m - 1].val();
                    }
                }
                return v;
            }
        }
    };
    node root;
    root.load();
    std::cout << root.val() << std::endl;
}

void day9_1()
{
    const int NUM_PLAYERS = 476;
    const int LAST_MARBLE = 71657;

    struct circle {
        std::list<int> stor;
        decltype(stor)::iterator it = stor.end();
        void insert(int marble) {
            it = stor.insert(it, marble);
        }
        int remove() {
            int marble = *it;
            it = stor.erase(it);
            if (it == stor.end()) {
                it = stor.begin();
            }
            return marble;
        }
        void move_clockwise(int move) {
            while (move-- > 0) {
                ++it;
                if (it == stor.end()) {
                    it = stor.begin();
                }
            }
        }
        void move_counterclockwise(int move) {
            while (move-- > 0) {
                if (it == stor.begin()) {
                    it = stor.end();
                }
                --it;
            }
        }
    };

    std::vector<int> score;
    score.resize(NUM_PLAYERS);
    int cur_player = 0;
    circle board;
    board.insert(0);
    for (int marble = 1; marble <= LAST_MARBLE; ++marble) {
        if (marble % 23 == 0) {
            score[cur_player] += marble;
            board.move_counterclockwise(7);
            score[cur_player] += board.remove();
        } else {
            board.move_clockwise(2);
            board.insert(marble);
        }
        if (++cur_player == NUM_PLAYERS) {
            cur_player = 0;
        }
    }

    int highscore = from(score)
                  | order_by_descending([](int s) { return s; })
                  | first();
    std::cout << highscore << std::endl;
}

void day9_2()
{
    const int64_t NUM_PLAYERS = 476;
    const int64_t LAST_MARBLE = 7165700;

    struct circle {
        std::list<int64_t> stor;
        decltype(stor)::iterator it = stor.end();
        void insert(int64_t marble) {
            it = stor.insert(it, marble);
        }
        int64_t remove() {
            int64_t marble = *it;
            it = stor.erase(it);
            if (it == stor.end()) {
                it = stor.begin();
            }
            return marble;
        }
        void move_clockwise(int move) {
            while (move-- > 0) {
                ++it;
                if (it == stor.end()) {
                    it = stor.begin();
                }
            }
        }
        void move_counterclockwise(int move) {
            while (move-- > 0) {
                if (it == stor.begin()) {
                    it = stor.end();
                }
                --it;
            }
        }
    };

    std::vector<int64_t> score;
    score.resize(NUM_PLAYERS);
    int cur_player = 0;
    circle board;
    board.insert(0);
    for (int64_t marble = 1; marble <= LAST_MARBLE; ++marble) {
        if (marble % 23 == 0) {
            score[cur_player] += marble;
            board.move_counterclockwise(7);
            score[cur_player] += board.remove();
        } else {
            board.move_clockwise(2);
            board.insert(marble);
        }
        if (++cur_player == NUM_PLAYERS) {
            cur_player = 0;
        }
    }

    int64_t highscore = from(score)
                      | order_by_descending([](int64_t s) { return s; })
                      | first();
    std::cout << highscore << std::endl;
}

void day10_1and2()
{
    struct light {
        int x = 0, y = 0, vx = 0, vy = 0;
        void move(int pow = 1) {
            x += (vx * pow);
            y += (vy * pow);
        }
    };
    std::vector<light> lights;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        light l;
        char c = 0;
        iss >> c >> c >> c >> c >> c >> c >> c >> c >> c >> c
            >> l.x >> c >> l.y >> c
            >> c >> c >> c >> c >> c >> c >> c >> c >> c >> c
            >> l.vx >> c >> l.vy;
        lights.push_back(l);
    }

    const int MAX_TIME_S = 100000;
    const int SKIP_S = 10490;
    const int CLOSENESS = 25;
    for (auto&& l : lights) {
        l.move(SKIP_S);
    }
    for (int sec = SKIP_S; sec < MAX_TIME_S; ++sec) {
        int close = 0;
        for (auto it1 = lights.begin(); close < CLOSENESS && it1 != lights.end(); ++it1) {
            for (auto it2 = std::next(it1); close < CLOSENESS && it2 != lights.end(); ++it2) {
                if ((std::abs(it1->x - it2->x) + std::abs(it1->y - it2->y)) == 1) {
                    ++close;
                }
            }
        }
        if (close >= CLOSENESS) {
            std::cout << sec << std::endl;
            int minx = from(lights)
                     | min([](auto&& l) { return l.x; });
            int maxx = from(lights)
                     | max([](auto&& l) { return l.x; });
            int miny = from(lights)
                     | min([](auto&& l) { return l.y; });
            int maxy = from(lights)
                     | max([](auto&& l) { return l.y; });
            for (int x = minx; x <= maxx; ++x) {
                for (int y = miny; y <= maxy; ++y) {
                    if (std::any_of(lights.begin(), lights.end(), [=](auto&& l) { return l.x == x && l.y == y; })) {
                        std::cout << '#';
                    } else {
                        std::cout << '.';
                    }
                }
                std::cout << std::endl;
            }
        } else if (sec % 1000 == 0) {
            std::cout << sec << std::endl;
        }
        for (auto&& l : lights) {
            l.move();
        }
    }
}

void day11_1()
{
    const int SERIAL = 2568;

    std::vector<std::vector<int>> grid;
    grid.resize(300);
    for (auto&& v : grid) {
        v.resize(300);
    }
    for (int x = 1; x <= 300; ++x) {
        for (int y = 1; y <= 300; ++y) {
            int& cell = grid[x - 1][y - 1];
            int rackid = x + 10;
            cell = rackid;
            cell *= y;
            cell += SERIAL;
            cell *= rackid;
            cell /= 100;
            cell %= 10;
            cell -= 5;
        }
    }

    int highx = 0, highy = 0, highpow = -1000;
    for (int x = 0; x < 298; ++x) {
        for (int y = 0; y < 298; ++y) {
            int pow = grid[x][y] +
                      grid[x + 1][y] +
                      grid[x + 2][y] +
                      grid[x][y + 1] +
                      grid[x + 1][y + 1] +
                      grid[x + 2][y + 1] +
                      grid[x][y + 2] +
                      grid[x + 1][y + 2] +
                      grid[x + 2][y + 2];
            if (pow > highpow) {
                highx = x + 1;
                highy = y + 1;
                highpow = pow;
            }
        }
    }
    std::cout << highx << "," << highy << " : " << highpow << std::endl;
}

void day11_2()
{
    const int SERIAL = 2568;

    std::vector<std::vector<int>> grid;
    grid.resize(300);
    for (auto&& v : grid) {
        v.resize(300);
    }
    for (int x = 1; x <= 300; ++x) {
        for (int y = 1; y <= 300; ++y) {
            int& cell = grid[x - 1][y - 1];
            int rackid = x + 10;
            cell = rackid;
            cell *= y;
            cell += SERIAL;
            cell *= rackid;
            cell /= 100;
            cell %= 10;
            cell -= 5;
        }
    }

    struct answer {
        int x = 0, y = 0, size = 0, pow = -1000;
    };
    auto find_answer = [&](int size) -> answer {
        answer a;
        a.size = size;
        for (int x = 0; x < (300 - size + 1); ++x) {
            for (int y = 0; y < (300 - size + 1); ++y) {
                int pow = 0;
                for (int i = x; i < x + size; ++i) {
                    for (int j = y; j < y + size; ++j) {
                        pow += grid[i][j];
                    }
                }
                if (pow > a.pow) {
                    a.x = x + 1;
                    a.y = y + 1;
                    a.pow = pow;
                }
            }
        }
        return a;
    };
    std::vector<std::future<answer>> futures;
    for (int i = 1; i <= 300; ++i) {
        futures.emplace_back(std::async(std::launch::async, find_answer, i));
    }
    answer higha = from(futures)
                 | select([](auto&& f) { return f.get(); })
                 | order_by_descending([](auto&& a) { return a.pow; })
                 | first();
    std::cout << higha.x << "," << higha.y << "," << higha.size << " : " << higha.pow << std::endl;
}

void day12_1()
{
    std::map<int, int> gen;
    for (int i = -52; i <= 202; ++i) {
        gen.emplace(i, 0);
    }
    {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string s;
        iss >> s >> s;
        for (int i = 0; !iss.eof(); ++i) {
            char c = 0;
            iss >> c;
            gen[i] = c == '#' ? 1 : 0;
        }

        std::getline(std::cin, line);
    }

    struct note {
        int minus2 = 0, minus1 = 0, nexus = 0, plus1 = 0, plus2 = 0;
        bool operator<(const note& right) const {
            int cmp = minus2 - right.minus2;
            if (cmp == 0) {
                cmp = minus1 - right.minus1;
                if (cmp == 0) {
                    cmp = nexus - right.nexus;
                    if (cmp == 0) {
                        cmp = plus1 - right.plus1;
                        if (cmp == 0) {
                            cmp = plus2 - right.plus2;
                        }
                    }
                }
            }
            return cmp < 0;
        }
    };
    std::set<note> notes;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        note n;
        char c = 0;
        std::string s;
        int* pval[5] = { &n.minus2, &n.minus1, &n.nexus, &n.plus1, &n.plus2 };
        for (int i = 0; i < 5; ++i) {
            iss >> c;
            *pval[i] = c == '#' ? 1 : 0;
        }
        iss >> s >> c;
        if (c == '#') {
            notes.emplace(n);
        }
    }

    for (int g = 0; g < 20; ++g) {
        auto nextgen = gen;
        for (int i = -50; i <= 200; ++i) {
            note n;
            n.minus2 = gen[i - 2];
            n.minus1 = gen[i - 1];
            n.nexus = gen[i];
            n.plus1 = gen[i + 1];
            n.plus2 = gen[i + 2];
            nextgen[i] = (notes.find(n) != notes.end()) ? 1 : 0;
        }
        gen = nextgen;
    }

    int sum_pots = from(gen)
                 | where([](auto&& p) { return p.second == 1; })
                 | select([](auto&& p) { return p.first; })
                 | sum([](int i) { return i; });
    std::cout << sum_pots << std::endl;
}

void day12_2()
{
    std::map<int64_t, int> gen;
    for (int64_t i = -1002; i <= 1002; ++i) {
        gen.emplace(i, 0);
    }
    {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string s;
        iss >> s >> s;
        for (int i = 0; !iss.eof(); ++i) {
            char c = 0;
            iss >> c;
            gen[i] = c == '#' ? 1 : 0;
        }

        std::getline(std::cin, line);
    }

    struct note {
        int minus2 = 0, minus1 = 0, nexus = 0, plus1 = 0, plus2 = 0;
        bool operator<(const note& right) const {
            int cmp = minus2 - right.minus2;
            if (cmp == 0) {
                cmp = minus1 - right.minus1;
                if (cmp == 0) {
                    cmp = nexus - right.nexus;
                    if (cmp == 0) {
                        cmp = plus1 - right.plus1;
                        if (cmp == 0) {
                            cmp = plus2 - right.plus2;
                        }
                    }
                }
            }
            return cmp < 0;
        }
    };
    std::set<note> notes;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        note n;
        char c = 0;
        std::string s;
        int* pval[5] = { &n.minus2, &n.minus1, &n.nexus, &n.plus1, &n.plus2 };
        for (int i = 0; i < 5; ++i) {
            iss >> c;
            *pval[i] = c == '#' ? 1 : 0;
        }
        iss >> s >> c;
        if (c == '#') {
            notes.emplace(n);
        }
    }

    for (int64_t g = 0; g < 100; ++g) {
        auto nextgen = gen;
        for (int64_t i = -1000; i <= 1000; ++i) {
            note n;
            n.minus2 = gen[i - 2];
            n.minus1 = gen[i - 1];
            n.nexus = gen[i];
            n.plus1 = gen[i + 1];
            n.plus2 = gen[i + 2];
            nextgen[i] = (notes.find(n) != notes.end()) ? 1 : 0;
        }
        gen = nextgen;
        //int64_t sum_pots = from(gen)
        //                 | where([](auto&& p) { return p.second == 1; })
        //                 | select([](auto&& p) { return p.first; })
        //                 | sum([](int64_t i) { return i; });
        //for (int64_t i = -200; i <= 200; ++i) {
        //    std::cout << (gen[i] == 1 ? '#' : '.');
        //}
        //std::cout << " -> " << g << " (" << sum_pots << ")" << std::endl;
    }

    int64_t sum_pots = from(gen)
                     | where([](auto&& p) { return p.second == 1; })
                     | select([](auto&& p) { return p.first; })
                     | sum([](int64_t i) { return i; });
    sum_pots += (50000000000 - 100) * 38;
    std::cout << sum_pots << std::endl;
}

void day13_1()
{
    enum class nextturn { left, straight, right, none };
    struct point {
        char track = ' ';
        char cart = 0;
        nextturn next = nextturn::none;
        bool moved = false;
    };
    std::vector<std::vector<point>> grid;
    grid.resize(150);
    for (auto&& g : grid) {
        g.resize(150);
    }
    for (int y = 0; y < 150; ++y) {
        std::string line;
        std::getline(std::cin, line);
        for (int x = 0; x < 150; ++x) {
            point p;
            char c = line[x];
            switch (c) {
                case ' ':
                case '-':
                case '|':
                case '/':
                case '\\':
                case '+': {
                    p.track = c;
                    break;
                }
                case 'v':
                case '^': {
                    p.track = '|';
                    p.cart = c;
                    p.next = nextturn::left;
                    break;
                }
                case '<':
                case '>': {
                    p.track = '-';
                    p.cart = c;
                    p.next = nextturn::left;
                    break;
                }
            }
            grid[x][y] = p;
        }
    }

    bool crashed = false;
    int crashx = 0, crashy = 0;
    for (int t = 0; !crashed; ++t) {
        for (auto&& g : grid) {
            for (auto&& p : g) {
                p.moved = false;
            }
        }
        for (int y = 0; !crashed && y < 150; ++y) {
            for (int x = 0; !crashed && x < 150; ++x) {
                point& rp = grid[x][y];
                if (rp.cart != 0 && !rp.moved) {
                    int destx = 0, desty = 0;
                    switch (rp.cart) {
                        case 'v': {
                            destx = x;
                            desty = y + 1;
                            break;
                        }
                        case '^': {
                            destx = x;
                            desty = y - 1;
                            break;
                        }
                        case '<': {
                            destx = x - 1;
                            desty = y;
                            break;
                        }
                        case '>': {
                            destx = x + 1;
                            desty = y;
                            break;
                        }
                    }
                    point* pdest = &grid[destx][desty];
                    if (pdest->cart != 0) {
                        crashed = true;
                        crashx = destx;
                        crashy = desty;
                        rp.cart = 0;
                        rp.next = nextturn::none;
                        pdest->track = 'X';
                    } else {
                        pdest->cart = rp.cart;
                        pdest->next = rp.next;
                        pdest->moved = true;
                        rp.cart = 0;
                        rp.next = nextturn::none;
                        switch (pdest->track) {
                            case '/': {
                                switch (pdest->cart) {
                                    case '^': {
                                        pdest->cart = '>';
                                        break;
                                    }
                                    case '>': {
                                        pdest->cart = '^';
                                        break;
                                    }
                                    case 'v': {
                                        pdest->cart = '<';
                                        break;
                                    }
                                    case '<': {
                                        pdest->cart = 'v';
                                        break;
                                    }
                                }
                                break;
                            }
                            case '\\': {
                                switch (pdest->cart) {
                                    case '^': {
                                        pdest->cart = '<';
                                        break;
                                    }
                                    case '<': {
                                        pdest->cart = '^';
                                        break;
                                    }
                                    case 'v': {
                                        pdest->cart = '>';
                                        break;
                                    }
                                    case '>': {
                                        pdest->cart = 'v';
                                        break;
                                    }
                                }
                                break;
                            }
                            case '+': {
                                switch (pdest->next) {
                                    case nextturn::left: {
                                        switch (pdest->cart) {
                                            case '<': {
                                                pdest->cart = 'v';
                                                break;
                                            }
                                            case 'v': {
                                                pdest->cart = '>';
                                                break;
                                            }
                                            case '>': {
                                                pdest->cart = '^';
                                                break;
                                            }
                                            case '^': {
                                                pdest->cart = '<';
                                                break;
                                            }
                                        }
                                        pdest->next = nextturn::straight;
                                        break;
                                    }
                                    case nextturn::straight: {
                                        pdest->next = nextturn::right;
                                        break;
                                    }
                                    case nextturn::right: {
                                        switch (pdest->cart) {
                                            case '<': {
                                                pdest->cart = '^';
                                                break;
                                            }
                                            case '^': {
                                                pdest->cart = '>';
                                                break;
                                            }
                                            case '>': {
                                                pdest->cart = 'v';
                                                break;
                                            }
                                            case 'v': {
                                                pdest->cart = '<';
                                                break;
                                            }
                                        }
                                        pdest->next = nextturn::left;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << crashx << ',' << crashy << std::endl;
}

void day13_2()
{
    enum class nextturn { left, straight, right, none };
    struct point {
        char track = ' ';
        char cart = 0;
        nextturn next = nextturn::none;
        bool moved = false;
    };
    std::vector<std::vector<point>> grid;
    grid.resize(150);
    for (auto&& g : grid) {
        g.resize(150);
    }
    for (int y = 0; y < 150; ++y) {
        std::string line;
        std::getline(std::cin, line);
        for (int x = 0; x < 150; ++x) {
            point p;
            char c = line[x];
            switch (c) {
                case ' ':
                case '-':
                case '|':
                case '/':
                case '\\':
                case '+': {
                    p.track = c;
                    break;
                }
                case 'v':
                case '^': {
                    p.track = '|';
                    p.cart = c;
                    p.next = nextturn::left;
                    break;
                }
                case '<':
                case '>': {
                    p.track = '-';
                    p.cart = c;
                    p.next = nextturn::left;
                    break;
                }
            }
            grid[x][y] = p;
        }
    }

    bool found_last = false;
    int lastx = 0, lasty = 0;
    for (int t = 0; !found_last; ++t) {
        int carts_left = 0;
        int potentiallastx = 0, potentiallasty = 0;
        for (auto&& g : grid) {
            for (auto&& p : g) {
                p.moved = false;
            }
        }
        for (int y = 0; !found_last && y < 150; ++y) {
            for (int x = 0; !found_last && x < 150; ++x) {
                point& rp = grid[x][y];
                if (rp.cart != 0 && !rp.moved) {
                    int destx = 0, desty = 0;
                    switch (rp.cart) {
                        case 'v': {
                            destx = x;
                            desty = y + 1;
                            break;
                        }
                        case '^': {
                            destx = x;
                            desty = y - 1;
                            break;
                        }
                        case '<': {
                            destx = x - 1;
                            desty = y;
                            break;
                        }
                        case '>': {
                            destx = x + 1;
                            desty = y;
                            break;
                        }
                    }
                    point* pdest = &grid[destx][desty];
                    if (pdest->cart != 0) {
                        rp.cart = 0;
                        rp.next = nextturn::none;
                        rp.moved = false;
                        pdest->cart = 0;
                        pdest->next = nextturn::none;
                        pdest->moved = false;
                    } else {
                        pdest->cart = rp.cart;
                        pdest->next = rp.next;
                        pdest->moved = true;
                        rp.cart = 0;
                        rp.next = nextturn::none;
                        ++carts_left;
                        potentiallastx = destx;
                        potentiallasty = desty;
                        switch (pdest->track) {
                            case '/': {
                                switch (pdest->cart) {
                                    case '^': {
                                        pdest->cart = '>';
                                        break;
                                    }
                                    case '>': {
                                        pdest->cart = '^';
                                        break;
                                    }
                                    case 'v': {
                                        pdest->cart = '<';
                                        break;
                                    }
                                    case '<': {
                                        pdest->cart = 'v';
                                        break;
                                    }
                                }
                                break;
                            }
                            case '\\': {
                                switch (pdest->cart) {
                                    case '^': {
                                        pdest->cart = '<';
                                        break;
                                    }
                                    case '<': {
                                        pdest->cart = '^';
                                        break;
                                    }
                                    case 'v': {
                                        pdest->cart = '>';
                                        break;
                                    }
                                    case '>': {
                                        pdest->cart = 'v';
                                        break;
                                    }
                                }
                                break;
                            }
                            case '+': {
                                switch (pdest->next) {
                                    case nextturn::left: {
                                        switch (pdest->cart) {
                                            case '<': {
                                                pdest->cart = 'v';
                                                break;
                                            }
                                            case 'v': {
                                                pdest->cart = '>';
                                                break;
                                            }
                                            case '>': {
                                                pdest->cart = '^';
                                                break;
                                            }
                                            case '^': {
                                                pdest->cart = '<';
                                                break;
                                            }
                                        }
                                        pdest->next = nextturn::straight;
                                        break;
                                    }
                                    case nextturn::straight: {
                                        pdest->next = nextturn::right;
                                        break;
                                    }
                                    case nextturn::right: {
                                        switch (pdest->cart) {
                                            case '<': {
                                                pdest->cart = '^';
                                                break;
                                            }
                                            case '^': {
                                                pdest->cart = '>';
                                                break;
                                            }
                                            case '>': {
                                                pdest->cart = 'v';
                                                break;
                                            }
                                            case 'v': {
                                                pdest->cart = '<';
                                                break;
                                            }
                                        }
                                        pdest->next = nextturn::left;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (carts_left == 1) {
            found_last = true;
            lastx = potentiallastx;
            lasty = potentiallasty;
        }
    }
    std::cout << lastx << ',' << lasty << std::endl;
}

void day14_1()
{
    const int NUM_RECIPES = 327901;

    std::list<int> board;
    board.push_back(3);
    board.push_back(7);
    auto cur1 = board.begin();
    auto cur2 = std::next(cur1);
    auto move = [&](std::list<int>::iterator& it, int move) {
        while (move-- > 0) {
            ++it;
            if (it == board.end()) {
                it = board.begin();
            }
        }
    };
    auto split = [](int i) {
        std::vector<int> digits;
        for (;;) {
            digits.push_back(i % 10);
            if (i == (i % 10)) {
                break;
            }
            i /= 10;
        }
        return from(std::move(digits))
             | reverse();
    };

    while (board.size() < NUM_RECIPES + 10) {
        int combined = *cur1 + *cur2;
        for (int i : split(combined)) {
            board.push_back(i);
        }
        move(cur1, 1 + *cur1);
        move(cur2, 1 + *cur2);
    }
    auto next_10 = from(board)
                 | skip(NUM_RECIPES)
                 | take(10);
    for (int i : next_10) {
        std::cout << i;
    }
    std::cout << std::endl;
}

void day14_2()
{
    const int NUM_RECIPES = 327901;

    std::list<int> board;
    board.push_back(3);
    board.push_back(7);
    auto cur1 = board.begin();
    auto cur2 = std::next(cur1);
    auto move = [&](std::list<int>::iterator& it, int move) {
        while (move-- > 0) {
            ++it;
            if (it == board.end()) {
                it = board.begin();
            }
        }
    };
    auto split = [](int i) {
        std::vector<int> digits;
        for (;;) {
            digits.push_back(i % 10);
            if (i == (i % 10)) {
                break;
            }
            i /= 10;
        }
        return from(std::move(digits))
             | reverse()
             | to_vector();
    };

    auto target = split(NUM_RECIPES);
    bool found = false;
    bool one_too_many = false;
    while (!found) {
        int combined = *cur1 + *cur2;
        for (int i : split(combined)) {
            board.push_back(i);
        }
        move(cur1, 1 + *cur1);
        move(cur2, 1 + *cur2);
        if (board.size() >= (target.size() + 1)) {
            found = std::equal(std::prev(board.end(), static_cast<int>(target.size() + 1)), std::prev(board.end()),
                               target.begin(), target.end());
            if (found) {
                one_too_many = true;
            }
        }
        if (!found && board.size() >= target.size()) {
            found = std::equal(std::prev(board.end(), static_cast<int>(target.size())), board.end(),
                               target.begin(), target.end());
        }
    }
    std::cout << (board.size() - target.size() - (one_too_many ? 1 : 0)) << std::endl;
}

void day15_1and2()
{
    struct tile : dij::node {
        int x = 0, y = 0;
        char ground = 0;
        char mob = 0;
        int hp = 0, atk = 0;
        bool moved = false;
    };
    struct graph : dij::graph {
        std::vector<std::vector<std::shared_ptr<tile>>> stor;
        graph() : stor() {
            stor.resize(32);
            for (auto&& s : stor) {
                s.resize(32);
            }
        }
        graph(const graph& g) : stor() {
            stor.resize(32);
            for (auto&& s : stor) {
                s.resize(32);
            }
            for (int x = 0; x < 32; ++x) {
                for (int y = 0; y < 32; ++y) {
                    add(*g.get(x, y));
                }
            }
        }
        graph(graph&&) = default;
        graph& operator=(const graph&) = delete;
        void add(tile t) {
            stor[t.x][t.y] = std::make_shared<tile>(t);
        }
        tile* get(int x, int y) const {
            return stor[x][y].get();
        }
        std::vector<tile*> all_tiles() const {
            return from(stor)
                 | select_many([](auto&& s) { return s; })
                 | select([](auto&& spt) -> tile* { return spt.get(); })
                 | to_vector();
        }
        std::vector<tile*> all_neighbours(tile* pt) const {
            std::vector<tile*> neighbourhood;
            for (int ymod : { -1, 0, 1 }) {
                for (int xmod : { -1, 0, 1 }) {
                    if (xmod == 0 || ymod == 0) {
                        int x = pt->x + xmod;
                        int y = pt->y + ymod;
                        if (x >= 0 && x < 32 && y >= 0 && y < 32) {
                            neighbourhood.push_back(stor[x][y].get());
                        }
                    }
                }
            }
            return neighbourhood;
        }
        virtual std::vector<dij::node*> all_passable() const override {
            return from(stor)
                 | select_many([](auto&& s) { return s; })
                 | where([](auto&& spt) { return spt->ground == '.' && spt->mob == 0; })
                 | select([](auto&& spt) -> dij::node* { return spt.get(); })
                 | to_vector();
        }
        virtual std::vector<dij::node*> neighbours(dij::node* pn, dij::node* pinclude) const override {
            tile* pt = dynamic_cast<tile*>(pn);
            tile* ptinclude = dynamic_cast<tile*>(pinclude);
            auto tile_valid = [&](tile* pv) {
                return (pv->ground == '.' && pv->mob == 0) || pv == ptinclude;
            };
            return from(all_neighbours(pt))
                 | where([&](tile* ptt) { return tile_valid(ptt); })
                 | select([](tile* ptt) -> dij::node* { return ptt; })
                 | to_vector();
        }
        virtual int64_t dist(dij::node*, dij::node*) const override {
            return 1;
        }
        virtual bool is_a_better(dij::node* pa, dij::node* pb) const override {
            tile* pta = dynamic_cast<tile*>(pa);
            tile* ptb = dynamic_cast<tile*>(pb);
            return pta->y < ptb->y || (pta->y == ptb->y && pta->x < ptb->x);
        }
    } orig_grid;
    for (int y = 0; y < 32; ++y) {
        std::string line;
        std::getline(std::cin, line);
        for (int x = 0; x < 32; ++x) {
            tile t;
            t.x = x;
            t.y = y;
            switch (line[x]) {
                case '#':
                case '.': {
                    t.ground = line[x];
                    break;
                }
                case 'G':
                case 'E': {
                    t.ground = '.';
                    t.mob = line[x];
                    t.hp = 200;
                    t.atk = 3;
                    break;
                }
            }
            orig_grid.add(t);
        }
    }

    auto dump_grid = [](const graph& g) {
        for (int y = 0; y < 32; ++y) {
            std::vector<tile*> mobs;
            for (int x = 0; x < 32; ++x) {
                tile* pt = g.get(x, y);
                if (pt->mob != 0) {
                    std::cout << pt->mob;
                    mobs.push_back(pt);
                } else {
                    std::cout << pt->ground;
                }
            }
            for (tile* pm : mobs) {
                std::cout << " (" << pm->x << "," << pm->y << ")_" << pm->hp;
            }
            std::cout << std::endl;
        }
    };
    auto has_enemies = [](const graph& g, tile* pt) {
        const char tolookfor = pt->mob == 'G' ? 'E' : 'G';
        return from(g.all_tiles())
             | where([=](tile* ptt) { return ptt->mob == tolookfor; })
             | any();
    };
    auto get_enemies_neighbours = [](const graph& g, tile* pt) {
        const char tolookfor = pt->mob == 'G' ? 'E' : 'G';
        return from(g.all_tiles())
             | where([=](tile* ptt) { return ptt->mob == tolookfor; })
             | select_many([&](tile* ptt) { return g.neighbours(ptt, pt); })
             | select([](dij::node* pn) { return dynamic_cast<tile*>(pn); })
             | order_by([](tile* ptt) { return ptt->y; })
             | then_by([](tile* ptt) { return ptt->x; })
             | to_vector();
    };
    auto get_attack_target = [](const graph& g, tile* pt) {
        char tolookfor = pt->mob == 'G' ? 'E' : 'G';
        return from(g.all_neighbours(pt))
             | where([=](tile* ptt) { return ptt->mob == tolookfor; })
             | order_by([](tile* ptt) { return ptt->hp; })
             | then_by([](tile* ptt) { return ptt->y; })
             | then_by([](tile* ptt) { return ptt->x; })
             | first_or_default();
    };
    
    auto run_combat = [&](int elf_atk) -> std::tuple<graph, int, int, std::map<char, int>> {
        graph grid(orig_grid);
        for (tile* pt : grid.all_tiles()) {
            if (pt->mob == 'E') {
                pt->atk = elf_atk;
            }
        }

        int turn = 0;
        bool done = false;
        std::map<char, int> casualties;
        while (!done) {
            for (tile* pt : grid.all_tiles()) {
                pt->moved = false;
            }
            for (int y = 0; !done && y < 32; ++y) {
                for (int x = 0; !done && x < 32; ++x) {
                    tile* pt = grid.get(x, y);
                    if (pt->mob != 0 && !pt->moved) {
                        // move
                        if (!has_enemies(grid, pt)) {
                            done = true;
                            break;
                        }
                        auto target_tiles = get_enemies_neighbours(grid, pt);
                        std::unordered_map<tile*, std::tuple<tile*, int64_t>> moves;
                        if (!target_tiles.empty()) {
                            std::unordered_map<dij::node*, int64_t> dij_dist;
                            std::unordered_map<dij::node*, dij::node*> dij_prev;
                            std::tie(dij_dist, dij_prev) = dij::get_dijkstra(&grid, pt);
                            for (tile* pe : target_tiles) {
                                auto distit = dij_dist.find(pe);
                                if (distit != dij_dist.end() && distit->second != std::numeric_limits<int64_t>::max()) {
                                    auto path = dij::assemble_path(dij_prev, pt, pe);
                                    tile* pnext = !path.empty() ? dynamic_cast<tile*>(path.front()) : nullptr;
                                    moves[pe] = std::make_tuple(pnext, distit->second);
                                }
                            }
                        }
                        auto closest = from(moves)
                                     | order_by([](auto&& r) { return std::get<1>(r.second); })
                                     | then_by([](auto&& r) { return r.first->y; })
                                     | then_by([](auto&& r) { return r.first->x; })
                                     | then_by([](auto&& r) { return std::get<0>(r.second)->y; })
                                     | then_by([](auto&& r) { return std::get<0>(r.second)->x; })
                                     | first_or_default();
                        int64_t closest_dist = std::get<1>(closest.second);
                        if (closest.first != nullptr && closest_dist > 0) {
                            tile* pnext = std::get<0>(closest.second);
                            pnext->mob = pt->mob;
                            pnext->hp = pt->hp;
                            pnext->atk = pt->atk;
                            pnext->moved = true;
                            pt->mob = 0;
                            pt->hp = 0;
                            pt->atk = 0;
                            pt->moved = false;
                            pt = pnext;
                        } else {
                            pt->moved = true;
                        }

                        // attack
                        tile* ptarget = get_attack_target(grid, pt);
                        if (ptarget != nullptr) {
                            ptarget->hp -= pt->atk;
                            if (ptarget->hp <= 0) {
                                ++casualties[ptarget->mob];
                                ptarget->mob = 0;
                                ptarget->hp = 0;
                                ptarget->atk = 0;
                                ptarget->moved = false;
                            }
                        }
                    }
                }
            }
            if (!done) {
                ++turn;
                //if (turn >= 18) {
                //    std::cout << "After turn " << turn << ":" << std::endl;
                //    dump_grid();
                //    std::string line;
                //    std::getline(std::cin, line);
                //}
            }
        }

        int remain_hp = from(grid.all_tiles())
                      | where([](tile* pt) { return pt->mob != 0; })
                      | default_if_empty()
                      | sum([](tile* pt) { return pt != nullptr ? pt->hp : 0; });
        return std::make_tuple(std::move(grid), turn, remain_hp, std::move(casualties));
    };

    std::cout << "First puzzle" << std::endl;
    auto first_outcome = run_combat(3);
    std::cout << "End of combat - " << std::get<1>(first_outcome) << " full rounds." << std::endl;
    dump_grid(std::get<0>(first_outcome));
    std::cout << "Combat outcome: " << std::get<1>(first_outcome) * std::get<2>(first_outcome) << std::endl << std::endl;

    std::cout << "Second puzzle" << std::endl;
    for (int elf_atk = 4; ; ++elf_atk) {
        auto outcome = run_combat(elf_atk);
        std::cout << "End of combat with elf attack power " << elf_atk << " - " << std::get<1>(outcome) << " full rounds." << std::endl;
        if (std::get<3>(outcome)['E'] == 0) {
            std::cout << "No elf casualties!" << std::endl;
            dump_grid(std::get<0>(outcome));
            std::cout << "Combat outcome: " << std::get<1>(outcome) * std::get<2>(outcome) << std::endl << std::endl;
            break;
        } else {
            std::cout << "Elf casualties: " << std::get<3>(outcome)['E'] << " - not good." << std::endl;
        }
    }
}

void day16_1()
{
    struct registers {
        int r[4] = { 0 };
        bool operator==(const registers& right) const {
            return std::equal(std::begin(r), std::end(r), std::begin(right.r), std::end(right.r));
        }
    };
    struct opcode {
        std::string name;
        std::function<void(registers&, int, int, int)> impl;
    };
    std::vector<opcode> opcodes = {
        {
            "addr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + regs.r[b];
            }
        },
        {
            "addi",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + b;
            }
        },
        {
            "mulr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * regs.r[b];
            }
        },
        {
            "muli",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * b;
            }
        },
        {
            "banr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & regs.r[b];
            }
        },
        {
            "bani",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & b;
            }
        },
        {
            "borr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | regs.r[b];
            }
        },
        {
            "bori",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | b;
            }
        },
        {
            "setr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a];
            }
        },
        {
            "seti",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = a;
            }
        },
        {
            "gtir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a > regs.r[b] ? 1 : 0);
            }
        },
        {
            "gtri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > b ? 1 : 0);
            }
        },
        {
            "gtrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a == regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == b ? 1 : 0);
            }
        },
        {
            "eqrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == regs.r[b] ? 1 : 0);
            }
        },
    };

    int morethan3 = 0;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream issb(line);
        registers before;
        std::string word;
        char ch = 0;
        issb >> word >> ch
             >> before.r[0] >> ch
             >> before.r[1] >> ch
             >> before.r[2] >> ch
             >> before.r[3];

        std::getline(std::cin, line);
        std::istringstream isso(line);
        int op = 0, a = 0, b = 0, c = 0;
        isso >> op >> a >> b >> c;

        std::getline(std::cin, line);
        std::istringstream issa(line);
        registers after;
        issa >> word >> ch
             >> after.r[0] >> ch
             >> after.r[1] >> ch
             >> after.r[2] >> ch
             >> after.r[3];

        std::getline(std::cin, line);

        std::set<std::string> newguesses;
        for (auto&& guess : opcodes) {
            registers during(before);
            guess.impl(during, a, b, c);
            if (during == after) {
                newguesses.insert(guess.name);
            }
        }
        if (newguesses.size() >= 3) {
            ++morethan3;
        }
    }
    std::cout << morethan3 << std::endl;
}

void day16_2()
{
    struct registers {
        int r[4] = { 0 };
        bool operator==(const registers& right) const {
            return std::equal(std::begin(r), std::end(r), std::begin(right.r), std::end(right.r));
        }
    };
    struct opcode {
        std::string name;
        std::function<void(registers&, int, int, int)> impl;
    };
    std::vector<opcode> opcodes = {
        {
            "addr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + regs.r[b];
            }
        },
        {
            "addi",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + b;
            }
        },
        {
            "mulr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * regs.r[b];
            }
        },
        {
            "muli",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * b;
            }
        },
        {
            "banr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & regs.r[b];
            }
        },
        {
            "bani",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & b;
            }
        },
        {
            "borr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | regs.r[b];
            }
        },
        {
            "bori",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | b;
            }
        },
        {
            "setr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a];
            }
        },
        {
            "seti",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = a;
            }
        },
        {
            "gtir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a > regs.r[b] ? 1 : 0);
            }
        },
        {
            "gtri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > b ? 1 : 0);
            }
        },
        {
            "gtrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a == regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == b ? 1 : 0);
            }
        },
        {
            "eqrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == regs.r[b] ? 1 : 0);
            }
        },
    };

    std::map<int, std::set<std::string>> guesses;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) {
            break;
        }
        std::istringstream issb(line);
        registers before;
        std::string word;
        char ch = 0;
        issb >> word >> ch
             >> before.r[0] >> ch
             >> before.r[1] >> ch
             >> before.r[2] >> ch
             >> before.r[3];

        std::getline(std::cin, line);
        std::istringstream isso(line);
        int op = 0, a = 0, b = 0, c = 0;
        isso >> op >> a >> b >> c;

        std::getline(std::cin, line);
        std::istringstream issa(line);
        registers after;
        issa >> word >> ch
             >> after.r[0] >> ch
             >> after.r[1] >> ch
             >> after.r[2] >> ch
             >> after.r[3];

        std::getline(std::cin, line);

        std::set<std::string> newguesses;
        for (auto&& guess : opcodes) {
            registers during(before);
            guess.impl(during, a, b, c);
            if (during == after) {
                newguesses.insert(guess.name);
            }
        }
        auto it = guesses.find(op);
        if (it == guesses.end()) {
            guesses[op] = newguesses;
        } else {
            it->second = from(it->second)
                       | intersect(newguesses)
                       | to<std::set<std::string>>();
        }
    }

    std::map<int, opcode*> instructions_map;
    while (!guesses.empty()) {
        std::set<std::string> toremove;
        for (auto it = guesses.begin(); it != guesses.end(); ) {
            if (it->second.size() == 1) {
                opcode& rop = from(opcodes)
                            | single([&](auto&& o) { return o.name == *it->second.begin(); });
                instructions_map[it->first] = &rop;
                toremove.insert(*it->second.begin());
                it = guesses.erase(it);
            } else {
                ++it;
            }
        }
        for (auto&& g : guesses) {
            g.second = from(g.second)
                     | except(toremove)
                     | to<std::set<std::string>>();
        }
    }

    std::string emptyline;
    std::getline(std::cin, emptyline);
    registers cpu;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) {
            break;
        }
        std::istringstream iss(line);
        int op = 0, a = 0, b = 0, c = 0;
        iss >> op >> a >> b >> c;
        instructions_map[op]->impl(cpu, a, b, c);
    }
    for (int i = 0; i < 4; ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << "R" << i << " = " << cpu.r[i];
    }
    std::cout << std::endl;
}

void day17_1and2()
{
    struct spot {
        int x = 0, y = 0;
        bool operator<(const spot& right) const {
            int cmp = x - right.x;
            if (cmp == 0) {
                cmp = y - right.y;
            }
            return cmp < 0;
        }
    };
    std::vector<spot> spots;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        char coord = 0, c = 0;
        int singleval = 0, rbeg = 0, rend = 0;
        iss >> coord >> c >> singleval >> c
            >> c >> c >> rbeg >> c >> c >> rend;
        if (coord == 'x') {
            for (int i = rbeg; i <= rend; ++i) {
                spots.push_back({ singleval, i });
            }
        } else {
            for (int i = rbeg; i <= rend; ++i) {
                spots.push_back({ i, singleval });
            }
        }
    }
    auto xs = from(spots)
            | select([](auto&& s) { return s.x; })
            | order_by([](int i) { return i; });
    int maxx = from(xs)
             | last();
    auto ys = from(spots)
            | select([](auto&& s) { return s.y; })
            | order_by([](int i) { return i; });
    int miny = from(ys)
             | first();
    int maxy = from(ys)
             | last();

    std::vector<std::vector<char>> terrain;
    terrain.resize(maxx + 2);
    for (auto&& ty : terrain) {
        ty.resize(maxy + 2);
        for (char& c : ty) {
            c = '.';
        }
    }
    for (auto&& s : spots) {
        terrain[s.x][s.y] = '#';
    }
    terrain[500][0] = '+';
    auto dump_terrain = [&]() {
        std::cout << "maxx = " << maxx << ", miny = " << miny << ", maxy = " << maxy << std::endl
                  << std::endl;
        for (int y = 0; y <= maxy; ++y) {
            for (int x = 0; x <= maxx; ++x) {
                std::cout << terrain[x][y];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    };

    //dump_terrain();

    std::queue<spot> startingpoints;
    startingpoints.push({ 500, 0 });
    std::set<spot> seenpoints;
    auto passable = [&](int x, int y) {
        return terrain[x][y] == '.' || terrain[x][y] == '|';
    };
    auto blocked = [&](int x, int y) {
        return terrain[x][y] != '.' && terrain[x][y] != '|';
    };
    while (!startingpoints.empty()) {
        spot curstart = startingpoints.front();
        int x = curstart.x, y = curstart.y;
        if (terrain[x][y] == '~') {
            startingpoints.push({ x, y - 1 });
            startingpoints.pop();
            continue;
        }
        while (y <= maxy && passable(x, y + 1)) {
            if (terrain[x][y] == '.') {
                terrain[x][y] = '|';
            }
            ++y;
        }
        if (y > maxy) {
            startingpoints.pop();
            continue;
        }
        int leftx = x, rightx = x;
        while (blocked(leftx, y + 1) && passable(leftx - 1, y)) {
            if (terrain[leftx][y] == '.') {
                terrain[leftx][y] = '|';
            }
            --leftx;
            if (terrain[leftx][y] == '.') {
                terrain[leftx][y] = '|';
            }
        }
        while (blocked(rightx, y + 1) && passable(rightx + 1, y)) {
            if (terrain[rightx][y] == '.') {
                terrain[rightx][y] = '|';
            }
            ++rightx;
            if (terrain[rightx][y] == '.') {
                terrain[rightx][y] = '|';
            }
        }
        if (passable(leftx, y + 1)) {
            spot toadd{ leftx, y };
            if (seenpoints.insert(toadd).second) {
                startingpoints.push(toadd);
            }
        }
        if (passable(rightx, y + 1)) {
            spot toadd{ rightx, y };
            if (seenpoints.insert(toadd).second) {
                startingpoints.push(toadd);
            }
        }
        if (blocked(leftx, y + 1) && blocked(rightx, y + 1)) {
            if ((x - leftx) < (rightx - x)) {
                x = rightx;
            } else {
                x = leftx;
            }
            terrain[x][y] = '~';
        } else {
            startingpoints.pop();
        }
    }
    int reachable = 0, water = 0;
    for (int x = 0; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            if (terrain[x][y] == '~' || terrain[x][y] == '|') {
                ++reachable;
            }
            if (terrain[x][y] == '~') {
                ++water;
            }
        }
    }
    std::cout << "Reachable: " << reachable << std::endl
              << "Water: " << water << std::endl
              << std::endl;

    //dump_terrain();
}

void day18_1()
{
    std::vector<std::vector<char>> terrain;
    terrain.resize(50);
    for (auto&& t : terrain) {
        t.resize(50);
    }
    for (int y = 0; y < 50; ++y) {
        std::string line;
        std::getline(std::cin, line);
        for (int x = 0; x < 50; ++x) {
            terrain[x][y] = line[x];
        }
    }

    auto dump_terrain = [&]() {
        for (int y = 0; y < 50; ++y) {
            for (int x = 0; x < 50; ++x) {
                std::cout << terrain[x][y];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    };

    auto adjescent = [&](int x, int y) -> coveo::enumerable<const char> {
        std::vector<char> v;
        if (y > 0) {
            if (x > 0) {
                v.push_back(terrain[x - 1][y - 1]);
            }
            v.push_back(terrain[x][y - 1]);
            if (x < 49) {
                v.push_back(terrain[x + 1][y - 1]);
            }
        }
        if (x > 0) {
            v.push_back(terrain[x - 1][y]);
        }
        if (x < 49) {
            v.push_back(terrain[x + 1][y]);
        }
        if (y < 49) {
            if (x > 0) {
                v.push_back(terrain[x - 1][y + 1]);
            }
            v.push_back(terrain[x][y + 1]);
            if (x < 49) {
                v.push_back(terrain[x + 1][y + 1]);
            }
        }
        return coveo::enumerate_container(std::move(v));
    };
    std::map<char, std::function<char(int, int)>> next_turn = {
        {
            '.',
            [&](int x, int y) {
                auto num_trees = from(adjescent(x, y))
                               | count([](char c) { return c == '|'; });
                return num_trees >= 3 ? '|' : '.';
            }
        },
        {
            '|',
            [&](int x, int y) {
                auto num_ly = from(adjescent(x, y))
                            | count([](char c) { return c == '#'; });
                return num_ly >= 3 ? '#' : '|';
            }
        },
        {
            '#',
            [&](int x, int y) {
                auto adj = adjescent(x, y);
                auto num_ly = from(adj)
                            | count([](char c) { return c == '#'; });
                auto num_trees = from(adj)
                               | count([](char c) { return c == '|'; });
                return (num_ly >= 1 && num_trees >= 1) ? '#' : '.';
            }
        },
    };

    for (uint64_t minute = 0; minute < 10; ++minute) {
        auto newterrain(terrain);
        for (int y = 0; y < 50; ++y) {
            for (int x = 0; x < 50; ++x) {
                newterrain[x][y] = next_turn[terrain[x][y]](x, y);
            }
        }
        terrain = newterrain;
        //dump_terrain();
    }
    auto acres = from(terrain)
               | select_many([](auto&& t) { return t; });
    auto num_trees = from(acres)
                   | count([](char c) { return c == '|'; });
    auto num_ly = from(acres)
                | count([](char c) { return c == '#'; });
    std::cout << num_trees * num_ly << std::endl;
}

void day18_2()
{
    std::vector<std::vector<char>> terrain;
    terrain.resize(50);
    for (auto&& t : terrain) {
        t.resize(50);
    }
    for (int y = 0; y < 50; ++y) {
        std::string line;
        std::getline(std::cin, line);
        for (int x = 0; x < 50; ++x) {
            terrain[x][y] = line[x];
        }
    }

    auto dump_terrain = [&]() {
        for (int y = 0; y < 50; ++y) {
            for (int x = 0; x < 50; ++x) {
                std::cout << terrain[x][y];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    };

    auto adjescent = [&](int x, int y) -> coveo::enumerable<const char> {
        std::vector<char> v;
        if (y > 0) {
            if (x > 0) {
                v.push_back(terrain[x - 1][y - 1]);
            }
            v.push_back(terrain[x][y - 1]);
            if (x < 49) {
                v.push_back(terrain[x + 1][y - 1]);
            }
        }
        if (x > 0) {
            v.push_back(terrain[x - 1][y]);
        }
        if (x < 49) {
            v.push_back(terrain[x + 1][y]);
        }
        if (y < 49) {
            if (x > 0) {
                v.push_back(terrain[x - 1][y + 1]);
            }
            v.push_back(terrain[x][y + 1]);
            if (x < 49) {
                v.push_back(terrain[x + 1][y + 1]);
            }
        }
        return coveo::enumerate_container(std::move(v));
    };
    std::map<char, std::function<char(int, int)>> next_turn = {
        {
            '.',
            [&](int x, int y) {
                auto num_trees = from(adjescent(x, y))
                               | count([](char c) { return c == '|'; });
                return num_trees >= 3 ? '|' : '.';
            }
        },
        {
            '|',
            [&](int x, int y) {
                auto num_ly = from(adjescent(x, y))
                            | count([](char c) { return c == '#'; });
                return num_ly >= 3 ? '#' : '|';
            }
        },
        {
            '#',
            [&](int x, int y) {
                auto adj = adjescent(x, y);
                auto num_ly = from(adj)
                            | count([](char c) { return c == '#'; });
                auto num_trees = from(adj)
                               | count([](char c) { return c == '|'; });
                return (num_ly >= 1 && num_trees >= 1) ? '#' : '.';
            }
        },
    };

    std::map<std::vector<std::vector<char>>, uint64_t> seen;
    uint64_t loopbeg = 0, loopdur = 0;
    for (uint64_t minute = 0; minute < 1000000000; ++minute) {
        auto newterrain(terrain);
        for (int y = 0; y < 50; ++y) {
            for (int x = 0; x < 50; ++x) {
                newterrain[x][y] = next_turn[terrain[x][y]](x, y);
            }
        }
        terrain = newterrain;
        auto it = seen.find(terrain);
        if (it == seen.end()) {
            seen[terrain] = minute;
        } else {
            std::cout << "Minute " << minute << " is like minute " << it->second << std::endl;
            loopbeg = it->second;
            loopdur = minute - loopbeg;
            break;
        }
    }
    uint64_t posinloop = ((1000000000 - 1) - loopbeg) % loopdur;
    std::cout << "Position in loop: " << posinloop << std::endl;
    auto finalterrain = from(seen)
                      | where([&](auto&& p) { return p.second == loopbeg + posinloop; })
                      | select([](auto&& p) { return p.first; })
                      | single();
    auto acres = from(finalterrain)
               | select_many([](auto&& t) { return t; });
    auto num_trees = from(acres)
                   | count([](char c) { return c == '|'; });
    auto num_ly = from(acres)
                | count([](char c) { return c == '#'; });
    std::cout << num_trees * num_ly << std::endl;
}

void day19_1and2()
{
    struct registers {
        int r[6] = { 0 };
        std::string dump() const {
            std::ostringstream oss;
            oss << "[";
            for (int i = 0; i < 6; ++i) {
                if (i != 0) {
                    oss << ", ";
                }
                oss << r[i];
            }
            oss << "]";
            return oss.str();
        }
    };
    using opcode_impl = std::function<void(registers&, int, int, int)>;
    std::map<std::string, opcode_impl> opcodes = {
        {
            "addr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + regs.r[b];
            }
        },
        {
            "addi",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] + b;
            }
        },
        {
            "mulr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * regs.r[b];
            }
        },
        {
            "muli",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] * b;
            }
        },
        {
            "banr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & regs.r[b];
            }
        },
        {
            "bani",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] & b;
            }
        },
        {
            "borr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | regs.r[b];
            }
        },
        {
            "bori",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a] | b;
            }
        },
        {
            "setr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = regs.r[a];
            }
        },
        {
            "seti",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = a;
            }
        },
        {
            "gtir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a > regs.r[b] ? 1 : 0);
            }
        },
        {
            "gtri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > b ? 1 : 0);
            }
        },
        {
            "gtrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] > regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqir",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (a == regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqri",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == b ? 1 : 0);
            }
        },
        {
            "eqrr",
            [](registers& regs, int a, int b, int c) {
                regs.r[c] = (regs.r[a] == regs.r[b] ? 1 : 0);
            }
        },
    };
    struct instruction {
        std::string opname;
        const opcode_impl* op = nullptr;
        int a = 0, b = 0, c = 0;
    };

    registers cpu;
    int* ip = nullptr;
    std::vector<instruction> program;
    {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        int n = 0;
        char c = 0;
        iss >> c >> c >> c >> n;
        ip = &cpu.r[n];
    }
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        instruction inst;
        iss >> inst.opname >> inst.a >> inst.b >> inst.c;
        inst.op = &opcodes[inst.opname];
        program.emplace_back(inst);
    }

    while (*ip >= 0 && *ip < program.size()) {
        const instruction& inst = program[*ip];
        (*inst.op)(cpu, inst.a, inst.b, inst.c);
        ++*ip;
    }
    std::cout << "Program #1, R0 = " << cpu.r[0] << std::endl;

    // If we had forever:
    //for (int& r : cpu.r) {
    //    r = 0;
    //}
    //cpu.r[0] = 1;
    //*ip = 0;
    //while (*ip >= 0 && *ip < program.size()) {
    //    const instruction& inst = program[*ip];
    //    std::cout << "(" << *ip << ") " << cpu.dump() << " "
    //                << inst.opname << " " << inst.a << " " << inst.b << " " << inst.c << " ";
    //    (*inst.op)(cpu, inst.a, inst.b, inst.c);
    //    std::cout << cpu.dump() << std::endl;
    //    ++*ip;
    //}
    //std::cout << "Program #2, R0 = " << cpu.r[0] << std::endl;

    // If we cheat:
    const int NUMBER = 10551370;
    int total = 0;
    for (int i = 1; i <= NUMBER; ++i) {
        if (NUMBER % i == 0) {
            total += i;
        }
    }
    std::cout << "Program #2, R0 = " << total << std::endl;
}

void day20_1and2()
{
    const std::string INPUT = "^ESWSSSSSWSWWNNNENNWWNWWSWNNNESEENEESE(NENWNEENNWNEEESESWSESENEENNNNWWW(NNWSSWNWSWNWWSESSS(E|WWNENWWSWWWNWNWSSESESS(WWN(WN(WSWNWWWWWNNWSSWNNNWSSSWWSESWSSSENNESESWSSSWWNWWSWSEE(SSSSWNWNN(ESNW|)WNNNNNEEE(NNWSWNNEENWNENNWWNWSWWSWNWWWSSSESENN(NWSNES|)EEENEE(E|SWS(WSESWWSWWN(ENENWESWSW|)WSSEEEE(NEWS|)SWWWSEEESSSSESSWSSWSWNWNEENWWNWWSSWNWSSWSEEEE(ESWSWSEENESSESENNWNN(ESEEENENWWW(SEWN|)(NNE(N|SEEN(W|EENEESENNESSSSESSSWSEENNENWNENWWNEENESSENENNESSEESESEEEEESSENEENENWWSWNWWWWNNWNNWNNWSSW(WWNWSWNNNENNWWNENNWWWSWNW(SSESEE(SWSESWSEE(NNNEWSSS|)SS(EESNWW|)WSWNN(E|WWWNN(EE(SWEN|)NNWSWNNE(WSSENEWSWNNE|)|WWWSSES(WSNE|)E(NNWESS|)E))|N(NEWS|)W)|NEN(WWS(WNSE|)S|ESENESE(N|ESESS(WNSE|)SENNN(NWES|)EESWSESSS(EEESWSESSE(EESWWSEEENEN(W|EN(W|NNENESSWSSSWS(W|EENESENNESSSENEEEEESESENNENEENNESESENESSSSWWWNN(ESENSWNW|)(WWSWSSSWWN(WWN(NWSSSESSEEESWWWWNWWNNWWSESSSWNNWNNWNW(SSESSSSE(NN|SWSSSESESESWWNWSSSESENN(W|ESESEENEENNWNENNEESSSSEENWNNNENNWWS(WWWWSESWSSS(E|W(NW(S|NENWWNWNENW(WSNE|)NEEN(NE(NWES|)SEESWS(W(SSWNNSSENN|)N|EEENNEES(ENENESENESESEENWNEEEEESENNNNEEESWSEEEEESSEEESEEESEESEEESSSWNWWN(WWSSE(SEE(NWES|)ESESSSWWSSSENNEENNNNN(W|NNNNNWWWNNNWSSWNNNENWNNWWWSEESWWSS(WNNNWWS(ESSWNSENNW|)WNWWW(SEEWWN|)NENNESENNESESE(NNWNNEENNNNWSSWS(WWNNE(ENNNNNWWWSWSSENENESSWS(E|WWSESWSSWSEE(EENWWN|SWWSWNWWWNEENNWNNNNNNESSSSEE(NNNW(NNWWNNWWNNWWNEENWNWWS(SWNWSSWNWNNE(S|NENNESS(SWEN|)ENENWNWWNWSSSWSWWWNWSWNNNWSWWNWSSWWW(SSSSESWSSWW(NEWS|)WSWSWSW(NNEN(WWNE(E|NNN)|E)|S(W(N|S)|EENESSWSSS(SWNSEN|)EENN(WSNE|)ENENENNWN(WWS(SS(S|EN(N|E))|W)|ENESSSENESSWSSWW(NENSWS|)WSWSEEESWWS(WNSE|)EEEENWNEESS(ENNNWNW(SWWWEEEN|)NENNNESSENESENNENNEEEN(WWWWNWSSESWSWNNWSWNWW(SEWN|)NNNENNESESSE(SWWN(WSNE|)N|NNEES(ENE(NWWWWWNNWSW(N|WSSW(SSSWENNN|)NW(NEWS|)S)|SS(E|W))|W))|EESSENESE(SSESWWWWSESWSEENNEESE(SSWSES(E|WWNWSSEESWSSEES(WWWNWWW(WSSSWS(E(ENENE(NWWSNEES|)SES(EENNW(W|S)|WW)|S)|WWWWW(S(E|WWW(SS|WW))|NEEENE(NE(NWWSWW(S(WWWWNSEEEE|)E|NEN(W|N))|S)|S)))|NEEENWWWNNNN(ESSSENNNEEN(E|N)|WWWNNNNW(NEESESSE(SWWNNSSEEN|)NNNN(ESEENW|N(WS(WWNEWSEE|)S|N))|SSWS(E|WSSWNNNEN(WWSNEE|)E))))|ENE(NWWWEEES|)E(SWEN|)E))|NNNNNWS(NESSSSNNNNWS|))|NNWWNW))|S))))|WWWNNWWWSWNNWWNWWSESS(ENESNWSW|)WSSWNWSW(SEWN|)NNENE(S|NWWSWWSES(WWWWNWNNWWSWSEE(SSSES(EENNW(WNSE|)S|WWWNNE(S|NWWNWWWWS(EEES(ESSNNW|)W|WSW(SESWENWN|)NWW(SEWN|)NEENE(ENNWNWWWWNNWWNNNWNENWNEEESSW(SSENESS(ENNNWNNENNWNNWWSSE(S(WWWWSWWSSWNNNWSWNNEENNESENENNWNEENNWWWS(EE|WNWNNWWWSWWSWWNWNWNEENNNEEESESWWW(NEWS|)SS(EEN(EEENESEEEESES(WWNWWSE(WNEESEWNWWSE|)|EESWSESWSS(W(N|SSE(N|SWW(WSWNSENE|)N))|ENESENNWNENWNENENWNNWSWWNWNWSS(ESESENES(NWSWNWESENES|)|WWNENWNNESEENEENEEESENNESSSEENWNNENWNWSWNNEEESENESSENESSSWNWSW(W|NN|SSENESESSESEENWNEENNNNWSSWNNNNW(SSSS(W|S(EE|S))|NEENWNNNN(WSWNWWSESWWSWS(EEEN(W|ENESSWSW(ENENNWESSWSW|))|WWNENWNN(ESE(ENWESW|)S|WSWWWN(WWWWWSWNWWWWWWWSEEESWWSEEESWSEEEENESSSEESWWWNNWSSS(WWWWSESS(WNWWNENNENN(NWSWS(E|SWNNWWWSEESSE(SSWWWSWNWNWNENWWNWSSWNNNWNENWNWSSWNNW(SSSEESSE(SSSEEN(W|E(SSSEN(ESSSWWSWSSWSESSESSEENNNW(NENESEN(NWN(NNEEN(WW|N)|WSW(N|WS(SS|E)))|ESESEEN(EESENN(WW|NESSESESE(N|S(E|WWWWN(E(E|N)|WSSSE(EN(ESNW|)W|SWWNWNWSSWWNWSWWWSSWNWWNEEN(WNNWNWN(E|NNWSSWW(NENNEEENNEN(EE|NNWWWWNENNWW(NNESNWSS|)S(SSSESEE(SWS(E|WNW(SSS|N))|N(ESNW|)W)|E))|SEESSE(S(ENSW|)WSSE(SESSEEN(WNSE|)EESWSWWSSEESSWSEEESEEEESSESEENENEENESEESENNWNEESSSSES(WWS(EE|WNWWWS(SWWSEESWSWNWWWSWWSWWNENEENNNEEN(WWNWNWN(WWSSSWWNNE(NNWSWNWSSSE(SWSSENESESWSEE(SSWWWWSWNNWSWWSESSEN(ESESWWSWSWSW(SEESSESWWNNWSSSSSESSW(N|SESESESWWNW(N|SSSESENN(ESEEENNENESESSSSWWWN(ENNESS|WSWNWSSWWW(N(N|EE)|SESWSEEEENN(WWSEWNEE|)NEESSENNEEENENNNESEEENENNNWNENN(EEE(SWSW(SEE(N|SESESWSESESWWWN(E|WNNN(ESSNNW|)NWSSSWSESWWNWSSSEEN(ESEN(ESESSSWSEENNENNW(S|N(W|EENNESENEEENWNWSWNWWS(E|WNNN(NW(S|W)|ESEEEENNW(NEEESWSSSESSENENWNEESEENWNE(NNWSWNNNENWN(EESSESWW(EENWNNSSESWW|)|N|WSSSSSS(E|W(WSNE|)NNN(W|NNN)))|ESSENEN(ESSWSSWNWWSSSSEESSENNNN(WSWNNSSENE|)ENNESSEENNENWW(WNNNWNENEENN(WWSEWNEE|)EN(W|ESSWSEEEESENEE(SEESENEN(W|E(SSSSE(NN|SSSSSWWSSWWWSEEESESE(NNWNEN(E|W)|SESWSWSEEESSWSSENESSSWWN(E|WWWWNENENE(SSWENN|)N(E|WWNNWSSWSWWNNWNNNWNNWWSWNNWSSWNNWNWWNN(ESENENEN(NESESWSWSES(WWNSEE|)EENWNEENESEEES(ENNEENWNWWS(SWNNWWNNNN(N|WSWNWNWSSESWW(SEESES(E(NNNWS|EES)|S)|NNNWSWW(NEWS|)SSEENW)|ESE(N|ESES(WWNWSNESEE|)E(ESNW|)NN(N|W)))|E)|WWWSESW(SSENESESES(WSSW(S|NNW(NEWS|)S)|EN(NWNW|ES))|W(WW|NN)))|W)|WSSSEESSWNWWWWSWSSESSWWWSSWWNNNWWSWSESWWSSSSSENNNNEE(NNN|ESSENNEEENN(WSNE|)EESENNESSSESSSSWSWWNNNEE(SWSNEN|)NWNWW(S(E|WWWSEEESWWWSSENESE(N|SWSESSENNNEESENEEENESSWWSESEESESSSWWSESWWS(EEEENEENEENESSEENWNNWNWSWWNNNENEESWS(WSNE|)EESESEENWNWNNEENWNNWNWWWNNNWWSSE(SSSWNWN(WWNWWWWW(SS(W|ENEEESESSW(N|S(EENE(NWNSES|)S(SSW(NWES|)SSSE(SWWSNEEN|)(NN|EE)|ENEENEN(ESS(ENSW|)(SS|W)|W))|W)))|NNEES(W|ENENNWWWNWS(SEEEWWWN|)WNNENNN(WWNWWS(W|SE(S(EENW|WS)|N))|E(NN|EESSSW(NNWSSNNESS|)SEEEENE(SESS(WNWWSSS(W|EE(NWN|SW))|EEE(ENEWSW|)SSSEEEES(ESWSESSENNNNESSESWSSESENN(ESEENESEENEENNNWNWSWWNENWNNNESEEESS(ENNENESENEENNWNNWWSSS(E(E|NN)|WWNENNWSWNWWNNESEEEENWWN(NESEESEEEENWWWNEEENENESENENWWNNWSSWWWWWSS(W(SEWN|)NWSWNWNENENWNEESS(SWEN|)EENN(W(NWWWEEES|)S|EESWSEENNNEN(ESS(W|EESENESSW(WWNSEE|)SEESWWSESENENNNESSEENENWNNWN(NNWWSSSW(NNNWW(NN|SESWWW)|S(EE(ESSNNW|)NNN|S))|EEESEESSE(SSSWSSWWWNWN(WWSESESWWWWWWWSEESSEEENEESSW(WSWWN(E|WSWNNWSWNN(NNNEN(WWSSW(SWWW(WW|SES(ESSW(N|SEENNNE(SSSSWSWSEEEN(NEEN(WWNEWSEE|)EEEESWSSEEEENENESENNWWWSWNW(SS(W|E)|NEEEENESENNNNENEE(SWSSW(SESSESEEESWWSWWN(NWN(WSSESWWWSW(SSEEENN(WSWENE|)EEEESWSESSENESSSSWWWSSEEN(W|ESE(NNNNNNNWNW(S|NEN(W|E(NNNNW(NENWESWS|)WW(WNNN|SEE)|SS)))|SWWSES(ENSW|)WWWWNWNE(ESENSWNW|)NWWNWSWNNWSSWSWSES(WWNNWNENWNWWWWWWSWNNNWWSESWSWSWNNWSWWSSSES(WWWWWNWSWWWNNNNENWWSSWNWWN(WSSWW(SWNW(SSEEES(WWWWWWWW(NEWS|)WW|ENN(ENESESS(ENN|WWNE)|W))|W)|NENN(NWS(WNWESE|)S|E))|EENEEEEESSW(SS(EEN(W|NNNNEENEEESS(WW(NEWS|)S(SWSSE(SWWNNNNNES(NWSSSSNNNNES|)|N)|EE)|EENWNEE(NEES(ENNNN(EENEEE(NW(NEWS|)WW|SESWSESESWSES(ESES(S|ENNNENWN(EEESSS(ES(W|ESENNNE(EESWWEENWW|)NWN(E|NW(NEWS|)SWSSE(S|N)))|W(W|NN))|WSS(WNNNNW(NEESSENN(SSWNNWESSENN|)|S)|S)))|WWWNWSWNW(S|NENENN(NESSSS(E(N|S)|W)|WSW(N|S)))))|WSWWN(E|WSS(WNNWNWN(WSSW(W|SESS(WNSE|)ENN(ESSEEWWNNW|)N)|EESEENN(E|W(S|W)))|EEE)))|W)|SSS)))|W(SEWN|)N)|N))|EENEEN(NE(EEEEEEN(WWW|ESSWSEESWWWWWNEENWWWWSW(N|S(EENSWW|)WW))|N)|WWW(WNEWSE|)S))|ENNESEE(NWES|)S(E|WW))))|N)|N)|E)|N)|EENWNWWS(E|WW(NENSWS|)SWSWNWSW(WWWWWSE(WNEEEEWWWWSE|)|SES(SWN|ENES)|N))))|W)|N(WW|N)))|W))|NN)|N|ESEEE(N|E))|EE))|N)|ENEE(SSWNSENN|)ENWWNENWWN(SEESWSNENWWN|))|N(EE|N))))|W))|ENESENE(WSWNWSNESENE|))|W(S|WWW(SWSESWWWN(ENSW|)WWSSSSW(WNENNW(S|N(EN|WSW))|SSSSENESESE(SSES(E(S|EN(ESNW|)W)|WW(S|NNWSSWNNWNEE(WWSESSNNWNEE|)))|NNNNNWNN(EEESS(WNWESE|)ESEE(NNW(NWSNES|)S|E)|W(SSSWSEE(N|S)|N))))|NNNN))))|WWN(W|E))|W)|W))|NNWWN(ENSW|)WSSE(SWEN|)E)))))|E)|N)|WNNWNWWNWSWNNWWSSSWNNWWSWSEE(N|SWS(EEEEN(N(NN|EEEESESWWWN(WSNE|)E)|WW)|WNWWNNWNEE(ENWNENWWSSWWSWWSEESESWS(WNWN(E|WSWWNENWNNWWWWNEENNENWWNNWWWSWWWNWWNNEEES(SENENENWNNNENNNWWSS(ENSW|)WWSSSWWWWNNNEEE(NWNNWSSWWNWWWWWNWNNW(NEEEESWS(WNSE|)SENEENWNEN(WNNSSE|)EESWSES(WS(ESNW|)WW|EENWNNESESSENEENNWW(SEWN|)NE(EESESWSEESWWS(WNWSSSWNNWS(NESSENSWNNWS|)|EESSESWSESWWSEESEENESESENESSE(SSENESSS(ENENWNEESE(EESSWW(W(NEEWWS|)W|SSSENNEE(N|S(WSNE|)EEEES(W|EES(EE(EENWNWSWNWWNN(WSWNNSSENE|)E(SEWN|)N|S(W|S))|W))))|NNWN(E|WW(NNES|SE)))|SWNNWSWWNNWSSW(SEESENES(SWWS(S|WNNWWN)|EE)|NNWNEEE(ESSNNW|)NWWNWWWS(E(SS|E)|W(NN|W))))|NNNNNWSWNWNEEENNE(SS|ENENESEES(SS|ENE(SEWN|)NWWNEENENN(ESNW|)WWS(E|SWNWWSS(E(N|S)|WNNWWSSS(ENNSSW|)WWWNENNNW(SSWSSSEE(SWS(WNWWNWS(WNN(EEESNWWW|)NNNENWW(W|SS)|SSESWS(EE(ESEE|NNNW)|S))|E)|EE)|NENEEEN(W|EEESES(ENE(NNNWSSWN(SENNESNWSSWN|)|S)|SWWN(E|NWSSWWN(WSSSSNNNNE|)E))))))))))|NWW(WS(ES|WWNE)|NENNESS(NNWSSWENNESS|))))|SSSSE(N|SSESWSW(SESESWW(N|SESSENEENW(W|NNESEESWSESENNEE(SWSESSENNE(NWES|)SSESWSSSWNNNWWSESSWNWSSEEEESENNNEN(ESENE(SEEEESSWWN(E|WWWSWSW(NNEWSS|)S(WWWWWN(EE|WSWNWSWNWWWNW(SSEEEWWWNN|)NNESESEENNENWWW(SESNWN|)NWW(SEWN|)NEN(WNNSSE|)ESENEE(NWWEES|)EEESWSSW(SSWSEWNENN|)NWW(NEEWWS|)W)|EENENESS(ENEES(EEEE|W)|W)))|NWNWW(SEWN|)NN(WSNE|)EES(W|EE(NWNE|ESWW)))|W)|NNN(WWNWNWNWW(NN(WSNE|)ESEE(NWES|)SE(S|N)|SESES(WWNWSNESEE|)ES(ESENSWNW|)W)|EEEEE(S|NWNNES)))))|NNN)))|SWWSEE)|WW))|EEE)|SS))))))|N)))))))|N))|NWWWWWN(SEEEEEWWWWWN|)))|SS)|W))|S)))))|N)|W)))|N)|E)|WNENWWNENNNWSWWWNEENENWWNNENNWNN(WSSS(E|WSSSS(SSWNNWSSWNWWSSSW(NNNWNN(W|NESEENWN(W|EEENNWW(SEWN|)N(NESENEESSW(N|SSSSWNWSSWWW(EEENNEWSSWWW|))|W)))|SESEEEESWSSES(WWWWS(E|SW(SEWN|)NNNW(NNW(SS|NNE(NWWSNEES|)SESEEESSWNW(W|S))|S))|ENEENNNE(ESS(E|SW(SWSS(ESENN(ESNW|)(W|N)|WN(N|W(SSSS|W)))|NN))|NWWSW(NWWWNN(W(SS|N)|ESENESEEEN(WW|E))|SS(ENSW|)W))))|E(NNN|E)))|ESESESSENNNEEESWWSESSESESS(ENNN(EEESWSESW(SEE|WNN)|WNENW(WSNE|)NNNNW(SWNWSWNW(NEWS|)W(SESNWN|)W|N))|WNWWSW(NNW(N(EE(SEWN|)N|W(NN|W))|S)|SE(SSWNSENN|)E(N|E)))))))|W)))|NNEN(NNW(NNNNNEES(SWNSEN|)ENNE(SEESW(SEEEWWWN|)W|NWNNE(S|NNWSWNWNNNW(NENESESSE(SWWNNSSEEN|)NN(NNW(WWW(S|NEENWWNNNNE(NWNENW|SSES(ESSEWNNW|)W))|S)|E(SS|E))|SSSSESSW(N|SSEEN(W|NN)))))|SS)|E))|N)|ENWNN(EEN(WN(E|N)|ESSW(SEWN|)W)|W))|N)|S)|E)|EES(ENN(NEES(W|E(EE|N))|W)|WSWS(WNSE|)E))|EE))|ENNNW(S|NNN(WWNWNWNN(WWWWNEENWWNWN(E|WSSWNWWWWW(WNEEEEEEN(SWWWWWEEEEEN|)|SEEEESESENE(SSSSWSSW(SES(S|ENEENWWNENNNEESWSSESEE(N(E|NW(S|NN|W))|S)|W)|NWSWNNWWS(SENSWN|)WWNENEENNE(NWWWSESWW(N|WW|S)|ESWSESENNES(NWSSWNSENNES|)))|N)))|EE(NWNEWSES|)S(ES(W|S)|W))|ESSESS(EENN(WS|ESE)|SS))))|N)|N)))|EEENNNNEESSE(S(WWNNSSEE|)S|NESEE(N(N|W)|S))))))))|W))|SS)|N)|N))|N)|NENWNEESENESS(WW|ES(SEESENNNEESENESSWS(EEEN(WNENWNNWSWWN(WSWWSW(SEWN|)NWNEEE|E)|EE)|WW(SWW(WNWSNESE|)SEESESE(S(WWNSEE|)E|N)|N(N|E)))|W)))|E))|EEESWS(WNSE|)E)|S(S|EENNN(ESSSS|WSS)))|EEE(EENEENNNEE(SWSESW|NNE(S|NWWSSWNNWSSWSS(SENNSSWN|)WWNENNNW(S(S|WNWSWW(SEWN|)NN(WS|ES))|NEE(E|SS))))|S))|EE)))|EEESEESSSEEESWSWSWSWW(NNW(WSEWNE|)NEE(E(SWSNEN|)E|NNWN(WW(NE|SESE)|E))|SESWSESWSWSSSEEEESWWWSSWNNWWWNWWNE(EESENSWNWW|)NWWWSSSSWWWSWW(SEEESENESSWWSSEEEN(WW|ENNNW(SS|N(WWW|NN(NN|EESWSESEENWNEESSEESESSESSSENNNNNESSESWSEEEESESWWWWW(NEEEWWWS|)SEESWWSESEEESSESEENESSEENWNENNWNENWWNNESEESSESWSSENEESWSWWSESEEN(EESES(WSSWWNWSWNN(WWN(WSWNWWN(E|WWNWNNWWWWN(NWNNE(NWWSSSSSSWNNWSWSESSS(SSESENNWNEEE(SWEN|)EENWWNN(ESENESS(S|E)|W(NN|SSWNWS))|WNNWNWNWWNENWNNENENWNW(NENESENNN(WSWNSENE|)ESSEN(N|ESSWWSEEE(N|SWWSESWWW(NN(ES|NW)|SW(W|N|SEE(NEESWENWWS|)SW(W|S)))))|WSWWWWW(NNE(SEEEWWWN|)NWNW(NEESNWWS|)SS|SS(W|EESENEN(WWW|E(ENSW|)SSSSSWSSSSW(SES(W|EENWNEE(EN(ESNW|)WN(NWWSESW(ENWNEEWWSESW|)|E)|S))|NWNNE(S|NNN(E(N|S)|WSSWW(SS(WWNE|EN)|NENNWS)))))))))|S)|E))|E)|EEE(S|N))|EE(NESENNNESSENESSESE(NNEEEENNNEENNESSSSSSSSW(NNNW(NNESNWSS|)S(S|WW)|SEEENESSSENNENNNWNEESSSEESWSEENNNNNEENNWNNWSWSS(W(SSSENN|WWWNNESENNE(SS|ENWNENWWWNNEEENNWWNNENENENWWNNEES(ESENEEEENNENNNESSESENE(NNWNENNWWS(WS(ESSEWNNW|)WWNWSWWWNNWWWNEEEESSENNNENNWWWSE(SWWNWSWNWNEEENENWWSWNWWNWWWWWWSWNWNWWNENESENEEEN(ESSEES(EENNW(WWNEEEESSENEEN(WW|EESENESSESWWWWS(WWWW(NEEEN(W|E(N|EE))|W)|SSENEN(EEESENNEE(NWNWW(WNEEEES(NWWWWSNEEEES|)|S(S|E))|SSSSWN(NN|WSWSWW(NN(W|E(NWES|)(E|S))|SESS(WNWSNESE|)(S|ENNESEENNW(W|S)))))|W)))|S)|WWWNWWS(W|E))|WWWWWWSWWWWN(EEE|WSSWSSWWNWWWSWSWSESSWNWWSSE(ESENESSSWWWWWNN(ESE(EE|N)|NWNNE(S|NWWSWNWNNESEEEENE(SSWENN|)ENWNNWSWWSS(ENEWSW|)WNNWSWNWSWNNN(WSWWN(WWWWW(SSSENNEESS(WNSE|)SESESSSSWSESSEENESSESWSSSEESEESENNNENESSWSSSSWWWN(WWSESSSSS(WWW(NWNWNWNW(SSEWNN|)WNNNNW(NEESSENNNESESWSSS(WNWSNESE|)SEENE(SSE(SW(SE|WNW)|N)|NWN(WSSNNE|)N(NNWNWWNEEE(S|NWWWNWSSSWNNNWSWW(SEES(SEEWWN|)WWWWNEN(SWSEEEWWWNEN|)|NENNN(WSSNNE|)EN(W|NENE(SS(ENNSSW|)WSESS(WNWSNESE|)E|N))))|EES(W|EE)))|WSESWW(WNENWESWSE|)SS)|SESSWNWWSES(WW(NN|W)|SSE(NESE(NNWWEESS|)E|S)))|ESEESSESSENNNENWNNWWW(SEESSNNWWN|)WNNESEEENN(WSWNWW|ESENNENNNESEEESSENEESSW(SEENESSESSWWN(NWWSESWSWSESWWSEESWSWS(EENENNEENWWNEN(ESESSEESENNEENEESWS(EESWSEE(SWWSSEE(NWES|)SWWSEES(WWS|ES)|ENWNE(ES(ENSW|)S|NWWNNESENNNNNE(SSSEEWWNNN|)NWNEE(NWWNEENWWWNWNENNWWS(E|WNNNEES(W|ENESESENEEN(WWWNWNWSWNWSWWSWNWWNWWSSSWNNNNWNWWNEENEENESSSW(NWSNES|)SEEENNESSS(W|EENENWW(NN(WNWNW(WWWWN(WSS(EE|SWNWWWSSESSE(NNNWESSS|)EESSE(N|SSSESWSEEEESSWNWWWSEESWSES(WWNNWWNW(NENWNEE(SSS|NNWWNWWNW(NNN(WNNNE(NEWS|)SS|ESSEEES(SEWN|)WW)|SSEES(WWSSS(WWWSSSWW(SEWN|)NNE(S|NNNNN(WSWNWSSWNWSW(WSEESEN(SWNWWNSEESEN|)|NNNWNNWNNNE(NWWSW(NNNWNN(ESE(N|SSEENW)|W(N|S))|SESSW(S|N))|ESS(WNSE|)ENESSESWS(E|SWNNWNE(WSESSEWNNWNE|))))|EESSS(WNN|ENNN)))|SEENNW(NEWS|)S)|EE)))|S)|ESESSW(NWNSES|)SESWS(W|EEE(ENNNWNNW(NNENEESSESEE(NWNN(ESNW|)NW(WWWNEEENWWNNNWWNWW(N|SSE(N|S(WWNSEE|)EE(NWES|)SWSSSWWSES(NWNEENSWWSES|)))|SS)|SSWWSES(ENSW|)WSWNNNN(EE|NW(N(W|N)|S)))|SSSESW)|SWWWSE)))))|NESEENW(ESWWNWESEENW|))|SS(E|SS))|EENEEESE(SSS(WNWN(NWS(SSEWNN|)WW|E)|E(NNNEWSSS|)ESSWNW)|N))|S))|EESWSSSS(WNNW(NEWS|)WWW(N|SSEEN(ESNW|)W)|ESENNESENNN(EESSW(SSWWEENN|)N|WWWS(S|EE))))))|E|S)))|WWSSWWSWNNWWWSESWWS(EES(WSNE|)EEENWWNN(SSEESWENWWNN|)|W))|W)|WNWWSESS(ENSW|)W(NW(S|NWNENENENE(NNE(S|NWNNNEE(NWWNWN(E|WSSESSSSSSWNNNWN(ENWNSESW|)WWSSE(N|ESSS(E|W(WSS(ENEWSW|)SSS(W|E)|NN))))|SS(WNSE|)S))|SS(W|E)))|S))|E)|N)))|EE)|WW)|E)|ESENEEES(WW|ENEEESENEEEEEESWS(S|WNWSWW(NEWS|)W(S|W))))))|N)))|E)|E)|SSSWNWW(SESESWWNW(NN|WWSWSESEEN(WNEWSE|)EEESSSSWSSENE(NNNNNNN|SSWWSSESSWSWNWSWNWWNWS(WNNWWNNN(NESSEENWNEESSENESSENE(NWNWNNWWN(WWSWNW(N(WSWENE|)EN(NESE(NNEWSS|)S(E|W)|W)|SSEEE(EESNWW|)N)|EEESE(NE(NWES|)S|S(S|W)))|SSWWW(SEEEEWWWWN|)NWSWNWW(EESENEWSWNWW|))|WWW)|SSSENNESESSW(SEEENNW(NEEENE(NNNNWESSSS|)SSSWSW(NNEWSS|)SWSSENENE(SSWSSE(SWSWSEE(N|SWSSWSEE(NN|SSWWWN(WWSSSSWSESWW(SSW(NWES|)SSEEENESENNNENWW(SSWW(SW|NE)|NEEENE(NWWSWNN(W(SS|N)|EEE)|SSWSSSE(NN|SWW(SWWSESEEE(NNWSWENESS|)SSWSS(ENSW|)WNNN(E|W)|NN))))|NNNENNW(S|WWNNNENNWSWWNNWWNNESEENWNEN(NNNEEE(NNWSWWNE(WSEENEWSWWNE|)|SWWSSSSSES(WWSNEE|)EEEESWWSESEE(N(ENWNNNWSWNWNWNN(WSSSESE(WNWNNNSSSESE|)|ESEE(ES(E(N|S)|WW)|NNW(WNEEWWSE|)S))|W)|SWWWW(N(E|NN)|S(WNSE|)E)))|WWWWN(N|WSSE(SWSESWWSS(WNNWWNNESENN(ESSNNW|)WWNENWW(WW|S)|EE(SWSNEN|)N(EESWENWW|)W)|EE)))))|EE)))|N)|N)|S)|N))))|N))|W)))|E(E|N)))|SWSWNNWNWSSE(WNNESEWNWSSE|))|S))|W)))))|NEN(W|EENWNEE(SS|NNWN(WSWNNWS(WNNWSWWSE(S|E)|SSE(EE|SS))|EESEENW(ESWWNWESEENW|))))))))))))|W)|W|S))|E)|N)|WW)|N)|S))))|N)|S)|E)))|E)|SS)|SW(W|S(E|SS)))))|S)|E)|SWW(N|WW))|ENESSWW(EENNWSNESSWW|)))|N)|EE)|W))|WW))|S))|E)))|NNESE(EEES(E(NNW|SSE)|WWW)|N))|E)|E)|N))))|NNNNN(W(WW|N)|E))|WN(N|WWSS(E(SENSWN|)N|W))))))|SEES(SESNWN|)WW)))|W)|W)|N(NN|WW))|E))|S(WWS|EES))|N)|E)|E)|ENNEESES(ENSW|)WW(N|S)))|SEESWSE(WNENWWEESWSE|))|S(WWNSEE|)SSSSWS)$";

    struct room_key {
        int x, y;
        room_key(int i, int j) : x(i), y(j) { }
        bool operator<(const room_key& right) const {
            int cmp = y - right.y;
            if (cmp == 0) {
                cmp = x - right.x;
            }
            return cmp < 0;
        }
    };
    struct room : dij::node {
        room_key key;
        std::map<char, room*> around = {
            { 'N', nullptr }, { 'E', nullptr }, { 'S', nullptr }, { 'W', nullptr },
        };
        room(int x, int y) : key(x, y) { }
    };
    struct map : dij::graph {
        std::map<room_key, std::shared_ptr<room>> rooms;

        room* get_or_add(int x, int y) {
            auto& rspr = rooms[{ x, y }];
            if (rspr == nullptr) {
                rspr = std::make_shared<room>(x, y);
            }
            return rspr.get();
        }
        room* move(room* pr, char d) {
            std::map<char, std::pair<int, int>> mods_stor {
                { 'N', { 0, -1 } }, { 'E', { 1, 0 } }, { 'S', { 0, 1 } }, { 'W', { -1, 0 } },
            };
            auto&& mods = mods_stor[d];
            room* pt = get_or_add(pr->key.x + mods.first, pr->key.y + mods.second);
            const char reverse_d = d == 'N' ? 'S' : (d == 'E' ? 'W' : (d == 'S' ? 'N' : 'E'));
            pr->around[d] = pt;
            pt->around[reverse_d] = pr;
            return pt;
        }

        virtual std::vector<dij::node*> all_passable() const override {
            return from(rooms)
                 | select([](auto&& rp) -> dij::node* { return rp.second.get(); })
                 | to_vector();
        }
        virtual std::vector<dij::node*> neighbours(dij::node* pn, dij::node* pinclude) const override {
            return from(dynamic_cast<room*>(pn)->around)
                 | select([](auto&& rp) -> dij::node* { return rp.second; })
                 | where([](dij::node* pn) { return pn != nullptr; })
                 | to_vector();
        }
        virtual int64_t dist(dij::node* pn1, dij::node* pn2) const override {
            return 1;
        }
        virtual bool is_a_better(dij::node* pa, dij::node* pb) const override {
            return false;
        }
    } area;
    room* start = area.get_or_add(0, 0);

    {
        std::unordered_set<room*> walkers{ start }, side;
        std::stack<std::unordered_set<room*>> prev, prevside;
        for (size_t i = 0; i < INPUT.size(); ++i) {
            //if (i % 100 == 0) {
            //    std::cout << i << " / " << INPUT.size() 
            //              << " (" << walkers.size() << " walkers, "
            //              << prev.size() << " stack deep, "
            //              << area.rooms.size() << " rooms)"
            //              << std::endl;
            //}
            char c = INPUT[i];
            if (c == '^' || c == '$') {
                // whatev
            } else if (c == '(') {
                prev.push(walkers);
                prevside.push(side);
                side.clear();
            } else if (c == ')') {
                walkers.insert(side.begin(), side.end());
                side = prevside.top();
                prevside.pop();
                prev.pop();
            } else if (c == '|') {
                side.insert(walkers.begin(), walkers.end());
                walkers = prev.top();
            } else {
                std::unordered_set<room*> newwalkers;
                for (room* pr : walkers) {
                    newwalkers.insert(area.move(pr, c));
                }
                walkers = std::move(newwalkers);
            }
        }
    }

    std::unordered_map<dij::node*, int64_t> dist;
    std::unordered_map<dij::node*, dij::node*> prev;
    std::tie(dist, prev) = dij::get_dijkstra(&area, start);
    auto furtest_dist = from(dist)
                      | max([](auto&& ndp) { return ndp.second; });
    std::cout << "First puzzle: furtest room requires passing through " << furtest_dist << " rooms." << std::endl;

    auto num_farther_than_1000 = from(dist)
                               | where([](auto&& ndp) { return ndp.second >= 1000; })
                               | count();
    std::cout << "Second puzzle: " << num_farther_than_1000 << " rooms require passing through at least 1,000 doors." << std::endl;

    //const int min_x = area.rooms.begin()->first.x;
    //const int min_y = area.rooms.begin()->first.y;
    //const int max_x = area.rooms.rbegin()->first.x;
    //const int max_y = area.rooms.rbegin()->first.y;
    //for (int y = min_y; y <= max_y; ++y) {
    //    if (y == min_y) {
    //        for (int x = min_x; x <= max_x; ++x) {
    //            room* pr = area.get_or_add(x, y);
    //            if (x == min_x) {
    //                std::cout << '#';
    //            }
    //            std::cout << (pr->around['N'] != nullptr ? '-' : '#') << '#';
    //        }
    //        std::cout << std::endl;
    //    }
    //    for (int x = min_x; x <= max_x; ++x) {
    //        room* pr = area.get_or_add(x, y);
    //        if (x == min_x) {
    //            std::cout << (pr->around['W'] != nullptr ? '|' : '#');
    //        }
    //        std::cout << (pr->key.x == 0 && pr->key.y == 0 ? 'X' : ' ')
    //                  << (pr->around['E'] != nullptr ? '|' : '#');
    //    }
    //    std::cout << std::endl;
    //    for (int x = min_x; x <= max_x; ++x) {
    //        room* pr = area.get_or_add(x, y);
    //        if (x == min_x) {
    //            std::cout << '#';
    //        }
    //        std::cout << (pr->around['S'] != nullptr ? '-' : '#') << '#';
    //    }
    //    std::cout << std::endl;
    //}
    //std::cout << std::endl << std::endl;

    //std::cout << "min_x=" << min_x << std::endl
    //          << "max_x=" << max_x << std::endl
    //          << "min_y=" << min_y << std::endl
    //          << "max_y=" << max_y << std::endl;
    //for (int y = min_y; y <= max_y; ++y) {
    //    for (int x = min_x; x <= max_x; ++x) {
    //        room* pr = area.get_or_add(x, y);
    //        int rep = 0;
    //        if (pr->around['N'] != nullptr) {
    //            rep |= 0x1;
    //        }
    //        if (pr->around['E'] != nullptr) {
    //            rep |= 0x2;
    //        }
    //        if (pr->around['S'] != nullptr) {
    //            rep |= 0x4;
    //        }
    //        if (pr->around['W'] != nullptr) {
    //            rep |= 0x8;
    //        }
    //        if (rep != 0) {
    //            std::ostringstream oss;
    //            oss << std::hex << rep;
    //            std::cout << oss.str();
    //        } else {
    //            std::cout << ' ';
    //        }
    //    }
    //    std::cout << std::endl;
    //}
    //std::cout << std::endl;
}

void day21_1and2()
{
    struct registers {
        int64_t r[6] = { 0 };
        std::string dump() const {
            std::ostringstream oss;
            oss << std::hex << "[";
            for (int i = 0; i < 6; ++i) {
                if (i != 0) {
                    oss << ", ";
                }
                oss << r[i];
            }
            oss << "]";
            return oss.str();
        }
    };
    using opcode_impl = std::function<void(registers&, int64_t, int64_t, int64_t)>;
    std::map<std::string, opcode_impl> opcodes = {
        {
            "addr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] + regs.r[b];
            }
        },
        {
            "addi",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] + b;
            }
        },
        {
            "mulr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] * regs.r[b];
            }
        },
        {
            "muli",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] * b;
            }
        },
        {
            "banr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] & regs.r[b];
            }
        },
        {
            "bani",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] & b;
            }
        },
        {
            "borr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] | regs.r[b];
            }
        },
        {
            "bori",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a] | b;
            }
        },
        {
            "setr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = regs.r[a];
            }
        },
        {
            "seti",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = a;
            }
        },
        {
            "gtir",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (a > regs.r[b] ? 1 : 0);
            }
        },
        {
            "gtri",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (regs.r[a] > b ? 1 : 0);
            }
        },
        {
            "gtrr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (regs.r[a] > regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqir",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (a == regs.r[b] ? 1 : 0);
            }
        },
        {
            "eqri",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (regs.r[a] == b ? 1 : 0);
            }
        },
        {
            "eqrr",
            [](registers& regs, int64_t a, int64_t b, int64_t c) {
                regs.r[c] = (regs.r[a] == regs.r[b] ? 1 : 0);
            }
        },
    };
    struct instruction {
        std::string opname;
        const opcode_impl* op = nullptr;
        int64_t a = 0, b = 0, c = 0;
    };

    registers cpu;
    int64_t* ip = nullptr;
    std::vector<instruction> program;
    {   
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        int n = 0;
        char c = 0;
        iss >> c >> c >> c >> n;
        ip = &cpu.r[n];
    }
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        instruction inst;
        iss >> inst.opname >> inst.a >> inst.b >> inst.c;
        inst.op = &opcodes[inst.opname];
        program.emplace_back(inst);
    }

    cpu = { { 0, 0, 0, 0, 0, 0 } };
    int64_t first = -1, last = -1;
    std::unordered_set<int64_t> seen;
    while (*ip >= 0 && *ip < static_cast<int64_t>(program.size())) {
        const instruction& inst = program[*ip];
        (*inst.op)(cpu, inst.a, inst.b, inst.c);
        ++*ip;
        if (*ip == 28) {
            if (!seen.emplace(cpu.r[4]).second) {
                break;
            }
            if (first == -1) {
                first = cpu.r[4];
            }
            last = cpu.r[4];
        }
    }
    std::cout << "Day 1: " << first << std::endl
              << "Day 2: " << last << std::endl;
}

void day22_1and2()
{
    const int64_t DEPTH = 5913;
    const int TARGET_X = 8;
    const int TARGET_Y = 701;
    const int CAVE_SIZE_X = TARGET_X + 50;
    const int CAVE_SIZE_Y = TARGET_Y + 50;

    enum class tool {
        none,
        torch,
        climbing_gear,
    };
    struct region : dij::node {
        int x = 0, y = 0;
        int64_t geo_index = 0;
        int64_t erosion_lvl = 0;
        char type = 0;
        int64_t risk = 0;
        tool reqd_tool;
    };
    struct cavern : dij::graph {
        const int64_t k_depth;
        const int k_size_x;
        const int k_size_y;
        const int k_target_x;
        const int k_target_y;
        std::vector<std::vector<std::map<tool, std::shared_ptr<region>>>> regions;
        cavern(int64_t depth, int size_x, int size_y, int target_x, int target_y)
            : k_depth(depth), k_size_x(size_x), k_size_y(size_y), k_target_x(target_x), k_target_y(target_y)
        {
            regions.resize(k_size_x);
            for (auto&& r : regions) {
                r.resize(k_size_y);
            }
            for (int x = 0; x < k_size_x; ++x) {
                for (int y = 0; y < k_size_y; ++y) {
                    std::shared_ptr<region> regs[2] = { std::make_shared<region>(), std::make_shared<region>() };
                    regs[0]->x = x;
                    regs[0]->y = y;
                    regs[0]->geo_index = get_geo_index(x, y);
                    regs[0]->erosion_lvl = get_erosion_lvl(regs[0]->geo_index);
                    regs[0]->type = get_type(regs[0]->erosion_lvl);
                    regs[0]->risk = get_risk(regs[0]->erosion_lvl);
                    *regs[1] = *regs[0];

                    auto reqd_tools = get_reqd_tools(regs[0]->type);
                    regs[0]->reqd_tool = reqd_tools[0];
                    regs[1]->reqd_tool = reqd_tools[1];

                    regions[x][y][regs[0]->reqd_tool] = regs[0];
                    regions[x][y][regs[1]->reqd_tool] = regs[1];
                }
            }
        }

        region* first_at(int x, int y) const {
            return regions[x][y].begin()->second.get();
        }

        virtual std::vector<dij::node*> all_passable() const override {
            std::vector<dij::node*> out;
            for (int x = 0; x < k_size_x; ++x) {
                for (int y = 0; y < k_size_y; ++y) {
                    for (auto&& r : regions[x][y]) {
                        out.push_back(r.second.get());
                    }
                }
            }
            return out;
        }
        virtual std::vector<dij::node*> neighbours(dij::node* pn, dij::node* pinclude) const override {
            region* pr = dynamic_cast<region*>(pn);
            std::vector<dij::node*> out;
            for (auto&& r : regions[pr->x][pr->y]) {
                if (r.second->reqd_tool != pr->reqd_tool) {
                    out.push_back(r.second.get());
                }
            }
            auto add_if = [&](int xmod, int ymod) {
                int x = pr->x + xmod;
                int y = pr->y + ymod;
                if (x >= 0 && x < k_size_x && y >= 0 && y < k_size_y) {
                    for (auto&& r : regions[x][y]) {
                        if (r.second->reqd_tool == pr->reqd_tool) {
                            out.push_back(r.second.get());
                        }
                    }
                }
            };
            add_if(-1, 0);
            add_if(0, -1);
            add_if(1, 0);
            add_if(0, 1);
            return out;
        }
        virtual int64_t dist(dij::node* pn1, dij::node* pn2) const {
            region* pr1 = dynamic_cast<region*>(pn1);
            region* pr2 = dynamic_cast<region*>(pn2);
            return pr1->reqd_tool != pr2->reqd_tool ? 7 : 1;
        }
        virtual bool is_a_better(dij::node* pa, dij::node* pb) const {
            return false;
        }
    private:
        auto get_geo_index(int x, int y) const -> int64_t {
            if (x == 0 && y == 0) {
                return 0;
            } else if (x == k_target_x && y == k_target_y) {
                return 0;
            } else if (y == 0) {
                return static_cast<int64_t>(x) * 16807;
            } else if (x == 0) {
                return static_cast<int64_t>(y) * 48271;
            }
            return first_at(x - 1, y)->erosion_lvl * first_at(x, y - 1)->erosion_lvl;
        };
        auto get_erosion_lvl(int64_t geo_index) const -> int64_t {
            return (geo_index + k_depth) % 20183;
        };
        auto get_type(int64_t erosion_lvl) const -> char {
            const std::unordered_map<int64_t, char> TYPES_PER_MOD = {
                { 0, '.' },
                { 1, '=' },
                { 2, '|' },
            };
            return TYPES_PER_MOD.find(erosion_lvl % 3)->second;
        };
        auto get_risk(int64_t erosion_lvl) const -> int64_t {
            return erosion_lvl % 3;
        };
        auto get_reqd_tools(char type) const -> std::vector<tool> {
            if (type == '.') {
                return { tool::torch, tool::climbing_gear };
            } else if (type == '=') {
                return { tool::none, tool::climbing_gear };
            } else {
                return { tool::none, tool::torch };
            }
        }
    } cave(DEPTH, CAVE_SIZE_X, CAVE_SIZE_Y, TARGET_X, TARGET_Y);
    
    int64_t total_risk = 0;
    for (int x = 0; x <= TARGET_X; ++x) {
        for (int y = 0; y <= TARGET_Y; ++y) {
            total_risk += cave.first_at(x, y)->risk;
        }
    }
    std::cout << "Risk: " << total_risk << std::endl
              << std::endl;

    region* pstart = cave.regions[0][0].at(tool::torch).get();
    region* pend = cave.regions[TARGET_X][TARGET_Y].at(tool::torch).get();
    auto [dist, prev] = dij::get_dijkstra(&cave, pstart);
    auto dist_to_end = dist[pend];
    std::cout << "Number of minutes to reach target: " << dist_to_end << std::endl
              << std::endl;
}

void day23_1and2()
{
    struct point {
        int64_t x, y, z;
        point(int64_t i = 0, int64_t j = 0, int64_t k = 0) : x(i), y(j), z(k) { }
        bool operator<(const point& right) const {
            int64_t cmp = x - right.x;
            if (cmp == 0) {
                cmp = y - right.y;
                if (cmp == 0) {
                    cmp = z - right.z;
                }
            }
            return cmp < 0;
        }
    };
    struct bot {
        point p;
        int64_t rad = 0;
        bool operator<(const bot& right) const {
            return rad < right.rad;
        }
    };
    auto manhattan = [](const point& a, const point& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z);
    };

    std::vector<bot> bots;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        bot b;
        char c = 0;
        iss >> c >> c >> c >> c >> c >> b.p.x
            >> c >> b.p.y
            >> c >> b.p.z
            >> c >> c >> c >> c >> b.rad;
        bots.push_back(b);
    }

    auto& raddest_bot = from(bots)
                      | max();
    auto bots_in_range = from(bots)
                       | where([&](auto&& b) { return manhattan(b.p, raddest_bot.p) <= raddest_bot.rad; })
                       | count();
    std::cout << "Bots in range of raddest bot: " << bots_in_range << std::endl;

    struct botrep {
        int64_t closest, farthest;
        botrep(int64_t i = 0, int64_t j = 0) : closest(i), farthest(j) { }
    };
    std::vector<botrep> botreps;
    for (auto&& b : bots) {
        auto m = manhattan(b.p, { 0, 0, 0 });
        botreps.emplace_back(std::max<int64_t>(m - b.rad, 0), m + b.rad);
    }
    enum class begend {
        closest,
        farthest,
    };
    auto sortedreps = from(botreps)
                    | select_many_with_index([&](auto&& br, size_t i) {
                                                 std::vector<std::tuple<int64_t, begend, size_t>> out;
                                                 out.emplace_back(br.closest, begend::closest, i);
                                                 out.emplace_back(br.farthest, begend::farthest, i);
                                                 return out;
                                             })
                    | order_by([](auto&& t) { return std::get<0>(t); });
    auto farthest = from(botreps)
                  | select([](auto&& br) { return br.farthest; })
                  | max();
    std::vector<int> overlaps;
    overlaps.reserve(farthest + 1);
    int num_overlaps = 0;
    for (auto&& t : sortedreps) {
        int64_t curoi = std::get<0>(t);
        while (curoi >= static_cast<int64_t>(overlaps.size())) {
            overlaps.push_back(num_overlaps);
        }
        if (std::get<1>(t) == begend::closest) {
            ++num_overlaps;
        } else {
            --num_overlaps;
        }
        overlaps.back() = num_overlaps;
    }
    auto max_overlap_and_dist = from(overlaps)
                              | select_with_index([](int o, size_t i) { return std::make_pair(o, i); })
                              | order_by_descending([](auto&& p) { return p.first; })
                              | then_by([](auto&& p) { return p.second; })
                              | first();
    std::cout << "Shortest distance between best point and {0, 0, 0}: " << max_overlap_and_dist.second << std::endl
              << "Number of bots in range of best point: " << max_overlap_and_dist.first << std::endl;
    
    //for (auto&& t : sortedreps) {
    //    std::cout << std::get<0>(t) << ": ";
    //    if (std::get<1>(t) == begend::closest) {
    //        std::cout << "Beginning of #" << std::get<2>(t) << std::endl;
    //    } else {
    //        std::cout << "End of #" << std::get<2>(t) << std::endl;
    //    }
    //}
}

void day24_1and2()
{
    enum class army {
        immune,
        infection,
    };
    struct group {
        army side;
        int64_t u_count = 0;
        int64_t hp = 0;
        std::set<std::string> immunities;
        std::set<std::string> weaknesses;
        int64_t atk = 0;
        std::string atk_type;
        int64_t initiative = 0;

        int64_t effective_power() const {
            return u_count * atk;
        }
        bool operator==(const group& right) const {
            return side == right.side &&
                   u_count == right.u_count &&
                   hp == right.hp &&
                   immunities == right.immunities &&
                   weaknesses == right.weaknesses &&
                   atk == right.atk &&
                   atk_type == right.atk_type &&
                   initiative == right.initiative;
        }
        std::string to_string() const {
            std::ostringstream oss;
            oss << u_count << " units each with " << hp << " hit points ";
            if (!immunities.empty() || !weaknesses.empty()) {
                oss << "(";
                if (!immunities.empty()) {
                    oss << "immune to ";
                    for (auto it = immunities.begin(); it != immunities.end(); ++it) {
                        if (it != immunities.begin()) {
                            oss << ", ";
                        }
                        oss << *it;
                    }
                    if (!weaknesses.empty()) {
                        oss << "; ";
                    }
                }
                if (!weaknesses.empty()) {
                    oss << "weak to ";
                    for (auto it = weaknesses.begin(); it != weaknesses.end(); ++it) {
                        if (it != weaknesses.begin()) {
                            oss << ", ";
                        }
                        oss << *it;
                    }
                }
                oss << ") ";
            }
            oss << "with an attack that does " << atk << " " << atk_type << " damage at initiative " << initiative;
            return oss.str();
        }
    };
    const std::vector<group> INPUT = {
        // Immune system:
        { army::immune,    2987, 5418,  { "slashing" },                         { "cold", "bludgeoning" },      17,  "cold",        5  },
        { army::immune,    1980, 9978,  { "cold" },                             { },                            47,  "cold",        19 },
        { army::immune,    648,  10733, { "radiation", "fire", "slashing" },    { },                            143, "fire",        9  },
        { army::immune,    949,  3117,  { },                                    { },                            29,  "fire",        10 },
        { army::immune,    5776, 5102,  { "slashing" },                         { "cold" },                     8,   "radiation",   15 },
        { army::immune,    1265, 4218,  { "radiation" },                        { },                            24,  "radiation",   16 },
        { army::immune,    3088, 10066, { },                                    { "slashing" },                 28,  "slashing",    1  },
        { army::immune,    498,  1599,  { "bludgeoning" },                      { "radiation" },                28,  "bludgeoning", 11 },
        { army::immune,    3705, 10764, { },                                    { },                            23,  "cold",        7  },
        { army::immune,    3431, 3666,  { "bludgeoning" },                      { "slashing" },                 8,   "bludgeoning", 8  },
        // Infection:
        { army::infection, 2835, 33751, { },                                    { "cold" },                     21,  "bludgeoning", 13 },
        { army::infection, 4808, 32371, { "bludgeoning" },                      { "radiation" },                11,  "cold",        14 },
        { army::infection, 659,  30577, { "radiation" },                        { "fire" },                     88,  "slashing",    12 },
        { army::infection, 5193, 40730, { "radiation", "fire", "bludgeoning" }, { "slashing" },                 14,  "cold",        20 },
        { army::infection, 1209, 44700, { },                                    { "bludgeoning", "radiation" }, 71,  "fire",        18 },
        { army::infection, 6206, 51781, { "cold" },                             { },                            13,  "fire",        4  },
        { army::infection, 602,  22125, { },                                    { "radiation", "bludgeoning" }, 73,  "cold",        3  },
        { army::infection, 5519, 37123, { },                                    { "slashing", "fire" },         12,  "radiation",   2  },
        { army::infection, 336,  23329, { "cold", "bludgeoning", "radiation" }, { "fire" },                     134, "cold",        17 },
        { army::infection, 2017, 50511, { "bludgeoning" },                      { },                            42,  "fire",        6  },
    };

    auto would_deal_to = [](const group& atk, const group& def) {
        int64_t dmg = atk.effective_power();
        if (def.weaknesses.find(atk.atk_type) != def.weaknesses.end()) {
            dmg *= 2;
        } else if (def.immunities.find(atk.atk_type) != def.immunities.end()) {
            dmg = 0;
        }
        return dmg;
    };
    auto fight = [&](const std::vector<group>& in_state) -> std::vector<group> {
        auto state(in_state);
        auto for_targeting = from(state)
                           | order_by_descending([](group& g) { return g.effective_power(); })
                           | then_by_descending([](group& g) { return g.initiative; });
        std::map<group*, group*> targetted_by;
        for (group& atk : for_targeting) {
            group* pdef = from(state)
                        | select([](group& d) { return &d; })
                        | where([&](group* pd) { return pd->side != atk.side; })
                        | where([&](group* pd) { return targetted_by.find(pd) == targetted_by.end(); })
                        | order_by_descending([&](group* pd) { return would_deal_to(atk, *pd); })
                        | then_by_descending([](group* pd) { return pd->effective_power(); })
                        | then_by_descending([](group* pd) { return pd->initiative; })
                        | first_or_default();
            if (pdef != nullptr && would_deal_to(atk, *pdef) > 0) {
                targetted_by[pdef] = &atk;
            }
        }

        auto for_attacking = from(targetted_by)
                           | order_by_descending([](auto&& dap) { return dap.second->initiative; });
        for (auto&& dap : for_attacking) {
            group& atk = *dap.second;
            group& def = *dap.first;
            if (atk.u_count > 0) {
                int64_t dmg = would_deal_to(atk, def);
                if (dmg > 0) {
                    def.u_count -= dmg / def.hp;
                }
            }
        }

        return from(state)
             | where([](group& g) { return g.u_count > 0; })
             | to_vector();
    };
    auto combat = [&](const std::vector<group>& in_state) -> std::tuple<std::vector<group>, bool> {
        auto state(in_state);
        auto num_clans = [](const std::vector<group>& in_state) {
            return from(in_state)
                 | select([](const group& g) { return g.side; })
                 | distinct()
                 | count();
        };
        for (int64_t round = 0; num_clans(state) > 1; ++round) {
            //if (round % 1000 == 0) {
            //    std::cout << "Round #" << round << std::endl;
            //    for (group& g : state) {
            //        std::cout << g.to_string() << std::endl;
            //    }
            //}
            auto newstate = fight(state);
            if (newstate == state) {
                return std::make_tuple(std::move(newstate), false);
            }
            state = std::move(newstate);
        }
        return std::make_tuple(std::move(state), true);
    };

    auto [final_state, finished] = combat(INPUT);
    auto remaining_u = from(final_state)
                     | sum([](group& g) { return g.u_count; });
    std::cout << "Puzzle #1: remaining winning units: " << remaining_u << std::endl;
    if (!finished) {
        std::cout << "  Note: combat did not finish! stalemate detected." << std::endl;
    }

    auto boost_immune = [](const std::vector<group>& in_state, int64_t boost) -> std::vector<group> {
        auto state(in_state);
        for (group& g : state) {
            if (g.side == army::immune) {
                g.atk += boost;
            }
        }
        return state;
    };
    for (int64_t b = 1; ; ++b) {
        auto [final_state, finished] = combat(boost_immune(INPUT, b));
        if (finished) {
            auto winner = from(final_state)
                        | select([](group& g) { return g.side; })
                        | distinct()
                        | default_if_empty(army::infection)
                        | first();
            if (winner == army::immune) {
                auto remaining_u = from(final_state)
                                 | sum([](group& g) { return g.u_count; });
                std::cout << "Puzzle #2: immune system wins with boost " << b << "; " << remaining_u << " units remaining." << std::endl;
                break;
            }
        }
    }
}

void day25_1()
{
    struct point {
        int x = 0, y = 0, z = 0, t = 0;
    };
    auto manhattan = [](const point& a, const point& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z) + std::abs(a.t - b.t);
    };

    std::vector<point> input;
    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "out") {
            break;
        }
        std::istringstream iss(line);
        point p;
        char c = 0;
        iss >> p.x >> c >> p.y >> c >> p.z >> c >> p.t;
        input.push_back(p);
    }

    struct constellation {
        std::vector<point> points;
    };
    auto close_enough_pp = [&](const point& a, const point& b) {
        return manhattan(a, b) <= 3;
    };
    auto close_enough_pc = [&](const point& p, const constellation& c) {
        return from (c.points)
             | where([&](auto&& cp) { return close_enough_pp(p, cp); })
             | any();
    };
    auto form_cons = [&](std::vector<point> ps, std::vector<constellation> cs) {
        size_t numpoints = ps.size();
        for (auto it = ps.begin(); it != ps.end(); ) {
            bool added = false;
            for (auto&& c : cs) {
                if (close_enough_pc(*it, c)) {
                    c.points.push_back(*it);
                    it = ps.erase(it);
                    added = true;
                    break;
                }
            }
            if (!added) {
                ++it;
            }
        }
        if (numpoints == ps.size()) {
            constellation c;
            c.points.push_back(ps.back());
            ps.pop_back();
            cs.push_back(std::move(c));
        }
        return std::make_tuple(std::move(ps), std::move(cs));
    };
    auto form_multiple_cons = [&](std::vector<point> ps) {
        std::vector<constellation> cs;
        while (!ps.empty()) {
            auto [newps, newcs] = form_cons(ps, cs);
            if (newps.size() == ps.size()) {
                break;
            }
            ps = std::move(newps);
            cs = std::move(newcs);
        }
        return std::make_tuple(std::move(ps), std::move(cs));
    };

    auto [finalpoints, finalcons] = form_multiple_cons(input);
    std::cout << "Puzzle #1: number of constellations: " << finalcons.size() + finalpoints.size() << std::endl;
}

int main()
{
    day25_1();
    return 0;
}
