/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2020 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "types.h"
#include "bitboard.h"

namespace Stockfish::PSQT {

#define S(mg, eg) make_score(mg, eg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
constexpr Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  { },
  { },
  { // Knight
   { S( 0,0), S(0,0), S(0,0), S(0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S( 0,0), S( 0,0), S( 0,0), S(0,0) },
   { S(0,0), S(0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S( 0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) }
  },
  { // Bishop
   { S( 0,0), S(0,0), S(0,0), S(0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S( 0,0), S( 0,0), S( 0,0), S(0,0) },
   { S(0,0), S(0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S( 0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) }
  },
  { // Rook
   { S( 0,0), S(0,0), S(0,0), S(0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S( 0,0), S( 0,0), S( 0,0), S(0,0) },
   { S(0,0), S(0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S( 0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) }
  },
  { // Queen
   { S( 0,0), S(0,0), S(0,0), S(0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S( 0,0), S( 0,0), S( 0,0), S(0,0) },
   { S(0,0), S(0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S( 0,0) },
   { S(0,0), S( 0,0), S(0,0), S( 0,0) },
   { S(0,0), S(0,0), S( 0,0), S(0,0) }
  },
  { // King
   { S(271,  1), S(327, 45), S(271, 85), S(198, 76) },
   { S(278, 53), S(303,100), S(234,133), S(179,135) },
   { S(195, 88), S(258,130), S(169,169), S(120,175) },
   { S(164,103), S(190,156), S(138,172), S( 98,172) },
   { S(154, 96), S(179,166), S(105,199), S( 70,199) },
   { S(123, 92), S(145,172), S( 81,184), S( 31,191) },
   { S( 88, 47), S(120,121), S( 65,116), S( 33,131) },
   { S( 59, 11), S( 89, 59), S( 45, 73), S( -1, 78) }
  }
};

constexpr Score PBonus[RANK_NB][FILE_NB] =
  { // Pawn (asymmetric distribution)
   { },
   { S(  0, 20), S(  0, 20), S( 0, 20), S( 0, 20), S( 0, 20), S( 0, 20), S(  0, 20), S( 0, 20) },
   { S( 0, 40), S(0, 40), S(0, 40), S( 0, 40), S( 0, 40), S( 0, 40), S(  0, 40), S(0, 40) },
   { S(  0, 60), S( 0, 60), S(  0, 60), S(  0, 60), S(  0, 60), S(  0, 60), S( 0, 60), S( 0, 60) },
   { S( 0, 80), S(  0, 80), S(0, 80), S( 0, 80), S( 0, 80), S(0, 80), S(0, 80), S( 0, 80) },
   { S(  0, 100), S( 0, 100), S(  0, 100), S(  0, 100), S(  0, 100), S(  0, 100), S( 0, 100), S( 0, 100) },
   { S( 0, 140), S(  0, 140), S(  0, 140), S( 0, 140), S(  0, 140), S( 0, 140), S(  0, 140), S(  0, 140) }
  };

#undef S

Score psq[PIECE_NB][SQUARE_NB];


// PSQT::init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] and PBonus[], adding the piece value, then the black halves of
// the tables are initialized by flipping and changing the sign of the white scores.
void init() {

  for (Piece pc : {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN})
  {
      Score score = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);

      for (Square s = SQ_A1; s <= SQ_H8; ++s)
      {
          File f = File(edge_distance(file_of(s)));
          psq[ pc][s] = score + (type_of(pc) == PAWN ? PBonus[rank_of(s)][file_of(s)]
                                                     : Bonus[pc][rank_of(s)][f]);
          psq[~pc][flip_rank(s)] = -psq[pc][s];
      }
  }
}

} // namespace PSQT
