/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   mem_helpers.h
    Desc:       operations with memory
    Created:    13.08.2024 by DimaSkup
\**********************************************************************************/
#ifndef MEM_HELPERS_H
#define MEM_HELPERS_H

// yes, I did it
#define NEW new(std::nothrow)


template<typename T>
inline void SafeDelete(T*& p)
{
	if (p) { delete(p); p = nullptr; }
}

///////////////////////////////////////////////////////////

template<typename T>
inline void SafeDeleteArr(T*& p)
{
	if (p) { delete[](p); p = nullptr; }
}

///////////////////////////////////////////////////////////

template<class T>
inline void SafeShutdown(T** ppT)
{
	if (*ppT) { (*ppT)->Shutdown(); delete(ppT); *ppT = nullptr; }
}

///////////////////////////////////////////////////////////

template<class T>
inline void SafeRelease(T** ppT)
{
	if (*ppT) { (*ppT)->Release(); *ppT = nullptr; }
}

#endif