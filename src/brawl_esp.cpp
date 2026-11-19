#include "brawl_esp.h"
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

bool BrawlESP::Attach(const std::wstring& emulatorProcess) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe{ sizeof(pe) };
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (emulatorProcess == pe.szExeFile) {
                m_pid = pe.th32ProcessID;
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    if (!found) return false;

    m_process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_pid);
    if (!m_process) return false;

    m_moduleBase = GetModuleBase(emulatorProcess);
    if (!m_moduleBase) { Detach(); return false; }
    return true;
}

void BrawlESP::Detach() {
    if (m_process) { CloseHandle(m_process); m_process = nullptr; }
    m_pid = 0; m_moduleBase = 0; m_brawlers.clear();
}

bool BrawlESP::ReadMemory(uintptr_t address, void* buffer, size_t size) const {
    if (!m_process || !address || !buffer || !size) return false;
    SIZE_T read = 0;
    return ReadProcessMemory(m_process, (LPCVOID)address, buffer, size, &read)
           && read == size;
}

uintptr_t BrawlESP::GetModuleBase(const std::wstring& moduleName) const {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32W me{ sizeof(me) };
    uintptr_t base = 0;
    if (Module32FirstW(snap, &me)) {
        do {
            if (moduleName == me.szModule) { base = (uintptr_t)me.modBaseAddr; break; }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

bool BrawlESP::UpdateBrawlers() {
    if (!m_process || !m_moduleBase) return false;

    m_vm = Read<ViewMatrix>(m_moduleBase + m_viewMatrix);

    uintptr_t local = Read<uintptr_t>(m_moduleBase + m_localPlayer);
    m_localTeam = local ? Read<int>(local + m_teamOff) : 0;

    uintptr_t listBase = Read<uintptr_t>(m_moduleBase + m_brawlerList);
    if (!listBase) { m_brawlers.clear(); return false; }

    m_brawlers.clear();
    m_brawlers.reserve(m_maxBrawlers);

    for (int i = 0; i < m_maxBrawlers; ++i) {
        uintptr_t addr = Read<uintptr_t>(listBase + i * m_stride);
        if (!addr || addr == local) continue;

        int hp = Read<int>(addr + m_healthOff);
        int maxHp = Read<int>(addr + m_maxHealthOff);
        if (hp <= 0 || maxHp <= 0 || hp > maxHp) continue;

        Brawler b{};
        b.address = addr;
        b.health = hp;
        b.maxHealth = maxHp;
        b.team = Read<int>(addr + m_teamOff);
        b.position = Read<Vec3>(addr + m_posOff);
        b.head = Read<Vec3>(addr + m_headOff);
        b.foot = Read<Vec3>(addr + m_footOff);
        b.valid = std::isfinite(b.position.x) && std::isfinite(b.position.y)
               && std::isfinite(b.position.z);
        if (b.valid) m_brawlers.push_back(b);
    }
    return true;
}

bool BrawlESP::WorldToScreen(const Vec3& world, Vec2& screen, const ViewMatrix& vm,
                             int screenW, int screenH) const {
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];
    if (w < 0.001f) return false;
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    float ndcX = x / w, ndcY = y / w;
    screen.x = (screenW * 0.5f) + (screenW * 0.5f) * ndcX;
    screen.y = (screenH * 0.5f) - (screenH * 0.5f) * ndcY;
    return true;
}

void BrawlESP::Render() {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    int w = (int)io.DisplaySize.x, h = (int)io.DisplaySize.y;

    for (const auto& b : m_brawlers) {
        if (b.team == m_localTeam) continue;

        Vec2 foot, head;
        if (!WorldToScreen(b.foot, foot, m_vm, w, h)) continue;
        if (!WorldToScreen(b.head, head, m_vm, w, h)) continue;

        float height = foot.y - head.y;
        float width = height * 0.45f;
        float x = head.x - width * 0.5f;

        ImU32 col = IM_COL32(255, 60, 60, 255);
        draw->AddRect(ImVec2(x, head.y), ImVec2(x + width, foot.y), col, 0.0f, 0, 1.5f);

        Vec2 center = { head.x, head.y + height * 0.5f };
        draw->AddLine(ImVec2(head.x, head.y), ImVec2(center.x, center.y), col, 1.0f);
        draw->AddLine(ImVec2(center.x, center.y), ImVec2(foot.x, foot.y), col, 1.0f);

        char hp[32];
        snprintf(hp, sizeof(hp), "%d/%d", b.health, b.maxHealth);
        draw->AddText(ImVec2(x, foot.y + 2), IM_COL32(255, 255, 255, 255), hp);
    }
}

void BrawlESP::SetOffsets(uintptr_t brawlerList, uintptr_t viewMatrix, uintptr_t localPlayer,
                          uintptr_t health, uintptr_t maxHealth, uintptr_t team,
                          uintptr_t position, uintptr_t head, uintptr_t foot,
                          size_t stride, int maxBrawlers) {
    m_brawlerList = brawlerList; m_viewMatrix = viewMatrix; m_localPlayer = localPlayer;
    m_healthOff = health; m_maxHealthOff = maxHealth; m_teamOff = team;
    m_posOff = position; m_headOff = head; m_footOff = foot;
    m_stride = stride; m_maxBrawlers = maxBrawlers;
}
