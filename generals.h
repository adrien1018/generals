#ifndef GENERALS_H_
#define GENERALS_H_

#include <vector>
#include <cstdint>
#include <algorithm>

class GeneralsGame;

struct GeneralsCell {
  uint32_t troops;
  int16_t owner;
  enum Type : uint8_t {
    kSpace = 0,
    kCity = 1,
    kGeneral = 2,
    kGeneralCity = 3,
    kMountain = 4,
    kUnknown = 8,
    kUnknownMountain = 12
  } type;
 private:
  uint8_t flag;
  void Hide_() {
    troops = 0; owner = -1;
    type = (type == kCity || type == kMountain) ? kUnknownMountain : kUnknown;
  }
 public:
  GeneralsCell(uint32_t t = 0, int16_t o = -1, Type ty = kSpace) :
      troops(t), owner(o), type(ty), flag(0) {}
  friend class GeneralsGame;
};

struct GeneralsMove {
  uint16_t x, y;
  enum Direction : uint16_t {
    kNoop = 0,
    kDown = 1,
    kRight = 2,
    kUp = 3,
    kLeft = 4
  } dir;
  bool half;
};

struct GeneralsPlayerInfor {
  uint32_t troops, lands;
  bool alive;
  GeneralsPlayerInfor() : troops(1), lands(1), alive(true) {}
};

class GeneralsGame {
 private:
  struct Position_ { int x, y; };
  std::vector<size_t> cities_;
  std::vector<GeneralsCell> map_;
  std::vector<GeneralsPlayerInfor> stat_;
  uint16_t height_, width_;
  int16_t players_, alivecount_;
  uint32_t rounds_;
  GeneralsCell& Pos_(const Position_& i) { return map_[i.x * width_ + i.y]; }
  bool Visible_(const Position_&, int) const;
 public:
  GeneralsGame() : height_(0), width_(0), players_(0), alivecount_(0),
      rounds_(0) {}

  void Init(int, int, int, int, int, uint64_t = 0);
  void NextRound(const GeneralsMove*);

  size_t Height() const { return height_; }
  size_t Width() const { return width_; }
  size_t Round() const { return rounds_; }
  bool Finished() const { return alivecount_ < 2; }
  const std::vector<GeneralsCell>& GetMap() const { return map_; }
  std::vector<GeneralsCell> GetMap(int) const;
  const std::vector<GeneralsPlayerInfor>& GetStat() const { return stat_; }
};

inline bool GeneralsGame::Visible_(const Position_& i, int pl) const {
  size_t pos = i.x * width_ + i.y;
  if (map_[pos].owner == pl) return true;
  if (i.y != 0 && map_[pos - 1].owner == pl) return true;
  if (i.y != width_ - 1 && map_[pos + 1].owner == pl) return true;
  if (i.x != 0) {
    if (map_[pos - width_].owner == pl) return true;
    if (i.y != 0 && map_[pos - width_ - 1].owner == pl) return true;
    if (i.y != width_ - 1 && map_[pos - width_ + 1].owner == pl) return true;
  }
  if (i.x != height_ - 1) {
    if (map_[pos + width_].owner == pl) return true;
    if (i.y != 0 && map_[pos + width_ - 1].owner == pl) return true;
    if (i.y != width_ - 1 && map_[pos + width_ + 1].owner == pl) return true;
  }
  return false;
}

#endif
