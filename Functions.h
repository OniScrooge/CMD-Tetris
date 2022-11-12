#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//Screen dimensions at CMD size
constexpr int nScreenWidth{ 120 };
constexpr int nScreenHeight{ 30 };

//Tetris block objects
std::wstring tetromino[7];

//Game field size
constexpr int nFieldWidth{ 12 };
constexpr int nFieldHeight{ 18 };
unsigned char* pField = nullptr;

int Rotate(const int& px, const int& py, const int& r)
{
	switch (r % 4)
	{
	case 0: return { (py * 4) + px };		// 0   degrees
	case 1: return { 12 + py - (px * 4) };	// 90  degrees
	case 2: return { 15 - (py * 4) - px };	// 180 degrees
	case 3: return { 3 - py + (px * 4) };	// 270 degrees
	}
	return 0;
}

bool DoesPieceFit(const int& nTetromino, const int& nRotation, const int& nPosX, const int& nPosY)
{
	for (int px{ 0 }; px < 4; px++)
		for (int py{ 0 }; py < 4; py++)
		{
			//Get index into piece
			int pi{ Rotate(px, py, nRotation) };

			//Get index into field
			int fi{ (nPosY + py) * nFieldWidth + (nPosX + px) };

			if (nPosX + px >= 0 && nPosY + py < nFieldHeight)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false; //Fail on first hit
				}
			}
		}

	return true;
}

#endif