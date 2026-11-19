#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstdio>
#include "imgui.h"

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };
struct ViewMatrix { float m[4][4]; };

struct Brawler {
    uintptr_t address;
    Vec3 position;
    Vec3 head;
    Vec3 foot;
    int health;
    int maxHealth;
    int team;
    bool valid;
};

class BrawlESP {
public:
    BrawlESP() = default;
    ~BrawlESP() { Detach(); }

    bool Attach(const std::wstring& emulatorProcess);
    void Detach();
    bool IsAttached() const { return m_process != nullptr; }

    bool ReadMemory(uintptr_t address, void* buffer, size_t size) const;
    template<typename T> T Read(uintptr_t address) const {
        T v{}; ReadMemory(address, &v, sizeof(T)); return v;
    }

    uintptr_t GetModuleBase(const std::wstring& moduleName) const;

    bool UpdateBrawlers();
    void Render();

    void SetOffsets(uintptr_t brawlerList, uintptr_t viewMatrix, uintptr_t localPlayer,
                    uintptr_t health, uintptr_t maxHealth, uintptr_t team,
                    uintptr_t position, uintptr_t head, uintptr_t foot,
                    size_t stride, int maxBrawlers);

    bool WorldToScreen(const Vec3& world, Vec2& screen, const ViewMatrix& vm,
                       int screenW, int screenH) const;

private:
    HANDLE m_process = nullptr;
    DWORD m_pid = 0;
    uintptr_t m_moduleBase = 0;

    uintptr_t m_brawlerList = 0;
    uintptr_t m_viewMatrix = 0;
    uintptr_t m_localPlayer = 0;
    uintptr_t m_healthOff = 0;
    uintptr_t m_maxHealthOff = 0;
    uintptr_t m_teamOff = 0;
    uintptr_t m_posOff = 0;
    uintptr_t m_headOff = 0;
    uintptr_t m_footOff = 0;
    size_t m_stride = 0;
    int m_maxBrawlers = 16;

    std::vector<Brawler> m_brawlers;
    int m_localTeam = 0;
    ViewMatrix m_vm{};
};
