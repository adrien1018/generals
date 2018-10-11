#include "generals.h"

#include <deque>
#include <random>

void GeneralsGame::Init(int h, int w, int pl, int moun, int city,
                        uint64_t seed) {
  // Adjust ranges
  height_ = h = std::min(std::max(h, 10), 10000);
  width_ = w = std::min(std::max(w, 10), 10000);
  alivecount_ = players_ = pl = std::min(std::max(pl, 2),
      std::min(h * w / 32, 10000));
  rounds_ = 0;
  map_.clear(); map_.resize(h * w);
  stat_.clear(); stat_.resize(pl);
  if (moun + city > h * w - pl) {
    double v = (double)(moun + city + 1) / (h * w - pl);
    moun = moun / v;
    city = moun / v;
  }

  std::mt19937_64 gen(seed);
  typedef std::uniform_int_distribution<int> irand;

  std::vector<Position_> gnrs;
  { // Generals position
    auto Dist = [](const Position_& a, const Position_& b) {
      return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    };
    auto MinDistance = [&Dist](const std::vector<Position_>& p) {
      int res = 1 << 30;
      for (size_t i = 0; i < p.size(); i++) {
        for (size_t j = i + 1; j < p.size(); j++) {
          res = std::min(res, Dist(p[i], p[j]));
        }
      }
      return res;
    };
    auto GeneratePositions = [&](size_t N) {
      std::vector<Position_> ret;
      while (N--) ret.push_back({irand(0, h - 1)(gen), irand(0, w - 1)(gen)});
      return ret;
    };

    gnrs = GeneratePositions(pl);
    int now = MinDistance(gnrs);
    for (int i = 0; i < 1 + pl * 2; i++) {
      auto tmp = GeneratePositions(pl);
      int dis = MinDistance(tmp);
      if (dis <= 3) { i--; continue; }
      if (dis > now) now = dis, tmp.swap(gnrs);
    }
  }

  // Generate map
  std::fill_n(map_.begin(), moun,
      GeneralsCell(0, -1, GeneralsCell::kMountain));
  std::fill_n(map_.begin() + moun, city,
      GeneralsCell(irand(40, 50)(gen), -1, GeneralsCell::kCity));
  std::shuffle(map_.begin(), map_.end(), gen);
  for (int i = 0; i < pl; i++) {
    if (Pos_(gnrs[i]).type != GeneralsCell::kSpace) {
      Position_ t;
      do {
        t = {irand(0, h - 1)(gen), irand(0, w - 1)(gen)};
      } while (Pos_(t).type != GeneralsCell::kSpace);
      std::swap(Pos_(t), Pos_(gnrs[i]));
    }
    Pos_(gnrs[i]) = GeneralsCell(1, i, GeneralsCell::kGeneral);
  }

  size_t N = (size_t)width_ * height_;
  { // Ensure connectivity between generals
    std::deque<Position_> stk1, stk2;
    int viscount = 1;
    stk1.push_back(gnrs[0]);
    auto IsAllVis = [&](){
      for (int i = 0; i < pl; i++) {
        if (Pos_(gnrs[i]).flag < 8) return false;
      }
      return true;
    };
    auto ExtendOnce = [&](int q){
      auto Push = [&](const Position_& x, int dir){
        auto& now = Pos_(x);
        if (now.flag) return;
        viscount++;
        now.flag = dir;
        if (now.type == GeneralsCell::kMountain ||
            now.type == GeneralsCell::kCity) {
          stk2.push_back(x);
        } else {
          stk1.push_back(x);
        }
      };
      while (stk1.size()) {
        Position_ x = stk1.front(); stk1.pop_front();
        Pos_(x).flag |= q;
        if (x.x != 0) Push({x.x - 1, x.y}, 1);
        if (x.y != 0) Push({x.x, x.y - 1}, 2);
        if (x.x != height_ - 1) Push({x.x + 1, x.y}, 3);
        if (x.y != width_ - 1) Push({x.x, x.y + 1}, 4);
      }
      stk1.swap(stk2);
    };
    bool flag = false;
    ExtendOnce(8);
    while (!IsAllVis()) {
      flag = true;
      ExtendOnce(16);
    }
    if (flag) {
      for (int i = 1; i < pl; i++) {
        Position_ pos = gnrs[i];
        if (Pos_(pos).flag < 16) continue;
        flag = true;
        while (flag) {
          auto& now = Pos_(pos);
          switch (now.flag & 7) {
            case 0: flag = false; break;
            case 1: pos.x++; break;
            case 2: pos.y++; break;
            case 3: pos.x--; break;
            case 4: pos.y--; break;
            default: throw;
          }
          if (now.type == GeneralsCell::kMountain ||
              now.type == GeneralsCell::kCity) {
            now.type = GeneralsCell::kSpace;
          } else if (now.flag < 16) {
            break;
          }
        }
      }
    }
    for (size_t i = 0; i < N; i++) map_[i].flag = 0;
  }

  // City list
  cities_.clear();
  for (size_t i = 0; i < N; i++) {
    if (map_[i].type == GeneralsCell::kGeneral ||
        map_[i].type == GeneralsCell::kCity) {
      cities_.push_back(i);
    }
  }
}

void GeneralsGame::NextRound(const GeneralsMove* mv) {
  if (!players_) return;
  if (alivecount_ < 2) return;
  size_t N = (size_t)width_ * height_;

  // Move troops / capture / change stats
  for (int16_t i = 0; i < players_; i++) {
    if (!stat_[i].alive) continue;

    if (mv[i].x >= height_ || mv[i].y >= width_) continue;
    Position_ pos = {mv[i].x, mv[i].y}, posn = pos;
    GeneralsCell& now = Pos_(pos);
    if (now.owner != i || now.troops <= 1) continue;
    switch (mv[i].dir) {
      case GeneralsMove::kNoop: continue;
      case GeneralsMove::kUp: {
        if (pos.x == 0) continue;
        posn.x--; break;
      }
      case GeneralsMove::kDown: {
        if (pos.x == height_ - 1) continue;
        posn.x++; break;
      }
      case GeneralsMove::kLeft: {
        if (pos.y == 0) continue;
        posn.y--; break;
      }
      case GeneralsMove::kRight: {
        if (pos.y == width_ - 1) continue;
        posn.y++; break;
      }
    }
    GeneralsCell& nxt = Pos_(posn);
    if (nxt.type == GeneralsCell::kMountain) continue;

    uint32_t mv_trp = mv[i].half ? now.troops / 2 : now.troops - 1;
    int16_t nown = nxt.owner;
    now.troops -= mv_trp;
    if (nown == i) {
      nxt.troops += mv_trp;
    } else {
      if (mv_trp <= nxt.troops) {
        stat_[i].troops -= mv_trp;
        stat_[nown].troops -= mv_trp;
        nxt.troops -= mv_trp;
      } else {
        stat_[i].troops -= nxt.troops;
        stat_[i].lands++;
        if (nown != -1) {
          stat_[nown].troops -= nxt.troops;
          stat_[nown].lands--;
        }
        nxt.owner = i;
        nxt.troops = mv_trp - nxt.troops;
        if (nxt.type == GeneralsCell::kGeneral) {
          nxt.type = GeneralsCell::kGeneralCity;
          for (size_t j = 0; j < N; j++) {
            if (map_[j].owner == nown) {
              stat_[i].lands++;
              stat_[i].troops += map_[j].troops = (map_[j].troops + 1) / 2;
              map_[j].owner = i;
            }
          }
          stat_[nown].lands = stat_[nown].troops = 0;
          stat_[nown].alive = false;
          alivecount_--;
        }
      }
    }
  }

  // Spawn troops
  rounds_++;
  if (rounds_ % 50 == 0) {
    for (size_t i = 0; i < N; i++) {
      if (map_[i].owner == -1) continue;
      map_[i].troops++;
      stat_[map_[i].owner].troops++;
    }
  } else if (rounds_ % 2 == 0) {
    for (size_t i : cities_) {
      if (map_[i].owner == -1) continue;
      map_[i].troops++;
      stat_[map_[i].owner].troops++;
    }
  }
}

std::vector<GeneralsCell> GeneralsGame::GetMap(int pl) const {
  if (pl < 0) return map_;
  std::vector<GeneralsCell> ret = map_;
  Position_ i = {0, 0};
  for (size_t j = 0; i.x < height_; i.x++) {
    for (i.y = 0; i.y < width_; i.y++, j++) {
      if (!Visible_(i, pl)) ret[j].Hide_();
    }
  }
  return ret;
};

