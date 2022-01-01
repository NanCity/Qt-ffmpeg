#include "tag.h"
M_Tag::M_Tag() {}

M_Tag::~M_Tag() {}


QString M_Tag::GetArtist() { return Artist; }

QString M_Tag::GetTitle() { return Title; }

QString M_Tag::GetAblue() { return Ablue; }

QString M_Tag::GetSize() { return Size; }

QString M_Tag::GetDuration() { return Duration; }

QPixmap M_Tag::GetAblueArt() { return AblueArt; }

inline void M_Tag::Print() { std::printf("tag point: *p\n", this); }
