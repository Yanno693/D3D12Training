#include "GameInputs.h"

SHORT GameInputs::m_aPrevInput[256];
SHORT GameInputs::m_aInput[256];

void GameInputs::Initialize()
{
	ZeroMemory(m_aPrevInput, sizeof(m_aPrevInput));
	ZeroMemory(m_aInput, sizeof(m_aInput));
}

void GameInputs::UpdateState()
{
	memcpy(m_aPrevInput, m_aInput, sizeof(m_aInput));

	for (UINT uiKeyIterator = 0; uiKeyIterator < 256; uiKeyIterator++)
	{
		m_aInput[uiKeyIterator] = GetKeyState(uiKeyIterator);
	}
}

bool GameInputs::GetKeyDown(const BYTE a_iKeyCode)
{
	return GetKey(a_iKeyCode) && !(m_aPrevInput[a_iKeyCode] & 0x8000);
}

bool GameInputs::GetKeyUp(const BYTE a_iKeyCode)
{
	return !GetKey(a_iKeyCode) && (m_aPrevInput[a_iKeyCode] & 0x8000);
}

bool GameInputs::GetKey(const BYTE a_iKeyCode)
{
	return m_aInput[a_iKeyCode] & 0x8000;
}