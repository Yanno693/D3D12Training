#pragma once

#include "D3DIncludes.h"

class GameInputs
{
private:
	static SHORT m_aPrevInput[256];
	static SHORT m_aInput[256];

	

public:
	static void Initialize();
	static void UpdateState();

	static bool GetKeyDown(const BYTE a_iKeyCode);
	static bool GetKeyUp(const BYTE a_iKeyCode);
	static bool GetKey(const BYTE a_iKeyCode);
};

