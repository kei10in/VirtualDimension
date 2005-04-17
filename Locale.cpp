#include "StdAfx.h"
#include ".\locale.h"

Locale Locale::m_instance;

Locale::Locale(void)
{
	m_resDll = (HINSTANCE)::LoadLibrary("langEN.dll");
}

Locale::~Locale(void)
{
	::FreeLibrary(m_resDll);
}
